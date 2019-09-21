/*
 * Copyright (C) 2019  Igor Cananea <icc@avalonbits.com>
 * Author: Igor Cananea <icc@avalonbits.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package parser

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
	"unicode/utf8"

	"github.com/avalonbits/gsm/lexer"
)

type Word uint32
type AST struct {
	Orgs     []Org
	Consts   map[string]string
	Includes map[string]string
	Exported map[string]struct{}
	Hash     string
}

type Org struct {
	Addr     uint32
	PIC      bool
	Sections []Section
}

func (o *Org) activeSection() *Section {
	return &o.Sections[len(o.Sections)-1]
}

func (o Org) WordCount() int {
	var count int
	for _, s := range o.Sections {
		count += s.wordCount()
	}
	return count
}

func (o Org) RelSizeWords(org Org) int {
	sz := o.Addr/4 + uint32(o.WordCount())
	wc := uint32(org.Addr / 4)
	return int(wc - sz)
}

type Statement struct {
	Value     uint32
	Instr     Instruction
	Label     string
	ArraySize int
	Str       string
	lineNum   int
}

func (s Statement) Errorf(format string, a ...interface{}) error {
	return errors.New(fmt.Sprintf("line %d: %s", s.lineNum, fmt.Sprintf(format, a...)))
}

func (s Statement) WordCount() int {
	if s.ArraySize > 0 {
		sz := s.ArraySize / 4
		if s.ArraySize%4 != 0 {
			sz += 1
		}
		return sz
	}

	if len(s.Str) > 0 {
		sz := utf8.RuneCountInString(s.Str) + 1
		sz *= 2
		if sz%4 != 0 {
			sz += 2
		}
		return sz / 4
	}

	return 1
}

type OpType int

const (
	OP_REG OpType = iota
	OP_NUMBER
	OP_DIFF
	OP_LABEL
	OP_EXTERNAL_LABEL
)

type Operand struct {
	Op   string
	Type OpType
}

type Instruction struct {
	Name string
	Op1  Operand
	Op2  Operand
	Op3  Operand
	Op4  Operand
}

func (i Instruction) String() string {
	var sb strings.Builder
	sb.WriteString(i.Name)
	opCount := operands[i.Name]

	if opCount == 0 {
		return sb.String()
	}

	sb.WriteRune(' ')
	if i.Name == "str" {
		sb.WriteRune('[')
	}
	sb.WriteString(i.Op1.Op)
	if i.Name == "str" {
		sb.WriteRune(']')
	}

	if opCount == 1 {
		return sb.String()
	}

	sb.WriteString(", ")
	if i.Name == "ldr" {
		sb.WriteRune('[')
	}
	sb.WriteString(i.Op2.Op)
	if i.Name == "ldr" {
		sb.WriteRune(']')
	}

	if opCount == 2 {
		return sb.String()
	}

	sb.WriteString(", ")
	sb.WriteString(i.Op3.Op)
	return sb.String()
}

type Block struct {
	Label      string
	inFunc     bool
	funcName   string
	Statements []Statement
	exported   bool
}

func (b Block) Errorf(format string, a ...interface{}) error {
	if len(b.Statements) > 0 {
		return b.Statements[0].Errorf(format, a...)
	}
	return fmt.Errorf(format, a...)
}

func (b Block) LocalLabelName() string {
	return b.JumpName("", b.Label)
}

func (b Block) LabelName(incl string) string {
	return b.JumpName(incl, b.Label)
}

func (b Block) JumpName(incl, label string) string {
	if incl == "" {
		return b.funcName + label
	}
	return incl + "." + b.funcName + label
}

func (b Block) WordCount() int {
	count := 0
	for _, s := range b.Statements {
		count += s.WordCount()
	}
	return count
}

type SType int

const (
	DATA_SECTION SType = iota
	TEXT_SECTION
	EMBED_FILE
	INCLUDE_FILE
)

type Section struct {
	Type        SType
	Blocks      []Block
	EmbedFile   string
	IncludeFile string
	IncludeName string
}

func (s *Section) activeBlock() *Block {
	return &s.Blocks[len(s.Blocks)-1]
}

func (s *Section) newBlock() *Block {
	s.Blocks = append(s.Blocks, Block{})
	return &s.Blocks[len(s.Blocks)-1]
}

func (s Section) wordCount() int {
	var count int
	for _, b := range s.Blocks {
		count += b.WordCount()
	}
	return count
}

type Tokenizer interface {
	NextToken() lexer.Token
	PeakToken() lexer.Token
	IgnoreWhiteSpace(ignore bool)
	Line() int
}

type Parser struct {
	tokenizer Tokenizer
	err       error
	Ast       *AST
	Entry     string
	Bin       bool
}

func (p *Parser) activeOrg() *Org {
	return &p.Ast.Orgs[len(p.Ast.Orgs)-1]
}

func (p *Parser) Errorf(format string, a ...interface{}) error {
	return errors.New(fmt.Sprintf("line %d: %s", p.tokenizer.Line(), fmt.Sprintf(format, a...)))
}

func New(t Tokenizer) *Parser {
	t.IgnoreWhiteSpace(true)
	return &Parser{tokenizer: t, Ast: &AST{
		Consts:   make(map[string]string),
		Includes: make(map[string]string),
		Exported: make(map[string]struct{}),
	}}
}

type state int

const (
	START state = iota
	ORG
	SECTION
	EMBED_STATEMENT
	INCLUDE_STATEMENT
	DATA_BLOCK
	TEXT_BLOCK
	ERROR
	END
)

func (p *Parser) Parse(requireLibrary bool) error {
	st := p.skipCommentsAndWhitespace(START)
	for ; st != END; st = p.skipCommentsAndWhitespace(st) {
		switch st {
		case START:
			st = p.mode(requireLibrary)
		case ORG:
			st = p.org()
		case SECTION:
			st = p.section()
		case EMBED_STATEMENT:
			st = p.embed()
		case INCLUDE_STATEMENT:
			st = p.include()
		case DATA_BLOCK:
			st = p.data_block(st)
		case TEXT_BLOCK:
			st = p.textBlock(st)
		case ERROR:
			if p.err == nil {
				panic("reached error state, but no error found")
			}
			st = END
		}
	}
	return p.err
}

func (p *Parser) skipCommentsAndWhitespace(next state) state {
	for {
		tok := p.tokenizer.PeakToken()
		if tok.Type == lexer.EOF {
			return END
		}
		if tok.Type != lexer.SEMICOLON && tok.Type != lexer.NEWLINE {
			return next
		}
		if tok.Type == lexer.SEMICOLON {
			for tok.Type != lexer.NEWLINE && tok.Type != lexer.EOF {
				tok = p.tokenizer.NextToken()
			}
		} else {
			p.tokenizer.NextToken()
		}
	}
}

func (p *Parser) mode(requireLibrary bool) state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.BIN_FILE && tok.Type != lexer.PROGRAM_FILE && tok.Type != lexer.LIBRARY_FILE {
		p.err = p.Errorf("expected .bin, .program or .library, got %q", tok.Literal)
		return ERROR
	}
	if requireLibrary && tok.Type != lexer.LIBRARY_FILE {
		p.err = p.Errorf("this file is being included by another, so it must be a .library")
		return ERROR
	}
	p.Bin = tok.Type == lexer.BIN_FILE
	if p.Bin {
		return INCLUDE_STATEMENT
	}
	if tok.Type == lexer.PROGRAM_FILE {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.IDENT {
			p.err = p.Errorf("expected an entry point, got %q", tok.Literal)
			return ERROR
		}
		p.Entry = tok.Literal
	}

	// For library and program, we create a single PIC org before moving to section parser.
	o := Org{PIC: true, Addr: 0}
	p.Ast.Orgs = append(p.Ast.Orgs, o)

	return INCLUDE_STATEMENT
}

func (p *Parser) readString() (string, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.D_QUOTE {
		return "", p.Errorf("expected a double quote (\"), got %q", tok.Literal)
	}

	p.tokenizer.IgnoreWhiteSpace(false)
	defer p.tokenizer.IgnoreWhiteSpace(true)

	var sb strings.Builder
	for {
		tok = p.tokenizer.NextToken()
		if tok.Type == lexer.NEWLINE {
			return "", p.Errorf("expected a double quote(\"), got a new line")
		}
		if tok.Type == lexer.D_QUOTE {
			break
		}
		if _, err := sb.WriteString(tok.Literal); err != nil {
			return "", p.Errorf("error reading token in string: %v", err)
		}
	}
	return sb.String(), nil
}

func (p *Parser) include() state {
	tok := p.tokenizer.PeakToken()
	if tok.Type != lexer.INCLUDE {
		if p.Bin {
			return ORG
		}
		return SECTION
	}

	// We know we are processing an include. We can skip it now.
	p.tokenizer.NextToken()
	includeStr, err := p.readString()
	if err != nil {
		p.err = err
		return ERROR
	}
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.AS {
		p.err = p.Errorf("expected as, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT {
		p.err = p.Errorf("expected an identifier, got %q", tok.Literal)
		return ERROR
	}

	if _, ok := p.Ast.Includes[tok.Literal]; ok {
		p.err = p.Errorf("include name %q was already defined.", tok.Literal)
		return ERROR
	}
	p.Ast.Includes[tok.Literal] = includeStr
	return INCLUDE_STATEMENT
}

func (p *Parser) org() state {
	if !p.Bin {
		p.err = p.Errorf(".org directives are only allowed in .bin files.")
		return ERROR
	}
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.ORG {
		p.err = p.Errorf("expected .org got %q", tok.Literal)
		return ERROR
	}
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.NUMBER {
		p.err = p.Errorf("expected an address constant, got %q\n", tok.Literal)
		return ERROR
	}

	n, err := p.parseNumber(tok.Literal)
	if err != nil {
		p.err = err
		return ERROR
	}
	o := Org{PIC: false, Addr: n}
	p.Ast.Orgs = append(p.Ast.Orgs, o)
	return SECTION
}

func (p *Parser) section() state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.SECTION {
		p.err = p.Errorf("expected .section, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.S_DATA && tok.Type != lexer.S_TEXT {
		p.err = p.Errorf("expected data, embed or text, got %q", tok.Literal)
		return ERROR
	}

	// For every new .section entry, we reserve memory for 8 blocks.
	s := Section{Blocks: make([]Block, 0, 8)}
	var next state
	if tok.Type == lexer.S_DATA {
		s.Type = DATA_SECTION
		next = DATA_BLOCK
	} else {
		s.Type = TEXT_SECTION
		next = TEXT_BLOCK
	}

	o := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	o.Sections = append(o.Sections, s)
	return next
}

func (p *Parser) embed() state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.EMBED {
		p.err = p.Errorf("expected .embed, got %q", tok.Literal)
		return ERROR
	}

	embedStr, err := p.readString()
	if err != nil {
		p.err = err
		return ERROR
	}

	s := Section{Type: EMBED_FILE, EmbedFile: embedStr}
	o := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	o.Sections = append(o.Sections, s)
	return SECTION
}

func (p *Parser) data_block(cur state) state {
	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		return ORG
	}
	if tok.Type == lexer.SECTION {
		return SECTION
	}

	if tok.Type == lexer.EMBED {
		return EMBED_STATEMENT
	}

	// Create a new block and add it to the section.
	aBlock := p.activeOrg().activeSection().newBlock()

	switch tok.Type {
	case lexer.IDENT:
		tok := p.tokenizer.NextToken()
		label := tok.Literal
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COLON {
			p.err = p.Errorf("expected \"%s:\", got \"%s%s\"",
				label, label, tok.Literal)
			return ERROR
		}
		aBlock.Label = label
	}
	return p.parseDataEntries(aBlock)
}

func (p *Parser) parseDataEntries(block *Block) state {
	for {
		if st := p.skipCommentsAndWhitespace(START); st != START {
			return st
		}
		tok := p.tokenizer.PeakToken()
		if tok.Type == lexer.ORG || tok.Type == lexer.SECTION || tok.Type == lexer.EMBED ||
			tok.Type == lexer.IDENT {
			return DATA_BLOCK
		}
		tok = p.tokenizer.NextToken()

		switch tok.Type {
		case lexer.ARRAY_TYPE:
			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.NUMBER {
				p.err = p.Errorf("expected a number, got %q", tok.Literal)
				return ERROR
			}
			n, err := p.parseNumber(tok.Literal)
			if err != nil {
				p.err = err
				return ERROR
			}
			if n <= 0 {
				p.err = p.Errorf("expected array with positive value, got %d", n)
				return ERROR
			}

			block.Statements = append(block.Statements, Statement{
				ArraySize: int(n),
				lineNum:   p.tokenizer.Line(),
			})

		case lexer.STRING_TYPE:
			str, err := p.readString()
			if err != nil {
				p.err = err
				return ERROR
			}
			block.Statements = append(block.Statements, Statement{
				Str:     str,
				lineNum: p.tokenizer.Line(),
			})

		case lexer.INT_TYPE:
			tok = p.tokenizer.NextToken()
			if tok.Type == lexer.IDENT {
				// This is likely a label the user wants to store the adress from.
				block.Statements = append(block.Statements, Statement{
					Label:   tok.Literal,
					lineNum: p.tokenizer.Line(),
				})
				break
			}

			n, err := p.parseNumber(tok.Literal)
			if err != nil {
				p.err = err
				return ERROR
			}

			block.Statements = append(block.Statements, Statement{
				Value:   n,
				lineNum: p.tokenizer.Line(),
			})
		case lexer.EQUATE:
			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.IDENT {
				p.err = p.Errorf("expected an identifier, got %q", tok.Literal)
				return ERROR
			}
			constant := tok.Literal
			if _, ok := p.Ast.Consts[constant]; ok {
				p.err = p.Errorf("constant %q was previously defined.", constant)
				return ERROR
			}

			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.NUMBER {
				p.err = p.Errorf("epxected a number, got %q", tok.Literal)
				return ERROR
			}
			p.Ast.Consts[constant] = tok.Literal
		default:
			p.err = p.Errorf("expected either a label or a constant definition, got %q", tok.Literal)
			return ERROR
		}
	}
}

func (p *Parser) textBlock(cur state) state {
	// Create a new text block
	aSection := p.activeOrg().activeSection()
	aBlock := aSection.newBlock()

	// If this is not the first block, check if it is part of a function context.
	if len(aSection.Blocks) > 1 {
		pBlock := &aSection.Blocks[len(aSection.Blocks)-2]
		aBlock.inFunc = pBlock.inFunc
		aBlock.funcName = pBlock.funcName
	}

	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		if aBlock.inFunc {
			p.err = p.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}
		return ORG
	}
	if tok.Type == lexer.SECTION {
		if aBlock.inFunc {
			p.err = p.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}

		return SECTION
	}
	if tok.Type == lexer.EMBED {
		if aBlock.inFunc {
			p.err = p.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}
		return EMBED_STATEMENT
	}

	tok = p.tokenizer.NextToken()
	switch tok.Type {
	case lexer.FUNC_START:
		return p.funcStart(aBlock, true)
	case lexer.INFUNC_START:
		return p.funcStart(aBlock, false)
	case lexer.FUNC_END:
		return p.funcEnd(aBlock)
	case lexer.IDENT:
		return p.textLabel(aBlock, tok.Literal)
	}
	return p.parseInstructions(aBlock)
}

func (p *Parser) funcStart(block *Block, exported bool) state {
	if block.inFunc {
		p.err = p.Errorf("Cannot start a new function within an existing function.")
		return ERROR
	}
	block.inFunc = true
	block.exported = exported

	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT {
		p.err = p.Errorf("Expected a name for the function, got %q", tok.Literal)
		return ERROR
	}
	block.Label = tok.Literal
	block.funcName = block.Label
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.COLON {
		p.err = p.Errorf("expected ':', got %q", tok.Literal)
		return ERROR
	}

	return p.parseInstructions(block)
}

func (p *Parser) funcEnd(block *Block) state {
	if !block.inFunc {
		p.err = p.Errorf("Cannot end a function that was not started.")
		return ERROR
	}
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT || tok.Literal != block.funcName {
		p.err = p.Errorf("Expected %q, the name of the function; got %q", block.funcName, tok.Literal)
		return ERROR
	}

	// The last statement of a function must be an unambiguous control transfer.
	// That means it must either be a "ret", "jmp" or a "halt". "call" is not allowed
	// because it can return back to the function. For the same reason, condition jumps
	// ("jne, "jeq", etc...) are also not allowed.
	if len(block.Statements) == 0 {
		p.err = p.Errorf("Cannot end a function with no instructions.")
		return ERROR
	}
	instr := &block.Statements[len(block.Statements)-1].Instr
	if instr.Name == "" || (instr.Name != "ret" && instr.Name != "jmp" && instr.Name != "halt") {
		p.err = p.Errorf("Function %q must end with either a 'ret', 'jmp' or 'halt' instruction.",
			block.funcName)
		return ERROR
	}
	block.inFunc = false
	return TEXT_BLOCK
}

func (p *Parser) textLabel(block *Block, label string) state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.COLON {
		p.err = p.Errorf("Expected a ':', got %q", tok.Literal)
		return ERROR
	}
	block.Label = label
	return p.parseInstructions(block)
}

func ParseNumber(lit string) (uint32, error) {
	var n int64
	var err error
	mul := int64(1)
	if lit[0] == '-' {
		lit = lit[1:]
		mul = int64(-1)
	}

	if n, err = strconv.ParseInt(lit, 0, 64); err != nil {
		return 0, fmt.Errorf("Expected a 32 bit hexadecimal number, got %q: %v",
			lit, err)
	}

	return uint32(mul * n), nil
}

func (p *Parser) parseNumber(lit string) (uint32, error) {
	n, err := ParseNumber(lit)
	if err != nil {
		return n, p.Errorf(err.Error())
	}
	return n, nil
}

var (
	operands = map[string]int{
		"ldppi": 4,
		"ldpip": 4,
		"stppi": 4,
		"stpip": 4,
		"add":   3,
		"sub":   3,
		"lsl":   3,
		"lsr":   3,
		"asr":   3,
		"and":   3,
		"orr":   3,
		"xor":   3,
		"mul":   3,
		"div":   3,
		"stri":  3,
		"strpi": 3,
		"strip": 3,
		"ldri":  3,
		"ldrpi": 3,
		"ldrip": 3,
		"mov":   2,
		"str":   2,
		"ldr":   2,
		"jeq":   2,
		"jne":   2,
		"jlt":   2,
		"jle":   2,
		"jgt":   2,
		"jge":   2,
		"call":  1,
		"jmp":   1,
		"halt":  0,
		"nop":   0,
		"ret":   0,
		"wfi":   0,
	}
)

func (p *Parser) parseInstructions(block *Block) state {
	for {
		if st := p.skipCommentsAndWhitespace(START); st != START {
			return st
		}
		tok := p.tokenizer.PeakToken()
		if tok.Type == lexer.ORG || tok.Type == lexer.SECTION || tok.Type == lexer.EMBED ||
			tok.Type == lexer.IDENT {
			return TEXT_BLOCK
		}
		tok = p.tokenizer.NextToken()

		if tok.Type == lexer.FUNC_END {
			return p.funcEnd(block)
		}
		if tok.Type != lexer.INSTRUCTION {
			p.err = p.Errorf("expected an instruction, got %q", tok.Literal)
			return ERROR
		}

		// Instructions either have 0-4 operands. ldr and str and variations have brakets around
		// one of the registers so we need to account
		// for them.
		instr := &Instruction{Name: tok.Literal}
		opCount, ok := operands[instr.Name]
		if !ok {
			p.err = p.Errorf("instruction not present in operand table: %q", instr.Name)
			return ERROR
		}

		s := Statement{Instr: *instr, lineNum: p.tokenizer.Line()}
		block.Statements = append(block.Statements, s)

		if opCount == 0 {
			// We are done. Next instruction
			continue
		}
		instr = &block.Statements[len(block.Statements)-1].Instr

		var err error
		if instr.Name == "str" {
			instr.Op1, err = p.parseAddressOperand(true)
		} else if instr.Name == "stri" || instr.Name == "strip" || instr.Name == "strpi" ||
			instr.Name == "stppi" || instr.Name == "stpip" {
			instr.Op1, instr.Op2, err = p.parseIndexOperand(true)
		} else {
			instr.Op1, err = p.parseOperand(opCount > 1)
		}
		if err != nil {
			p.err = err
			return ERROR
		}

		if opCount == 1 {
			// We are done. Next instruction
			continue
		}

		if instr.Name == "ldr" {
			instr.Op2, err = p.parseAddressOperand(false)
		} else if instr.Name == "ldri" || instr.Name == "ldrip" || instr.Name == "ldrpi" {
			instr.Op2, instr.Op3, err = p.parseIndexOperand(false)
		} else if instr.Name != "stri" && instr.Name != "strip" && instr.Name != "strpi" &&
			instr.Name != "stppi" && instr.Name != "stpip" {
			instr.Op2, err = p.parseOperand(opCount >= 3)
		}
		if err != nil {
			p.err = err
			return ERROR
		}

		if opCount == 2 || instr.Name == "ldri" || instr.Name == "ldrip" || instr.Name == "ldrpi" {
			// We are done. Next instruction
			continue
		}

		if instr.Name == "ldpip" || instr.Name == "ldppi" {
			instr.Op3, instr.Op4, err = p.parseIndexOperand(false)
		} else {
			instr.Op3, err = p.parseOperand(opCount > 3)
			if err != nil {
				p.err = err
				return ERROR
			}
			if opCount > 3 {
				instr.Op4, err = p.parseOperand(false)
			}
		}
		if err != nil {
			p.err = err
			return ERROR
		}
	}
}

func (p *Parser) parseOperand(comma bool) (Operand, error) {
	op := Operand{}
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.REGISTER && tok.Type != lexer.IDENT && tok.Type != lexer.NUMBER {
		return op, p.Errorf("expected a register, a number or a label, got %q",
			tok.Literal)
	}
	op.Type = OP_REG
	if tok.Type == lexer.IDENT {
		op.Type = OP_LABEL
	} else if tok.Type == lexer.NUMBER {
		op.Type = OP_NUMBER
	}
	op.Op = tok.Literal
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return op, p.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil

}

func (p *Parser) parseAddressOperand(comma bool) (Operand, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.L_BRACKET {
		return Operand{}, p.Errorf("expected a left bracket '[', got %q", tok.Literal)
	}

	op, err := p.parseOperand(false)
	if err != nil {
		return op, err
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.R_BRACKET {
		return op, p.Errorf("expected a right bracket ']', got %q", tok.Literal)
	}
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return op, p.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil
}

func (p *Parser) parseIndexOperand(comma bool) (Operand, Operand, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.L_BRACKET {
		return Operand{}, Operand{},
			p.Errorf("expected a left bracket '[', got %q", tok.Literal)
	}

	op1, err := p.parseOperand(true)
	if err != nil {
		return Operand{}, Operand{}, err
	}

	op2, err := p.parseOperand(false)
	if err != nil {
		return Operand{}, Operand{}, err
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.R_BRACKET {
		return Operand{}, Operand{},
			p.Errorf("expected a right bracket ']', got %q", tok.Literal)
	}
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return Operand{}, Operand{},
				p.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op1, op2, nil
}

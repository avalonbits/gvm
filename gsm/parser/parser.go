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
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"unicode/utf8"

	"github.com/avalonbits/gsm/lexer"
)

func Parse(in io.Reader, requireLibrary bool) (*AST, error) {
	lex := lexer.New(in)
	p := New(lex)
	if err := p.Parse(requireLibrary); err != nil {
		return nil, err
	}
	p.Ast.Hash = lex.Hash()
	return p.Ast, nil
}

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
	Sections *Section
}

/*
   [] -> <- [] -> <-
*/

func (o *Org) activeSection() *Section {
	return o.Sections.Prev
}

func (o *Org) newSection() *Section {
	sec := &Section{
		Blocks: make([]Block, 0, 8),
	}

	o.linkSection(sec)
	return sec
}

func (o *Org) linkSection(sec *Section) {
	// First section.
	if o.Sections == nil {
		o.Sections = sec
		sec.Prev = sec
		sec.Next = sec
		return
	}

	sec.Next = o.Sections
	sec.Prev = o.Sections.Prev
	o.Sections.Prev.Next = sec
	o.Sections.Prev = sec
	return
}

func (o Org) WordCount() int {
	if o.Sections == nil {
		return 0
	}
	var count int
	for s := o.Sections; ; s = s.Next {
		count += s.wordCount()
		if s.Next == o.Sections {
			break
		}
	}
	return count
}

func (o Org) RelSizeWords(org Org) int {
	oSize := o.Addr/4 + uint32(o.WordCount())
	orgStart := uint32(org.Addr / 4)
	return int(orgStart - oSize)
}

type Statement struct {
	Value     uint32
	Instr     Instruction
	Label     string
	ArraySize int
	Str       string
	Blob      []byte
	lineNum   int

	ResolveReference bool
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
	Code OpCode
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
	Exported   bool
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
	s := b.funcName + label
	if b.funcName == label {
		s = label
	}
	if incl == "" {
		return s
	}
	return incl + "." + s
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
	Prev        *Section
	Next        *Section
}

func (s *Section) activeBlock() *Block {
	ln := len(s.Blocks)
	if ln == 0 {
		return nil
	}
	return &s.Blocks[ln-1]
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

var errEOF = errors.New("EOF")

func (p *Parser) Parse(requireLibrary bool) error {
	err := p.mode(requireLibrary)
	if err != nil && err != errEOF {
		return err
	}
	return nil
}

func (p *Parser) mode(requireLibrary bool) error {
	if err := p.skipCommentsAndWhitespace(); err != nil {
		return err
	}
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.BIN_FILE && tok.Type != lexer.PROGRAM_FILE && tok.Type != lexer.LIBRARY_FILE {
		return p.Errorf("expected .bin, .program or .library, got %q", tok.Literal)
	}
	if requireLibrary && tok.Type != lexer.LIBRARY_FILE {
		return p.Errorf("this file is being included by another, so it must be a .library")
	}
	p.Bin = tok.Type == lexer.BIN_FILE
	if p.Bin {
		return p.binary()
	}
	if tok.Type == lexer.PROGRAM_FILE {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.IDENT {
			return p.Errorf("expected an entry point, got %q", tok.Literal)
		}
		p.Entry = tok.Literal
	}

	// For library and program, we create a single PIC org before moving to section parser.
	o := Org{PIC: true, Addr: 0}
	p.Ast.Orgs = append(p.Ast.Orgs, o)

	return p.programOrLibrary()
}

func (p *Parser) binary() error {
	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}
		if err := p.org(); err != nil {
			return err
		}
	}
	return errEOF
}

func (p *Parser) programOrLibrary() error {
	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}

		tok := p.tokenizer.PeakToken()
		switch tok.Type {
		case lexer.SECTION:
			if err := p.section(); err != nil {
				return err
			}
		case lexer.INCLUDE:
			if err := p.include(); err != nil {
				return err
			}
		case lexer.EMBED:
			if err := p.embed(); err != nil {
				return err
			}
		default:
			return p.Errorf("unexpected token %q", tok.Literal)
		}
	}
	return errEOF
}

func (p *Parser) org() error {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.ORG {
		return p.Errorf("expected .org got %q", tok.Literal)
	}
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.NUMBER {
		return p.Errorf("expected an address constant, got %q\n", tok.Literal)
	}

	n, err := p.parseNumber(tok.Literal)
	if err != nil {
		return err
	}
	o := Org{PIC: false, Addr: n}
	p.Ast.Orgs = append(p.Ast.Orgs, o)

	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}

		tok := p.tokenizer.PeakToken()
		switch tok.Type {
		case lexer.SECTION:
			if err := p.section(); err != nil {
				return err
			}
		case lexer.INCLUDE:
			if err := p.include(); err != nil {
				return err
			}
		case lexer.EMBED:
			if err := p.embed(); err != nil {
				return err
			}
		default:
			return nil
		}
	}
	return errEOF
}

func (p *Parser) section() error {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.SECTION {
		return p.Errorf("expected .section, got %q", tok.Literal)
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.S_DATA && tok.Type != lexer.S_TEXT {
		return p.Errorf("expected data or text, got %q", tok.Literal)
	}

	// For every new .section entry, we reserve memory for 8 blocks.
	o := p.activeOrg()
	s := o.newSection()

	if tok.Type == lexer.S_DATA {
		s.Type = DATA_SECTION
		return p.dataBlock()
	}
	s.Type = TEXT_SECTION
	return p.textBlock()
}

func (p *Parser) dataBlock() error {
	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}

		tok := p.tokenizer.PeakToken()
		if tok.Type.IsTopLevel() {
			return nil
		}

		// Create a new block and add it to the section.
		aBlock := p.activeOrg().activeSection().newBlock()

		switch tok.Type {
		case lexer.IDENT:
			tok := p.tokenizer.NextToken()
			label := tok.Literal
			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.COLON {
				return p.Errorf("expected \"%s:\", got \"%s%s\"", label, label, tok.Literal)
			}
			aBlock.Label = label
		}
		if err := p.parseDataEntries(aBlock); err != nil {
			return err
		}
	}
}

func (p *Parser) textBlock() error {
	for {
		aBlock := p.activeOrg().activeSection().activeBlock()
		if err := p.skipCommentsAndWhitespace(); err != nil {
			if err == errEOF && (aBlock != nil && aBlock.inFunc) {
				return p.Errorf("function %q was not closed.", aBlock.funcName)
			}
			return err
		}

		tok := p.tokenizer.PeakToken()
		if tok.Type.IsTopLevel() {
			if aBlock == nil || len(aBlock.Statements) == 0 {
				return p.Errorf("empty data sections are not allowed")
			}
			if aBlock.inFunc {
				return p.Errorf(
					"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			}
			return nil
		}

		// Create a new text block
		prevBlock := aBlock
		aSection := p.activeOrg().activeSection()
		aBlock = aSection.newBlock()

		// If this is not the first block, check if it is part of a function context.
		if prevBlock != nil && prevBlock.inFunc {
			aBlock.inFunc = prevBlock.inFunc
			aBlock.funcName = prevBlock.funcName
		}

		tok = p.tokenizer.NextToken()
		switch tok.Type {
		case lexer.FUNC_START:
			if err := p.funcStart(aBlock, true); err != nil {
				return err
			}
		case lexer.INFUNC_START:
			if err := p.funcStart(aBlock, false); err != nil {
				return err
			}
		case lexer.FUNC_END:
			if err := p.funcEnd(aBlock); err != nil {
				return err
			}
			continue
		case lexer.IDENT:
			label := tok.Literal
			tok := p.tokenizer.NextToken()
			if tok.Type != lexer.COLON {
				return p.Errorf("expected a ':', got %q", tok.Literal)
			}
			aBlock.Label = label
		default:
			return p.Errorf("invalid token %q", tok.Literal)
		}
		if err := p.parseInstructions(aBlock); err != nil {
			return err
		}
	}
}

func (p *Parser) parseDataEntries(block *Block) error {
	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}
		tok := p.tokenizer.PeakToken()
		if tok.Type.IsTopLevel() || tok.Type == lexer.IDENT {
			return nil
		}
		tok = p.tokenizer.NextToken()

		switch tok.Type {
		case lexer.ARRAY_TYPE:
			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.NUMBER {
				return p.Errorf("expected a number, got %q", tok.Literal)
			}
			n, err := p.parseNumber(tok.Literal)
			if err != nil {
				return err
			}
			if n <= 0 {
				return p.Errorf("expected array with positive value, got %d", n)
			}

			block.Statements = append(block.Statements, Statement{
				ArraySize: int(n),
				lineNum:   p.tokenizer.Line(),
			})

		case lexer.STRING_TYPE:
			str, err := p.readString()
			if err != nil {
				return err
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
				return err
			}

			block.Statements = append(block.Statements, Statement{
				Value:   n,
				lineNum: p.tokenizer.Line(),
			})
		case lexer.EQUATE:
			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.IDENT {
				return p.Errorf("expected an identifier, got %q", tok.Literal)
			}
			constant := tok.Literal
			if _, ok := p.Ast.Consts[constant]; ok {
				return p.Errorf("constant %q was previously defined.", constant)
			}

			tok = p.tokenizer.NextToken()
			if tok.Type != lexer.NUMBER {
				return p.Errorf("epxected a number, got %q", tok.Literal)
			}
			p.Ast.Consts[constant] = tok.Literal
		default:
			return p.Errorf("expected either a label, value or a constant definition, got %q", tok.Literal)
		}
	}
}

func (p *Parser) funcStart(block *Block, exported bool) error {
	if block.inFunc {
		return p.Errorf("Cannot start a new function within an existing function.")
	}
	block.inFunc = true
	block.Exported = exported

	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT {
		return p.Errorf("Expected a name for the function, got %q", tok.Literal)
	}
	block.Label = tok.Literal
	block.funcName = block.Label
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.COLON {
		return p.Errorf("expected ':', got %q", tok.Literal)
	}
	return nil
}

func (p *Parser) funcEnd(block *Block) error {
	if !block.inFunc {
		return p.Errorf("cannot end a function that was not started.")
	}

	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT || tok.Literal != block.funcName {
		return p.Errorf("expected %q, the name of the function; got %q", block.funcName, tok.Literal)
	}

	// The last statement of a function must be an unambiguous control transfer.
	// That means it must either be a "ret", "jmp" or a "halt". "call" is not allowed
	// because it can return back to the function. For the same reason, condition jumps
	// ("jne, "jeq", etc...) are also not allowed.
	if len(block.Statements) == 0 {
		return p.Errorf("function cannot be empty.")
	}
	instr := &block.Statements[len(block.Statements)-1].Instr
	if instr.Name == "" || (instr.Name != "ret" && instr.Name != "jmp" && instr.Name != "halt") {
		return p.Errorf("function %q must end with either a 'ret', 'jmp' or 'halt' instruction.",
			block.funcName)
	}
	block.inFunc = false
	return nil
}

func (p *Parser) include() error {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.INCLUDE {
		return p.Errorf("expected .include, got %q", tok.Literal)
	}

	includeStr, err := p.readString()
	if err != nil {
		return err
	}
	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.AS {
		return p.Errorf("expected as, got %q", tok.Literal)
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT {
		return p.Errorf("expected an identifier, got %q", tok.Literal)
	}

	if _, ok := p.Ast.Includes[tok.Literal]; ok {
		return p.Errorf("include name %q was already defined.", tok.Literal)
	}
	p.Ast.Includes[tok.Literal] = includeStr

	o := p.activeOrg()

	// Ok, we have an include. Try to read the file.
	in, err := os.Open(includeStr)
	if err != nil {
		return err
	}
	defer in.Close()

	dir, _ := filepath.Split(includeStr)
	if dir != "" {
		// We need to change the current working directory in order for the include to be
		// parsed correctly. So, we save the cwd, set the new cwd and then restore once we are
		// done processing.
		curWD, err := os.Getwd()
		if err != nil {
			return err
		}
		if err := os.Chdir(dir); err != nil {
			panic(err)
		}
		defer func() {
			if err := os.Chdir(curWD); err != nil {
				panic(err)
			}
		}()
	}

	// Parse the file, producing an AST.
	ast, err := Parse(in, true)
	if err != nil {
		return err
	}

	// Set the include name on each section.
	for _, org := range ast.Orgs {
		if org.Sections == nil {
			continue
		}
		for s := org.Sections; ; s = s.Next {
			s.IncludeName = tok.Literal
			if s.Next == org.Sections {
				break
			}
		}
	}

	// Ok, parsing was sucessful, so now we include the sections here.
	o.linkSection(ast.Orgs[0].Sections)
	return nil
}

func (p *Parser) skipCommentsAndWhitespace() error {
	for {
		tok := p.tokenizer.PeakToken()
		if tok.Type == lexer.EOF {
			return errEOF
		}
		if tok.Type != lexer.SEMICOLON && tok.Type != lexer.NEWLINE {
			return nil
		}
		if tok.Type == lexer.SEMICOLON {
			for tok.Type != lexer.NEWLINE && tok.Type != lexer.EOF {
				tok = p.tokenizer.NextToken()
			}
		} else {
			p.tokenizer.NextToken()
		}
	}
	return errEOF
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

func (p *Parser) embed() error {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.EMBED {
		return p.Errorf("expected .embed, got %q", tok.Literal)
	}

	embedStr, err := p.readString()
	if err != nil {
		return err
	}

	o := p.activeOrg()

	// 1. Read the file.
	in, err := os.Open(embedStr)
	if err != nil {
		return err
	}
	defer in.Close()

	bytes, err := ioutil.ReadAll(in)
	if err != nil {
		return err
	}

	section := o.newSection()
	section.Type = DATA_SECTION
	section.Blocks = []Block{
		{
			Statements: []Statement{
				{
					Blob: bytes,
				},
			},
		},
	}
	return nil
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

func (p *Parser) parseInstructions(block *Block) error {
	for {
		if err := p.skipCommentsAndWhitespace(); err != nil {
			return err
		}
		tok := p.tokenizer.PeakToken()
		if tok.Type.IsTopLevel() || tok.Type == lexer.IDENT ||
			tok.Type == lexer.FUNC_START || tok.Type == lexer.INFUNC_START {
			return nil
		}
		tok = p.tokenizer.NextToken()

		if tok.Type == lexer.FUNC_END {
			return p.funcEnd(block)
		}
		if tok.Type != lexer.INSTRUCTION {
			return p.Errorf("expected an instruction, got %q", tok.Literal)
		}

		// Instructions either have 0-4 operands. ldr and str and variations have brakets around
		// one of the registers so we need to account
		// for them.
		instr := &Instruction{Name: tok.Literal}
		opCount, ok := operands[instr.Name]
		if !ok {
			return p.Errorf("instruction not present in operand table: %q", instr.Name)
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
			return err
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
			return err
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
				return err
			}
			if opCount > 3 {
				instr.Op4, err = p.parseOperand(false)
			}
		}
		if err != nil {
			return err
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

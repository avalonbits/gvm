package parser

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/avalonbits/gsm/lexer"
)

type Word uint32
type AST struct {
	Orgs   []Org
	consts map[string]Word
}

type Org struct {
	Addr     uint32
	Sections []Section
}

func (o Org) WordCount() uint32 {
	var count uint32
	for _, s := range o.Sections {
		count += s.wordCount()
	}
	return count
}

type SType int

const (
	SECTION_DATA SType = iota
	SECTION_TEXT
)

type Statement struct {
	Value uint32
	Instr Instruction
}

type Instruction struct {
	name string
	op1  string
	op2  string
	op3  string
}

type Block struct {
	Label      string
	Statements []Statement
}
type Section struct {
	Blocks []Block
}

func (s Section) wordCount() uint32 {
	return 0
}

type Tokenizer interface {
	NextToken() lexer.Token
	PeakToken() lexer.Token
}

type Parser struct {
	tokenizer Tokenizer
	err       error
	Ast       *AST
}

func New(t Tokenizer) *Parser {
	return &Parser{tokenizer: t, Ast: &AST{
		consts: make(map[string]Word),
	}}
}

type state int

const (
	START state = iota
	SECTION
	DATA_BLOCK
	TEXT_BLOCK
	ERROR
	END
)

func (p *Parser) Parse() error {
	st := p.skipCommentsAndWhitespace(START)
	for ; st != END; st = p.skipCommentsAndWhitespace(st) {
		switch st {
		case START:
			st = p.org()
		case SECTION:
			st = p.section()
		case DATA_BLOCK:
			st = p.data_block(st)
		case TEXT_BLOCK:
			st = p.text_block(st)
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

func (p *Parser) org() state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.ORG {
		p.err = fmt.Errorf("expected .org, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.NUMBER {
		p.err = fmt.Errorf("expected an address constant, got %q\n", tok.Literal)
		return ERROR
	}

	n, err := parseNumber(tok.Literal)
	if err != nil {
		p.err = err
		return ERROR
	}

	o := Org{Addr: n}
	p.Ast.Orgs = append(p.Ast.Orgs, o)
	return SECTION
}

func (p *Parser) section() state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.SECTION {
		p.err = fmt.Errorf("expected .section, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.S_DATA && tok.Type != lexer.S_TEXT {
		p.err = fmt.Errorf("expected data or text, got %q", tok.Literal)
		return ERROR
	}

	s := Section{Blocks: make([]Block, 1, 4)}
	var next state
	if tok.Type == lexer.S_DATA {
		next = DATA_BLOCK
	} else {
		next = TEXT_BLOCK
	}
	o := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	o.Sections = append(o.Sections, s)
	return next
}

func (p *Parser) data_block(cur state) state {
	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		return START
	}
	if tok.Type == lexer.SECTION {
		return SECTION
	}

	// Get the active block.
	aOrg := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	aSection := &aOrg.Sections[len(aOrg.Sections)-1]
	aBlock := &aSection.Blocks[len(aSection.Blocks)-1]

	tok = p.tokenizer.NextToken()
	switch tok.Type {
	case lexer.IDENT:
		label := tok.Literal
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COLON {
			p.err = fmt.Errorf("expected \"%s:\", got \"%s%s\"",
				label, label, tok.Literal)
			return ERROR
		}

		// If the current block has no instructions, then we can reuse the block.
		if len(aBlock.Statements) == 0 {
			aBlock.Label = label
		} else {
			aSection.Blocks = append(aSection.Blocks, Block{Label: label})
		}
		return DATA_BLOCK
	case lexer.INT_TYPE:
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.IDENT {
			p.err = fmt.Errorf("exptected a name for the constant, got %q.", tok.Literal)
			return ERROR
		}

		name := tok.Literal
		tok = p.tokenizer.NextToken()
		n, err := parseNumber(tok.Literal)
		if err != nil {
			p.err = err
			return ERROR
		}

		if _, ok := p.Ast.consts[name]; ok {
			p.err = fmt.Errorf("constant %q was defined earlier in the file", name)
			return ERROR
		}
		p.Ast.consts[name] = Word(n)
		aBlock.Statements = append(aBlock.Statements, Statement{Value: n})
		return DATA_BLOCK
	default:
		p.err = fmt.Errorf("expected either a label or a constant definition, got %q", tok.Literal)
		return ERROR
	}
}

func (p *Parser) text_block(cur state) state {
	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		return START
	}
	if tok.Type == lexer.SECTION {
		return SECTION
	}

	// Get the active block.
	aOrg := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	aSection := &aOrg.Sections[len(aOrg.Sections)-1]
	aBlock := &aSection.Blocks[len(aSection.Blocks)-1]

	tok = p.tokenizer.NextToken()
	if tok.Type == lexer.IDENT {
		label := tok.Literal
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COLON {
			p.err = fmt.Errorf("expected \"%s:\", got \"%s%s\"",
				label, label, tok.Literal)
			return ERROR
		}

		// If the current block has no instructions, then we can reuse the block.
		if len(aBlock.Statements) == 0 {
			aBlock.Label = label
		} else {
			aSection.Blocks = append(aSection.Blocks, Block{Label: label})
		}
		return TEXT_BLOCK
	}

	if tok.Type != lexer.INSTRUCTION {
		p.err = fmt.Errorf("expected an instruction, got %q", tok.Literal)
		return ERROR
	}
	return p.parseInstruction(aBlock, tok)
}

func parseNumber(lit string) (uint32, error) {
	var n uint64
	var err error
	if strings.HasPrefix(lit, "0x") || strings.HasPrefix(lit, "0X") {
		if n, err = strconv.ParseUint(lit[2:], 16, 32); err != nil {
			return 0, fmt.Errorf("Expected a 32 bit hexadecimal number, got %q: %v",
				lit, err)
		}
	} else if strings.HasPrefix(lit, "0") {
		if n, err = strconv.ParseUint(lit[1:], 0, 32); err != nil {
			return 0, fmt.Errorf("expected a 32 bit octal number, got %q: %v", lit, err)
		}
	} else if n, err = strconv.ParseUint(lit, 10, 32); err != nil {
		return 0, fmt.Errorf("expected a 32 bit decimal number, got %q: %v", lit, err)
	}

	return uint32(n), nil
}

var (
	operands = map[string]int{
		"add":  3,
		"sub":  3,
		"lsl":  3,
		"lsr":  3,
		"asr":  3,
		"and":  3,
		"orr":  3,
		"xor":  3,
		"mov":  2,
		"str":  2,
		"ldr":  2,
		"jeq":  2,
		"jne":  2,
		"jlt":  2,
		"jle":  2,
		"jgt":  2,
		"jge":  2,
		"call": 1,
		"jmp":  1,
		"halt": 0,
		"nop":  0,
		"ret":  0,
	}
)

func (p *Parser) parseInstruction(block *Block, tok lexer.Token) state {
	// Instructions either have 0-3 operands. ldr and str have brakets around
	// one of the registers so we need to account
	// for them.
	instr := &Instruction{name: tok.Literal}
	opCount, ok := operands[instr.name]
	if !ok {
		p.err = fmt.Errorf("instruction not present in operand table: %q", instr.name)
		return ERROR
	}

	s := Statement{Instr: *instr}
	block.Statements = append(block.Statements, s)

	if opCount == 0 {
		return TEXT_BLOCK
	}
	instr = &block.Statements[len(block.Statements)-1].Instr

	var err error
	if instr.name == "str" {
		instr.op1, err = p.parseAddressOperand(true)
	} else {
		instr.op1, err = p.parseOperand(opCount > 1)
	}
	if err != nil {
		p.err = err
		return ERROR
	}

	if opCount == 1 {
		return TEXT_BLOCK
	}

	if instr.name == "ldr" {
		instr.op2, err = p.parseAddressOperand(false)
	} else {
		instr.op2, err = p.parseOperand(opCount == 3)
	}
	if err != nil {
		p.err = err
		return ERROR
	}

	if opCount == 2 {
		return TEXT_BLOCK
	}

	instr.op3, err = p.parseOperand(false)
	if err != nil {
		p.err = err
		return ERROR
	}

	return TEXT_BLOCK
}

func (p *Parser) parseOperand(comma bool) (string, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.REGISTER && tok.Type != lexer.IDENT && tok.Type != lexer.NUMBER {
		return "", fmt.Errorf("expected a register, a number or a label, got %q",
			tok.Literal)
	}
	op := tok.Literal
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return "", fmt.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil

}

func (p *Parser) parseAddressOperand(comma bool) (string, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.L_BRACKET {
		return "", fmt.Errorf("expected a left bracket '[', got %q", tok.Literal)
	}

	op, err := p.parseOperand(false)
	if err != nil {
		return "", err
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.R_BRACKET {
		return "", fmt.Errorf("expected a right bracket ']', got %q", tok.Literal)
	}
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return "", fmt.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil
}

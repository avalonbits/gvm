package parser

import (
	"fmt"
	"strconv"
	"strings"
	"unicode/utf8"

	"github.com/avalonbits/gsm/lexer"
)

type Word uint32
type AST struct {
	Orgs   []Org
	Consts map[string]string
}

type Org struct {
	Addr     uint32
	Sections []Section
}

func (o Org) WordCount() int {
	var count int
	for _, s := range o.Sections {
		count += s.wordCount()
	}
	return count
}

type Statement struct {
	Value     uint32
	Instr     Instruction
	Label     string
	ArraySize int
	Str       string
}

func (s Statement) WordCount() int {
	if s.ArraySize > 0 {
		return s.ArraySize / 4
	} else if len(s.Str) > 0 {
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
	OP_REG = iota
	OP_NUMBER
	OP_DIFF
	OP_LABEL
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
}

func (b Block) LabelName() string {
	return b.funcName + b.Label
}

func (b Block) JumpName(label string) string {
	return b.funcName + label
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
}

type Parser struct {
	tokenizer Tokenizer
	err       error
	Ast       *AST
}

func New(t Tokenizer) *Parser {
	t.IgnoreWhiteSpace(true)
	return &Parser{tokenizer: t, Ast: &AST{
		Consts: make(map[string]string),
	}}
}

type state int

const (
	START state = iota
	SECTION
	EMBED_STATEMENT
	INCLUDE_STATEMENT
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
		case EMBED_STATEMENT:
			st = p.embed()
		case INCLUDE_STATEMENT:
			st = p.include()
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

	n, err := ParseNumber(tok.Literal)
	if err != nil {
		p.err = err
		return ERROR
	}

	o := Org{Addr: n}
	p.Ast.Orgs = append(p.Ast.Orgs, o)
	return SECTION
}

func (p *Parser) section() state {
	tok := p.tokenizer.PeakToken()
	if tok.Type != lexer.SECTION && tok.Type != lexer.EMBED && tok.Type != lexer.INCLUDE {
		p.err = fmt.Errorf("expected .section, got %q", tok.Literal)
		return ERROR
	}

	if tok.Type == lexer.EMBED {
		return EMBED_STATEMENT
	}

	if tok.Type == lexer.INCLUDE {
		return INCLUDE_STATEMENT
	}
	p.tokenizer.NextToken()

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.S_DATA && tok.Type != lexer.S_TEXT {
		p.err = fmt.Errorf("expected dat, embed a or text, got %q", tok.Literal)
		return ERROR
	}

	s := Section{Blocks: make([]Block, 1, 4)}
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
		p.err = fmt.Errorf("expected .embed, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.D_QUOTE {
		p.err = fmt.Errorf("expected a double quote (\"), got %q", tok.Literal)
		return ERROR
	}

	var sb strings.Builder
	for {
		tok = p.tokenizer.NextToken()
		if tok.Type == lexer.NEWLINE {
			p.err = fmt.Errorf("expected a double quote(\"), got a new line")
			return ERROR
		}
		if tok.Type == lexer.D_QUOTE {
			break
		}
		sb.WriteString(tok.Literal)
	}

	s := Section{Type: EMBED_FILE, EmbedFile: sb.String()}
	o := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	o.Sections = append(o.Sections, s)
	return SECTION
}

func (p *Parser) include() state {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.INCLUDE {
		p.err = fmt.Errorf("expected .include, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.D_QUOTE {
		p.err = fmt.Errorf("expected a double quote (\"), got %q", tok.Literal)
		return ERROR
	}

	var sb strings.Builder
	for {
		tok = p.tokenizer.NextToken()
		if tok.Type == lexer.NEWLINE {
			p.err = fmt.Errorf("expected a double quote(\"), got a new line")
			return ERROR
		}
		if tok.Type == lexer.D_QUOTE {
			break
		}
		sb.WriteString(tok.Literal)
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.AS {
		p.err = fmt.Errorf("expected as, got %q", tok.Literal)
		return ERROR
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.IDENT {
		p.err = fmt.Errorf("expected an identifier, got %q", tok.Literal)
		return ERROR
	}

	s := Section{
		Type:        INCLUDE_FILE,
		IncludeFile: sb.String(),
		IncludeName: tok.Literal,
	}
	o := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	o.Sections = append(o.Sections, s)
	return SECTION
}

func (p *Parser) data_block(cur state) state {
	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		return START
	}
	if tok.Type == lexer.SECTION {
		return SECTION
	}

	if tok.Type == lexer.EMBED {
		return EMBED_STATEMENT
	}
	if tok.Type == lexer.INCLUDE {
		return INCLUDE_STATEMENT
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
	case lexer.ARRAY_TYPE:
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.NUMBER {
			p.err = fmt.Errorf("expected a number, got %q", tok.Literal)
			return ERROR
		}
		n, err := ParseNumber(tok.Literal)
		if err != nil {
			p.err = err
			return ERROR
		}
		if n > 0 && n%4 != 0 {
			n += (4 - (n % 4))
		}

		aBlock.Statements = append(aBlock.Statements, Statement{ArraySize: int(n)})
		return DATA_BLOCK

	case lexer.STRING_TYPE:
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.D_QUOTE {
			p.err = fmt.Errorf("expected '\"', got %q", tok.Literal)
			return ERROR
		}

		var sb strings.Builder
		for {
			tok = p.tokenizer.NextToken()
			if tok.Type == lexer.NEWLINE {
				p.err = fmt.Errorf("epxected '\"', got a new line.")
				return ERROR
			}
			if tok.Type == lexer.D_QUOTE {
				break
			}
			sb.WriteString(tok.Literal)
		}
		aBlock.Statements = append(aBlock.Statements, Statement{Str: sb.String()})
		return DATA_BLOCK

	case lexer.INT_TYPE:
		tok = p.tokenizer.NextToken()
		if tok.Type == lexer.IDENT {
			// This is likely a label the user wants to store the adress from.
			aBlock.Statements = append(aBlock.Statements, Statement{Label: tok.Literal})
			return DATA_BLOCK
		}

		n, err := ParseNumber(tok.Literal)
		if err != nil {
			p.err = err
			return ERROR
		}

		aBlock.Statements = append(aBlock.Statements, Statement{Value: n})
		return DATA_BLOCK
	case lexer.EQUATE:
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.IDENT {
			p.err = fmt.Errorf("expected an identifier, got %q", tok.Literal)
			return ERROR
		}
		constant := tok.Literal
		if _, ok := p.Ast.Consts[constant]; ok {
			p.err = fmt.Errorf("constant %q was previously defined.", constant)
			return ERROR
		}

		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.NUMBER {
			p.err = fmt.Errorf("epxected a number, got %q", tok.Literal)
			return ERROR
		}
		p.Ast.Consts[constant] = tok.Literal

		return DATA_BLOCK
	default:
		p.err = fmt.Errorf("expected either a label or a constant definition, got %q", tok.Literal)
		return ERROR
	}
}

func (p *Parser) text_block(cur state) state {
	// Get the active block.
	aOrg := &p.Ast.Orgs[len(p.Ast.Orgs)-1]
	aSection := &aOrg.Sections[len(aOrg.Sections)-1]
	aBlock := &aSection.Blocks[len(aSection.Blocks)-1]

	tok := p.tokenizer.PeakToken()
	if tok.Type == lexer.ORG {
		if aBlock.inFunc {
			p.err = fmt.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}
		return START
	}
	if tok.Type == lexer.SECTION {
		if aBlock.inFunc {
			p.err = fmt.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}

		return SECTION
	}
	if tok.Type == lexer.EMBED || tok.Type == lexer.INCLUDE {
		if aBlock.inFunc {
			p.err = fmt.Errorf(
				"expected function end for %q, got %q", aBlock.funcName, tok.Literal)
			return ERROR
		}
		if tok.Type == lexer.EMBED {
			return EMBED_STATEMENT
		}
		return INCLUDE_STATEMENT
	}

	inFunc := aBlock.inFunc
	tok = p.tokenizer.NextToken()
	inFuncScope :=
		tok.Type == lexer.FUNC_START || tok.Type == lexer.INFUNC_START ||
			tok.Type == lexer.FUNC_END
	if tok.Type == lexer.FUNC_START || tok.Type == lexer.INFUNC_START {
		tok = p.tokenizer.NextToken()
		if inFunc {
			p.err = fmt.Errorf("expected function end for %q, got function start for %q",
				aBlock.funcName, tok.Literal)
			return ERROR
		}
		inFunc = true
	} else if tok.Type == lexer.FUNC_END {
		tok = p.tokenizer.NextToken()
		if !aBlock.inFunc {
			p.err = fmt.Errorf(
				"found @endf for %q, but %q is not @func.", tok.Literal, tok.Literal)
			return ERROR
		}
		inFunc = false
	}

	if tok.Type == lexer.IDENT {
		label := tok.Literal
		if !inFunc && aBlock.inFunc {
			if label != aBlock.funcName {
				p.err = fmt.Errorf("expected @endf %v, got @endf %v", aBlock.funcName, label)
				return ERROR
			}

			// At this point we know we reached the end of a block, so we need to create
			// a new one.
			aSection.Blocks = append(aSection.Blocks, Block{})
			return TEXT_BLOCK
		}

		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COLON {
			p.err = fmt.Errorf("expected \"%s:\", got \"%s%s\"",
				label, label, tok.Literal)
			return ERROR
		}

		// If the current block has no instructions, then we can reuse the block.
		if len(aBlock.Statements) == 0 {
			aBlock.inFunc = inFunc
			if inFunc {
				aBlock.funcName = label
			} else {
				aBlock.Label = label
			}
		} else {
			b := Block{inFunc: inFunc}
			if inFuncScope && inFunc {
				b.funcName = label
			} else if inFunc {
				b.funcName = aBlock.funcName
				b.Label = label
			} else {
				b.Label = label
			}
			aSection.Blocks = append(aSection.Blocks, b)
		}
		return TEXT_BLOCK
	} else if inFuncScope {
		p.err = fmt.Errorf("Expected an identifier for func, got %q", tok.Literal)
		return ERROR
	}

	if tok.Type != lexer.INSTRUCTION {
		p.err = fmt.Errorf("expected an instruction, got %q", tok.Literal)
		return ERROR
	}
	return p.parseInstruction(aBlock, tok)
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

func (p *Parser) parseInstruction(block *Block, tok lexer.Token) state {
	// Instructions either have 0-4 operands. ldr and str and variations have brakets around
	// one of the registers so we need to account
	// for them.
	instr := &Instruction{Name: tok.Literal}
	opCount, ok := operands[instr.Name]
	if !ok {
		p.err = fmt.Errorf("instruction not present in operand table: %q", instr.Name)
		return ERROR
	}

	s := Statement{Instr: *instr}
	block.Statements = append(block.Statements, s)

	if opCount == 0 {
		return TEXT_BLOCK
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
		return TEXT_BLOCK
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
		return TEXT_BLOCK
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

	return TEXT_BLOCK
}

func (p *Parser) parseOperand(comma bool) (Operand, error) {
	op := Operand{}
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.REGISTER && tok.Type != lexer.IDENT && tok.Type != lexer.NUMBER {
		return op, fmt.Errorf("expected a register, a number or a label, got %q",
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
			return op, fmt.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil

}

func (p *Parser) parseAddressOperand(comma bool) (Operand, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.L_BRACKET {
		return Operand{}, fmt.Errorf("expected a left bracket '[', got %q", tok.Literal)
	}

	op, err := p.parseOperand(false)
	if err != nil {
		return op, err
	}

	tok = p.tokenizer.NextToken()
	if tok.Type != lexer.R_BRACKET {
		return op, fmt.Errorf("expected a right bracket ']', got %q", tok.Literal)
	}
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return op, fmt.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op, nil
}

func (p *Parser) parseIndexOperand(comma bool) (Operand, Operand, error) {
	tok := p.tokenizer.NextToken()
	if tok.Type != lexer.L_BRACKET {
		return Operand{}, Operand{},
			fmt.Errorf("expected a left bracket '[', got %q", tok.Literal)
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
			fmt.Errorf("expected a right bracket ']', got %q", tok.Literal)
	}
	if comma {
		tok = p.tokenizer.NextToken()
		if tok.Type != lexer.COMMA {
			return Operand{}, Operand{},
				fmt.Errorf("expected a comma ',', got %q", tok.Literal)
		}
	}
	return op1, op2, nil
}

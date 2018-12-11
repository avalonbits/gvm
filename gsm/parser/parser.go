package parser

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/avalonbits/gsm/lexer"
)

type AST struct {
	Orgs []Org
}

type Org struct {
	Addr    uint32
	Section []Section
}

func (o Org) WordCount() uint32 {
	var count uint32
	for _, s := range o.Section {
		count += s.wordCount()
	}
	return count
}

type SType int

const (
	SECTION_DATA SType = iota
	SECTION_TEXT
)

type Section struct {
	sType SType
	//statements []Statement
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
	return &Parser{tokenizer: t, Ast: &AST{}}
}

type state int

const (
	START state = iota
	SECTION
	ERROR
	END
)

func (p *Parser) Parse() error {
	st := START
	for st = p.skipCommentsAndWhitespace(st); st != END; {
		switch st {
		case START:
			st = p.org()
		case SECTION:
			st = p.section()
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

func parseNumber(lit string) (uint32, error) {
	var n uint64
	var err error
	if strings.HasPrefix(lit, "0x") {
		if n, err = strconv.ParseUint(lit[2:], 16, 32); err != nil {
			return 0, fmt.Errorf("Expected a 32 bit hexadecimal number, got %v: %v",
				lit, err)
		}
	} else if strings.HasPrefix(lit, "0") {
		if n, err = strconv.ParseUint(lit[1:], 0, 32); err != nil {
			return 0, fmt.Errorf("expected a 32 bit octal number, got %v: %v", lit, err)
		}
	} else if n, err = strconv.ParseUint(lit, 10, 32); err != nil {
		return 0, fmt.Errorf("expected a 32 bit decimal number, got %v: %v", lit, err)
	}

	return uint32(n), nil
}

package parser

import (
	"fmt"
	"log"

	"github.com/avalonbits/gsm/lexer"
)

type Tokenizer interface {
	NextToken() lexer.Token
	PeakToken() lexer.Token
}

type Parser struct {
	tokenizer Tokenizer
	err       error
}

func New(t Tokenizer) *Parser {
	return &Parser{tokenizer: t}
}

type state int

const (
	START state = iota
	ERROR
	END
)

func (p *Parser) Parse() error {
	st := START
	for st = p.skipCommentsAndWhitespace(st); st != END; {
		switch st {
		case START:
			st = p.org()
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
				log.Println(tok.Literal)
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

	fmt.Println(".org", tok.Literal)
	return START
}

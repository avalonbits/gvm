package lexer

import (
	"bufio"
	"fmt"
	"io"
	"strings"
	"unicode"
)

type TokenType string

type Token struct {
	Type    TokenType
	Literal string
}

func (t Token) String() string {
	return fmt.Sprintf("%s(%s)", t.Type, t.Literal)
}

const (
	ILLEGAL   TokenType = "ILLEGAL"
	EOF                 = "EOF"
	COMMA               = ","
	SEMICOLON           = ";"
	NEWLINE             = "\n"
	COLON               = ":"

	SECTION     TokenType = ".SECTION"
	S_DATA                = "DATA"
	S_TEXT                = "TEXT"
	ORG                   = ".ORG"
	LABEL                 = "LABEL"
	INT_TYPE              = ".INT"
	IDENT                 = "IDENT"
	NUMBER                = "NUMBER"
	REGISTER              = "REGISTER"
	INSTRUCTION           = "INSTRUCTION"
)

var keywords = map[string]TokenType{
	".section": SECTION,
	"data":     S_DATA,
	"text":     S_TEXT,
	".org":     ORG,
	".int":     INT_TYPE,
	"mov":      INSTRUCTION,
	"add":      INSTRUCTION,
	"sub":      INSTRUCTION,
	"jmp":      INSTRUCTION,
	"jeq":      INSTRUCTION,
	"jne":      INSTRUCTION,
	"jlt":      INSTRUCTION,
	"jle":      INSTRUCTION,
	"jgt":      INSTRUCTION,
	"jge":      INSTRUCTION,
	"and":      INSTRUCTION,
	"orr":      INSTRUCTION,
	"xor":      INSTRUCTION,
	"ldr":      INSTRUCTION,
	"str":      INSTRUCTION,
	"call":     INSTRUCTION,
	"ret":      INSTRUCTION,
	"lsl":      INSTRUCTION,
	"lsr":      INSTRUCTION,
	"asr":      INSTRUCTION,
	"halt":     INSTRUCTION,
	"nop":      INSTRUCTION,
}

func lookupIdent(ident string) TokenType {
	if tok, ok := keywords[ident]; ok {
		return tok
	}
	return IDENT
}

type Lexer struct {
	buf *bufio.Reader
	r   rune
	tok *Token
}

func New(r io.Reader) *Lexer {
	return &Lexer{buf: bufio.NewReader(r)}
}

func (l *Lexer) readRune() {
	r, sz, err := l.buf.ReadRune()
	if sz == 0 || err != nil {
		l.r = 0
	} else {
		l.r = r
	}
}

func (l *Lexer) skipWhitespace() {
	for unicode.IsSpace(l.r) && l.r != '\n' {
		l.readRune()
	}
}

func (l *Lexer) PeakToken() Token {
	if l.tok == nil {
		l.tok = l.nextToken()
	}
	return *l.tok

}

func (l *Lexer) NextToken() Token {
	t := l.tok
	if t != nil {
		l.tok = nil
	} else {
		t = l.nextToken()
	}
	return *t
}

func (l *Lexer) nextToken() *Token {
	l.readRune()
	l.skipWhitespace()
	ch := string(l.r)
	switch l.r {
	case ',':
		return newTok(COMMA, ch)
	case ';':
		return newTok(SEMICOLON, ch)
	case ':':
		return newTok(COLON, ch)
	case '\n':
		return newTok(NEWLINE, ch)
	case 0:
		return newTok(EOF, "")
	default:
		if l.r == '.' || unicode.IsLetter(l.r) {
			ident := l.readIdent()
			return newTok(lookupIdent(ident), ident)
		}
		if unicode.IsDigit(l.r) {
			num := l.readNum()
			return newTok(NUMBER, num)
		}
		return newTok(ILLEGAL, "")
	}
}

func newTok(tp TokenType, lit string) *Token {
	return &Token{Type: tp, Literal: lit}
}

func (l *Lexer) readIdent() string {
	var sb strings.Builder
	for unicode.IsLetter(l.r) || unicode.IsDigit(l.r) || l.r == '_' || l.r == '.' {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}

func (l *Lexer) readNum() string {
	var sb strings.Builder
	for unicode.IsDigit(l.r) || l.r == 'x' || (l.r >= 'A' && l.r <= 'F') || (l.r >= 'a' && l.r <= 'f') {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}

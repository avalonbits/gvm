package lexer

import (
	"bufio"
	"fmt"
	"io"
	"strings"
	"unicode"
)

type TokenType int

type Token struct {
	Type    TokenType
	Literal string
}

func (t Token) String() string {
	return fmt.Sprintf("%s(%s)", t.Type, t.Literal)
}

const (
	ILLEGAL TokenType = iota
	EOF
	COMMA
	SEMICOLON
	NEWLINE
	COLON
	L_BRACKET
	R_BRACKET
	F_SLASH
	SECTION
	EMBED
	D_QUOTE
	S_DATA
	S_TEXT
	ORG
	LABEL
	INT_TYPE
	ARRAY_TYPE
	IDENT
	NUMBER
	REGISTER
	INSTRUCTION
)

var keywords = map[string]TokenType{
	".section": SECTION,
	".embed":   EMBED,
	"data":     S_DATA,
	"text":     S_TEXT,
	".org":     ORG,
	".int":     INT_TYPE,
	".array":   ARRAY_TYPE,
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
	"ldri":     INSTRUCTION,
	"ldrpi":    INSTRUCTION,
	"ldrip":    INSTRUCTION,
	"str":      INSTRUCTION,
	"stri":     INSTRUCTION,
	"strpi":    INSTRUCTION,
	"strip":    INSTRUCTION,
	"call":     INSTRUCTION,
	"ret":      INSTRUCTION,
	"lsl":      INSTRUCTION,
	"lsr":      INSTRUCTION,
	"asr":      INSTRUCTION,
	"mul":      INSTRUCTION,
	"div":      INSTRUCTION,
	"mull":     INSTRUCTION,
	"halt":     INSTRUCTION,
	"nop":      INSTRUCTION,
	"r0":       REGISTER,
	"r1":       REGISTER,
	"r2":       REGISTER,
	"r3":       REGISTER,
	"r4":       REGISTER,
	"r5":       REGISTER,
	"r6":       REGISTER,
	"r7":       REGISTER,
	"r8":       REGISTER,
	"r9":       REGISTER,
	"r10":      REGISTER,
	"r11":      REGISTER,
	"r12":      REGISTER,
	"r13":      REGISTER,
	"r14":      REGISTER,
	"r15":      REGISTER,
	"r16":      REGISTER,
	"r17":      REGISTER,
	"r18":      REGISTER,
	"r19":      REGISTER,
	"r20":      REGISTER,
	"r21":      REGISTER,
	"r22":      REGISTER,
	"r23":      REGISTER,
	"r24":      REGISTER,
	"r25":      REGISTER,
	"r26":      REGISTER,
	"r27":      REGISTER,
	"r28":      REGISTER,
	"r29":      REGISTER,
	"r30":      REGISTER,
	"r31":      REGISTER,
	"pc":       REGISTER,
	"sp":       REGISTER,
	"fp":       REGISTER,
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
		l.r = unicode.ToLower(r)
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
	case '[':
		return newTok(L_BRACKET, ch)
	case ']':
		return newTok(R_BRACKET, ch)
	case '\n':
		return newTok(NEWLINE, ch)
	case '/':
		return newTok(F_SLASH, ch)
	case '"':
		return newTok(D_QUOTE, ch)
	case 0:
		return newTok(EOF, "")
	default:
		if l.r == '.' || unicode.IsLetter(l.r) {
			ident := l.readIdent()
			return newTok(lookupIdent(ident), ident)
		}
		if unicode.IsDigit(l.r) || l.r == '-' {
			num := l.readNum()
			return newTok(NUMBER, num)
		}
		return newTok(ILLEGAL, ch)
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
	for unicode.IsDigit(l.r) || l.r == 'x' || l.r == '-' || (l.r >= 'A' && l.r <= 'F') || (l.r >= 'a' && l.r <= 'f') {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}

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

package lexer

import (
	"bufio"
	"crypto/md5"
	"encoding/hex"
	"fmt"
	"hash"
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

func (t TokenType) IsTopLevel() bool {
	return t == ORG || t == SECTION || t == EMBED || t == INCLUDE
}

const (
	ILLEGAL TokenType = iota
	EOF
	COMMA
	SEMICOLON
	NEWLINE
	COLON
	WHITE_SPACE
	L_BRACKET
	R_BRACKET
	F_SLASH
	SECTION
	EMBED
	INCLUDE
	EQUATE
	AS
	D_QUOTE
	S_DATA
	S_TEXT
	ORG
	LABEL
	INT_TYPE
	ARRAY_TYPE
	STRING_TYPE
	IDENT
	NUMBER
	REGISTER
	INSTRUCTION
	FUNC_START
	INFUNC_START
	FUNC_END
	BIN_FILE
	PROGRAM_FILE
	LIBRARY_FILE
)

var keywords = map[string]TokenType{
	"@func":    FUNC_START,
	"@infunc":  INFUNC_START,
	"@endf":    FUNC_END,
	".bin":     BIN_FILE,
	".program": PROGRAM_FILE,
	".library": LIBRARY_FILE,
	".section": SECTION,
	".embed":   EMBED,
	".include": INCLUDE,
	".equ":     EQUATE,
	"as":       AS,
	"data":     S_DATA,
	"text":     S_TEXT,
	".org":     ORG,
	".int":     INT_TYPE,
	".array":   ARRAY_TYPE,
	".str":     STRING_TYPE,
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
	"ldppi":    INSTRUCTION,
	"ldpip":    INSTRUCTION,
	"str":      INSTRUCTION,
	"stri":     INSTRUCTION,
	"strpi":    INSTRUCTION,
	"strip":    INSTRUCTION,
	"stpip":    INSTRUCTION,
	"stppi":    INSTRUCTION,
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
	"wfi":      INSTRUCTION,
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
	"rZ":       REGISTER,
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
	buf      *bufio.Reader
	r        rune
	tok      *Token
	ignoreWS bool
	line     int
	hasher   hash.Hash
}

func (l *Lexer) Line() int {
	return l.line
}

func New(r io.Reader) *Lexer {
	hasher := md5.New()
	tee := io.TeeReader(r, hasher)
	return &Lexer{
		buf:      bufio.NewReader(tee),
		ignoreWS: true,
		line:     1,
		hasher:   hasher,
	}
}

func (l *Lexer) IgnoreWhiteSpace(ignore bool) {
	l.ignoreWS = ignore
}

func (l *Lexer) Hash() string {
	return hex.EncodeToString(l.hasher.Sum(nil))
}

func (l *Lexer) readRune() {
	r, sz, err := l.buf.ReadRune()
	if sz == 0 || err != nil {
		l.r = 0
	} else {
		l.r = r
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
		l.line++
		return newTok(NEWLINE, ch)
	case '/':
		return newTok(F_SLASH, ch)
	case '"':
		return newTok(D_QUOTE, ch)
	case 0:
		return newTok(EOF, "")
	default:
		if unicode.IsSpace(l.r) && l.r != '\n' {
			if l.ignoreWS {
				l.readWhiteSpace()
				return l.nextToken()
			} else {
				return newTok(WHITE_SPACE, l.readWhiteSpace())
			}
		}
		if l.r == '.' || l.r == '@' || l.r == '_' || unicode.IsLetter(l.r) {
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
	for unicode.IsLetter(l.r) || unicode.IsDigit(l.r) || l.r == '_' || l.r == '.' || l.r == '@' {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}

func (l *Lexer) readWhiteSpace() string {
	var sb strings.Builder
	for unicode.IsSpace(l.r) && l.r != '\n' {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}
func (l *Lexer) readNum() string {
	var sb strings.Builder
	for unicode.IsDigit(l.r) || l.r == 'x' || l.r == '-' || (l.r >= 'A' && l.r <= 'F') ||
		(l.r >= 'a' && l.r <= 'f') {
		sb.WriteRune(l.r)
		l.readRune()
	}
	l.buf.UnreadRune()
	return sb.String()
}

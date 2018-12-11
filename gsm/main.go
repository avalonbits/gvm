package main

import (
	"bufio"
	"encoding/binary"
	"flag"
	"os"

	"github.com/avalonbits/gsm/lexer"
	"github.com/avalonbits/gsm/parser"
)

type Rom struct {
	buf *bufio.Writer
}

func (r *Rom) Generate(ast *parser.AST) error {
	r.buf.Write([]byte("1987gvm"))
	word := make([]byte, 4)
	for _, o := range ast.Orgs {
		binary.LittleEndian.PutUint32(word, o.Addr)
		if _, err := r.buf.Write(word); err != nil {
			return err
		}
		binary.LittleEndian.PutUint32(word, o.WordCount())
		if _, err := r.buf.Write(word); err != nil {
			return err
		}
	}
	return nil
}

func (r *Rom) Flush() {
	r.buf.Flush()
}

var (
	outFile = flag.String("o", "a.rom", "Name of output file.")
)

func main() {
	flag.Parse()
	if flag.NArg() < 1 {
		panic("Missing asm file.")
	}

	in, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer in.Close()

	lex := lexer.New(in)
	p := parser.New(lex)
	if err := p.Parse(); err != nil {
		panic(err)
	}

	out, err := os.Create(*outFile)
	if err != nil {
		panic(err)
	}
	defer out.Close()
	rom := &Rom{buf: bufio.NewWriter(out)}
	defer rom.Flush()
	if err := rom.Generate(p.Ast); err != nil {
		panic(err)
	}
	rom.buf.Flush()
}

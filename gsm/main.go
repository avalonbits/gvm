package main

import (
	"bufio"
	"flag"
	"os"

	"github.com/avalonbits/gsm/code"
	"github.com/avalonbits/gsm/lexer"
	"github.com/avalonbits/gsm/parser"
)

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

	if err := code.Generate(p.Ast, bufio.NewWriter(out)); err != nil {
		panic(err)
	}
}

package main

import (
	"bufio"
	"flag"
	"os"

	"github.com/avalonbits/gsm/lexer"
	"github.com/avalonbits/gsm/parser"
)

type Rom struct {
	buf *bufio.Writer
}

var (
	outFile = flag.String("o", "a.rom", "Name of output file.")
)

func main() {
	flag.Parse()

	in, err := os.Open(os.Args[1])
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
	rom.buf.Write([]byte("1987gvm"))
	rom.buf.Flush()
}

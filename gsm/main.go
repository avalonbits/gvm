package main

import (
	"bufio"
	"flag"
	"os"

	"github.com/avalonbits/gsm/code"
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

	ast, err := code.Parse(in)
	if err != nil {
		panic(err)
	}

	out, err := os.Create(*outFile)
	if err != nil {
		panic(err)
	}
	defer out.Close()

	if err := code.Generate(ast, bufio.NewWriter(out)); err != nil {
		panic(err)
	}
}

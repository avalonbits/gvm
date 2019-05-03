package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"
)

var (
	asm = flag.Bool("asm", false, "If true, outputs gvm assembly with a color table.")
	cc  = flag.Bool("cc", false, "if true, outputs a c static array with the color table.")
)

func main() {
	flag.Parse()

	if !*asm && !*cc {
		panic("Pick -asm or -cc")
	}
	inJson, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer inJson.Close()

	outCode, err := os.Create(flag.Arg(1))
	if err != nil {
		panic(err)
	}
	defer outCode.Close()
	out := bufio.NewWriter(outCode)
	defer out.Flush()

	dec := json.NewDecoder(inJson)
	cs := []Color{}
	if err := dec.Decode(&cs); err != nil && err != io.EOF {
		panic(err)
	}
	for i := range cs {
		c := &cs[i]
		c.HexString = "0xFF" + strings.ToUpper(reverse(c.HexString)[:len(c.HexString)-1])
	}
	if *asm {
		for _, c := range cs {
			out.WriteString(fmt.Sprintf("\t.int %s ; %3d - %s\n", c.HexString, c.Id, c.Name))
		}
	} else if *cc {
		out.WriteString("static const uint32_t kColorTable[256] = {")
		for i, c := range cs {
			if i%8 == 0 {
				if i != 0 {
					out.WriteString(", ")
				}
				out.WriteString("\n\t")
			} else {
				out.WriteString(", ")
			}
			out.WriteString(c.HexString)
		}
		out.WriteString("\n};")
	}
}

type Color struct {
	_comment  string `json:"-"`
	Id        int    `json:"colorId"`
	HexString string
	Rgb       string `json:"-"`
	Hsl       string `json:"-"`
	Name      string
}

func reverse(s string) string {
	n := 0
	rune := make([]rune, len(s))
	for _, r := range s {
		rune[n] = r
		n++
	}
	rune = rune[0:n]
	// Reverse
	for i := 0; i < n/2; i++ {
		rune[i], rune[n-1-i] = rune[n-1-i], rune[i]
	}
	// Convert back to UTF-8.
	return string(rune)
}

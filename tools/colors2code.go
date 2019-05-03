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

func main() {
	flag.Parse()
	inJson, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer inJson.Close()

	outAsm, err := os.Create(flag.Arg(1))
	if err != nil {
		panic(err)
	}
	defer outAsm.Close()
	out := bufio.NewWriter(outAsm)
	defer out.Flush()

	dec := json.NewDecoder(inJson)
	cs := []Color{}
	if err := dec.Decode(&cs); err != nil && err != io.EOF {
		panic(err)
	}
	for _, c := range cs {
		c.HexString = "0xFF" + strings.ToUpper(reverse(c.HexString)[:len(c.HexString)-1])
		out.WriteString(fmt.Sprintf("\t.int %s ; %3d - %s\n", c.HexString, c.Id, c.Name))
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

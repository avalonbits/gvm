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
		c.HexString = "0xFF" + strings.ToUpper(reverseHex(c.HexString[1:]))
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

func reverseHex(s string) string {
	return s[4:6] + s[2:4] + s[0:2]
}

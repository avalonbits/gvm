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
	"flag"
	"log"
	"os"
	"path/filepath"

	"github.com/avalonbits/gsm/code"
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

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	out, err := os.Create(*outFile)
	if err != nil {
		panic(err)
	}
	defer out.Close()

	in, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer in.Close()

	// Change the current working directory the the directory of the file.
	dir, _ := filepath.Split(flag.Arg(0))
	if dir != "" {
		if err := os.Chdir(dir); err != nil {
			panic(err)
		}
	}

	ast, err := parser.Parse(in)
	if err != nil {
		panic(err)
	}

	if err := code.GenerateFromObject(ast, bufio.NewWriter(out)); err != nil {
		panic(err)
	}
}

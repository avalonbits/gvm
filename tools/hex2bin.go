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
	"encoding/binary"
	"flag"
	"io"
	"log"
	"os"
	"strconv"
	"strings"
)

var (
	_8x8 = flag.Bool("8x8", false, "If set, will use 8x8 font is being read.")
)

func main() {
	flag.Parse()
	inHex, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer inHex.Close()
	in := bufio.NewReaderSize(inHex, 1024)

	outBin, err := os.Create(flag.Arg(1))
	if err != nil {
		panic(err)
	}
	defer outBin.Close()
	out := bufio.NewWriter(outBin)

	charSize := 16
	if *_8x8 {
		charSize = 8
	}
	defChar := make([]byte, charSize)
	for i := range defChar {
		defChar[i] = 0xFF
	}
	var lastNum = int64(-1)
	for {
		line, err := in.ReadString('\n')
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}

		fields := strings.Split(line, ":")
		num, err := strconv.ParseInt(fields[0], 16, 32)
		if err != nil {
			panic(err)
		}
		repeat := int64(num - lastNum - 1)
		if repeat+lastNum > 0xFFFF {
			repeat = 0xFFFF - lastNum
		}
		if repeat > 0 {
			log.Printf("%d | %x | %x\n", repeat, lastNum, num)
			for i := 0; i < int(repeat); i++ {
				out.Write(defChar)
			}
		}
		writeChar(fields[1][:32], out)
		lastNum = num
	}
	out.Flush()
}

func writeChar(char string, out *bufio.Writer) {
	b := make([]byte, 4)
	for i := 0; i < len(char); i += 8 {
		num, err := strconv.ParseUint(char[i:i+8], 16, 32)
		if err != nil {
			panic(err)
		}
		binary.BigEndian.PutUint32(b, uint32(num))
		out.Write(b)
	}
}

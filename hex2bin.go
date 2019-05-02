package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"flag"
	"io"
	"log"
	"os"
	"strconv"
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

		line, _, err := in.ReadLine()
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}

		fields := bytes.Split(line, []byte{':'})
		num, err := strconv.ParseInt(string(fields[0]), 16, 32)
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
		writeChar(fields[1], out)
		lastNum = num
	}
}

func writeChar(char []byte, out *bufio.Writer) {
	b := make([]byte, 4)
	for i := 0; i < len(char); i += 8 {
		num, err := strconv.ParseUint(string(char[i:i+8]), 16, 32)
		if err != nil {
			panic(err)
		}
		binary.BigEndian.PutUint32(b, uint32(num))
		out.Write(b)
	}
}

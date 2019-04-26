package main

import (
	"bufio"
	"bytes"
	"flag"
	"io"
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
		num, err := strconv.ParseInt(string(fields[0]), 16, 64)
		if err != nil {
			panic(err)
		}
		repeat := int(num - lastNum - 1)
		lastNum = num
		if repeat == 0 {
			writeChar(fields[0], out)
		} else {
			for i := 0; i < repeat; i++ {
				out.Write(defChar)
			}
		}
	}
}

func writeChar(char []byte, out *bufio.Writer) {
}

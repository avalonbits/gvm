package code

import (
	"bufio"
	"encoding/binary"

	"github.com/avalonbits/gsm/parser"
)

func Generate(ast *parser.AST, buf *bufio.Writer) error {
	buf.Write([]byte("1987gvm"))
	word := make([]byte, 4)
	for _, o := range ast.Orgs {
		binary.LittleEndian.PutUint32(word, o.Addr)
		if _, err := buf.Write(word); err != nil {
			return err
		}
		binary.LittleEndian.PutUint32(word, uint32(o.WordCount()))
		if _, err := buf.Write(word); err != nil {
			return err
		}
	}

	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if statement.Instr.Name != "" {
						// This is a statement.
					} else {
						// Raw value.
						binary.LittleEndian.PutUint32(word, statement.Value)
						if _, err := buf.Write(word); err != nil {
							return nil
						}
					}
				}
			}
		}
	}
	return buf.Flush()
}

func statementIterator(ast *parser.AST, fn func(instr parser.Instruction) uint32) {
}

package code

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"strconv"

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

	labelMap := map[string]uint32{}
	if err := convertLabels(labelMap, ast); err != nil {
	}
	if err := convertNames(labelMap, ast); err != nil {
		return err
	}
	if err := rewriteInstructions(ast); err != nil {
		return err
	}
	if err := convertInstructions(ast); err != nil {
		return err
	}
	if err := writeToFile(ast, buf); err != nil {
		return err
	}
	return buf.Flush()
}

func convertLabels(labelMap map[string]uint32, ast *parser.AST) error {
	for _, org := range ast.Orgs {
		baseAddr := org.Addr
		wordCount := uint32(0)
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				if block.Label != "" {
					if _, ok := labelMap[block.Label]; ok {
						return fmt.Errorf("label redefinition: %q", block.Label)
					}
					if _, ok := ast.Consts[block.Label]; ok {
						return fmt.Errorf("label redefinition: %q was defined as a const",
							block.Label)
					}
					labelMap[block.Label] = baseAddr + (wordCount * 4)
				}
				wordCount += uint32(len(block.Statements))
			}
		}
	}
	return nil
}

func convertNames(labelMap map[string]uint32, ast *parser.AST) error {
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if statement.Instr.Name == "" {
						continue
					}
					ops := []*parser.Operand{
						&statement.Instr.Op1,
						&statement.Instr.Op2,
						&statement.Instr.Op3,
					}
					for _, op := range ops {
						err := convertOperand(labelMap, ast.Consts, op)
						if err != nil {
							return fmt.Errorf("error processing instruction %q: $v",
								statement.Instr, err)
						}
					}
				}
			}
		}
	}
	return nil
}

func convertOperand(labelMap map[string]uint32, consts map[string]string, op *parser.Operand) error {
	if op.Type != parser.OP_LABEL {
		return nil
	}
	if value, ok := consts[op.Op]; ok {
		op.Op = value
		return nil
	}
	if value, ok := labelMap[op.Op]; ok {
		op.Op = "0x" + strconv.FormatUint(uint64(value), 16)
		return nil
	}

	return fmt.Errorf("operando %q is not a label or a constant", op.Op)
}

func rewriteInstructions(ast *parser.AST) error {
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
				}
			}
		}
	}
	return nil
}

/*
func convertInstructions(ast *parser.AST) error {
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
				}
			}
		}
	}
	return nil
}
*/

func writeToFile(ast *parser.AST, buf *bufio.Writer) error {
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					/* Raw value.
					binary.LittleEndian.PutUint32(word, statement.Value)
					if _, err := buf.Write(word); err != nil {
						return err
					}*/
				}
			}
		}
	}
	return nil
}

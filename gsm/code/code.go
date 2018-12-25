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
	if err := assignAddresses(labelMap, ast); err != nil {
	}
	if err := convertNames(labelMap, ast); err != nil {
		return err
	}
	if err := rewriteInstructions(ast); err != nil {
		return err
	}
	if err := writeToFile(ast, buf); err != nil {
		return err
	}
	return buf.Flush()
}

func assignAddresses(labelMap map[string]uint32, ast *parser.AST) error {
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
				for i := range block.Statements {
					statement := &block.Statements[i]
					if statement.Instr.Name == "" {
						continue
					}
					ops := []*parser.Operand{
						&statement.Instr.Op1,
						&statement.Instr.Op2,
						&statement.Instr.Op3,
					}
					addr := labelMap[block.Label] + uint32(i*4)
					for _, op := range ops {
						err := convertOperand(
							statement.Instr.Name, addr, labelMap, ast.Consts, op)
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

func convertOperand(instr string, instrAddr uint32, labelMap map[string]uint32, consts map[string]string, op *parser.Operand) error {
	if op.Type != parser.OP_LABEL {
		return nil
	}
	if value, ok := consts[op.Op]; ok {
		op.Op = value
		return nil
	}
	if value, ok := labelMap[op.Op]; ok {
		switch instr {
		case "jmp", "jne", "jeq", "jlt", "jle", "jge", "jgt", "call":
			value -= instrAddr
		}
		op.Op = strconv.FormatInt(int64(int32(value)), 10)
		op.Type = parser.OP_NUMBER
		return nil
	}

	return fmt.Errorf("operando %q is not a label or a constant", op.Op)
}

func rewriteInstructions(ast *parser.AST) error {
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if statement.Instr.Name == "" {
						continue
					}
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
	word := make([]byte, 4)
	for _, org := range ast.Orgs {
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if section.Type == parser.DATA_SECTION {
						binary.LittleEndian.PutUint32(word, statement.Value)
						if _, err := buf.Write(word); err != nil {
							return err
						}
						continue
					}
					w, err := encode(statement.Instr)
					if err != nil {
						return err
					}
					binary.LittleEndian.PutUint32(word, uint32(w))
					if _, err := buf.Write(word); err != nil {
						return err
					}
				}
			}
		}
	}
	return nil
}

func rToI(reg string) uint32 {
	n, err := strconv.Atoi(reg[1:])
	if err != nil {
		panic(err)
	}
	return uint32(n)
}

func encode(i parser.Instruction) (parser.Word, error) {
	switch i.Name {
	case "nop":
		return Nop(), nil
	case "ret":
		return Ret(), nil
	case "halt":
		return Halt(), nil
	case "mov":
		return encode2op(i, MovRR, MovRI)
	case "ldr":
		return encode2op(i, LoadRR, LoadRI)
	case "add":
		return encode3op(i, AddRR, AddRI)
	case "sub":
		return encode3op(i, SubRR, SubRI)
	case "jmp":
		if i.Op1.Type == parser.OP_LABEL {
			return parser.Word(0),
				fmt.Errorf("%q: label substitution was not performed.", i)
		}
		if i.Op1.Type == parser.OP_REG {
			return parser.Word(0), fmt.Errorf("%q: first operand must be an address.", i)
		}
		return Jmp(toNum(i.Op1.Op)), nil
	case "jeq":
		return encodeJumpc(i, Jeq)
	case "jne":
		return encodeJumpc(i, Jne)
	case "jgt":
		return encodeJumpc(i, Jgt)
	case "jge":
		return encodeJumpc(i, Jge)
	case "jlt":
		return encodeJumpc(i, Jlt)
	case "jle":
		return encodeJumpc(i, Jle)
	case "call":
		return encode1op(i, CallR, CallI)
	case "and":
		return encode3op(i, AndRR, AndRI)
	case "orr":
		return encode3op(i, OrrRR, OrrRI)
	case "xor":
		return encode3op(i, XorRR, XorRI)
	case "lsl":
		return encode3op(i, LslRR, LslRI)
	case "lsr":
		return encode3op(i, LsrRR, LsrRI)
	case "asr":
		return encode3op(i, AsrRR, AsrRI)
	}
	return parser.Word(0), fmt.Errorf("Invalid instruction %q", i)
}

type _1op func(uint32) parser.Word
type _2op func(uint32, uint32) parser.Word
type _3op func(uint32, uint32, uint32) parser.Word

func encode1op(i parser.Instruction, rr, ri _1op) (parser.Word, error) {
	if i.Op1.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op1.Type == parser.OP_REG {
		return rr(rToI(i.Op1.Op)), nil
	} else {
		return ri(toNum(i.Op1.Op)), nil
	}
}

func encode2op(i parser.Instruction, rr, ri _2op) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op2.Type == parser.OP_REG {
		return rr(rToI(i.Op1.Op), rToI(i.Op2.Op)), nil
	} else {
		return ri(rToI(i.Op1.Op), toNum(i.Op2.Op)), nil
	}
}

func encode3op(i parser.Instruction, rr, ri _3op) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a register.", i)
	}
	if i.Op3.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op3.Type == parser.OP_REG {
		return rr(rToI(i.Op1.Op), rToI(i.Op2.Op), rToI(i.Op3.Op)), nil
	} else {
		return ri(rToI(i.Op1.Op), rToI(i.Op2.Op), toNum(i.Op3.Op)), nil
	}
}

func encodeJumpc(i parser.Instruction, jump _2op) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op2.Type == parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be an address.", i)
	}
	return jump(rToI(i.Op1.Op), toNum(i.Op2.Op)), nil
}

func toNum(lit string) uint32 {
	n, err := parser.ParseNumber(lit)
	if err != nil {
		panic(err)
	}
	return n
}

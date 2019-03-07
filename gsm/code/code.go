package code

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"os"
	"strconv"

	"github.com/avalonbits/gsm/parser"
)

func Generate(ast *parser.AST, buf *bufio.Writer) error {
	buf.Write([]byte("s1987gvm"))

	if err := embedFile(ast); err != nil {
		return err
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

func embedFile(ast *parser.AST) error {
	for _, org := range ast.Orgs {
		for i := range org.Sections {
			section := &org.Sections[i]
			if section.Type != parser.EMBED_FILE {
				continue
			}

			// Ok, we have a file. Lets convert it to a data section with blocks.

			// 1. Read the file.
			in, err := os.Open(section.EmbedFile)
			if err != nil {
				return err
			}
			defer in.Close()

			stats, err := in.Stat()
			if err != nil {
				return err
			}

			size := stats.Size()
			bytes := make([]byte, size)

			bufr := bufio.NewReader(in)
			if _, err := bufr.Read(bytes); err != nil {
				return err
			}

			// 2. For every 4 bytes, create a block and add it to the current statement.
			section.Blocks = make([]parser.Block, 1)
			block := &section.Blocks[0]
			for i := int64(0); i < size; i += 4 {
				v := binary.LittleEndian.Uint32(bytes[i : i+4])
				block.Statements = append(block.Statements, parser.Statement{Value: v})
			}

			// 3. Change the type and we are good to go!
			section.Type = parser.DATA_SECTION
			section.EmbedFile = ""
		}
	}
	return nil
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

					if statement.Instr.Name != "" {
						// Processing an instruction.
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
								return fmt.Errorf("error processing instruction %q: %v",
									statement.Instr, err)
							}
						}
						continue
					}

					if statement.Label != "" {
						// This is a data entry with a label. Get the address of the label and set it to the statement value.
						addr, ok := labelMap[statement.Label]
						if !ok {
							return fmt.Errorf("label does not exist: %q", statement.Label)
						}
						statement.Label = ""
						statement.Value = addr
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
		op.Type = parser.OP_NUMBER
		return nil
	}
	if value, ok := labelMap[op.Op]; ok {
		switch instr {
		case "jmp", "jne", "jeq", "jlt", "jle", "jge", "jgt", "call":
			value -= instrAddr
		case "ldr", "str":
			if value&0xFFFFF != value {
				value -= instrAddr
				v := int32(value)
				if v > (1<<15)-1 || v < -(1<<15) {
					return fmt.Errorf("Operand is out of range.")
				}
				op.Type = parser.OP_DIFF
			}
		}

		// We need first to convert from uint32 -> int32 so we can get the value
		// in the correct range. Then we can convert to int64 which is the required
		// type for FormatInt.
		op.Op = strconv.FormatInt(int64(int32(value)), 10)
		if op.Type != parser.OP_DIFF {
			op.Type = parser.OP_NUMBER
		}
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
		binary.LittleEndian.PutUint32(word, org.Addr)
		if _, err := buf.Write(word); err != nil {
			return err
		}
		binary.LittleEndian.PutUint32(word, uint32(org.WordCount()))
		if _, err := buf.Write(word); err != nil {
			return err
		}
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
		return encodeLoad(i)
	case "ldrip":
		return encodeLoadIP(i)
	case "ldrpi":
		return encodeLoadPI(i)
	case "str":
		return encodeStor(i)
	case "strip":
		return encodeStorIP(i)
	case "strpi":
		return encodeStorPI(i)
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
	case "mul":
		return encode3op(i, MulRR, MulRI)
	case "div":
		return encode3op(i, DivRR, DivRI)
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

func encodeLoad(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op2.Type == parser.OP_REG {
		return LoadRR(rToI(i.Op1.Op), rToI(i.Op2.Op)), nil
	} else if i.Op2.Type == parser.OP_NUMBER {
		return LoadRI(rToI(i.Op1.Op), toNum(i.Op2.Op)), nil
	} else {
		// We are convert a ldr -> ldri and doing it pc relative. pc is r29.
		return LoadIX(rToI(i.Op1.Op), 29, toNum(i.Op2.Op)), nil
	}
}

func encodeLoadIP(i parser.Instruction) (parser.Word, error) {
	if i.Op3.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: register operand must be a register.", i)
	}
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	return LoadIP(rToI(i.Op1.Op), rToI(i.Op2.Op), toNum(i.Op3.Op)), nil
}

func encodeLoadPI(i parser.Instruction) (parser.Word, error) {
	if i.Op3.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: register operand must be a register.", i)
	}
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	return LoadPI(rToI(i.Op1.Op), rToI(i.Op2.Op), toNum(i.Op3.Op)), nil
}
func encodeStor(i parser.Instruction) (parser.Word, error) {
	if i.Op2.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op1.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op1.Type == parser.OP_REG {
		return StorRR(rToI(i.Op1.Op), rToI(i.Op2.Op)), nil
	} else if i.Op1.Type == parser.OP_NUMBER {
		return StorRI(toNum(i.Op1.Op), rToI(i.Op2.Op)), nil
	} else {
		return StorIX(29, rToI(i.Op2.Op), toNum(i.Op1.Op)), nil
	}
}

func encodeStorIP(i parser.Instruction) (parser.Word, error) {
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: register operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op1.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	return StorIP(rToI(i.Op1.Op), rToI(i.Op3.Op), toNum(i.Op2.Op)), nil
}

func encodeStorPI(i parser.Instruction) (parser.Word, error) {
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: register operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op1.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	return StorPI(rToI(i.Op1.Op), rToI(i.Op3.Op), toNum(i.Op2.Op)), nil
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

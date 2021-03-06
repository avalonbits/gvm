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

package code

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"strconv"
	"unicode/utf8"

	"github.com/avalonbits/gsm/parser"
)

type object struct {
	hash string
	node *Node
}

func generateObject(ast *parser.AST, name string, allObjs map[string]*object) error {
	localLabelMap := map[string]uint32{}
	if err := assignLocalAddresses(localLabelMap, ast); err != nil {
		return err
	}
	resolve, err := convertLocalNames(localLabelMap, ast)
	if err != nil {
		return err
	}

	if resolve {
		if err := resolveExternalReferences(allObjs, ast); err != nil {
			return err
		}
	}

	var b bytes.Buffer

	return writeObject(
		ast, name, allObjs, bufio.NewReadWriter(bufio.NewReader(&b), bufio.NewWriter(&b)))
}

func GenerateFromObject(ast *parser.AST, buf *bufio.Writer) error {
	defer buf.Flush()

	objs := map[string]*object{}
	if err := generateObject(ast, "__root", objs); err != nil {
		return err
	}

	// For now, assume this is a .bin program.
	buf.Write([]byte("s1987gvm"))
	if err := writeObjectToFile(objs, buf); err != nil {
		return err
	}
	return nil
}

func isJmpInstr(instr string) bool {
	switch instr {
	case "call", "jmp", "jgt", "jge", "jlt", "jle", "jeq", "jne":
		return true
	}
	return false
}

type firstUse struct {
	org     int
	section int
}

func assignLocalAddresses(labelMap map[string]uint32, ast *parser.AST) error {
	for _, org := range ast.Orgs {
		baseAddr := org.Addr
		wordCount := uint32(0)
		if org.Sections == nil {
			continue
		}
		for section := org.Sections; ; section = section.Next {
			for _, block := range section.Blocks {
				label := block.LabelName(section.IncludeName)
				if label != section.IncludeName {
					if _, ok := labelMap[label]; ok {
						return block.Errorf("label redefinition: %q", block.Label)
					}
					if _, ok := ast.Consts[label]; ok {
						return block.Errorf("label redefinition: %q was defined as a const",
							block.Label)
					}
					if _, ok := ast.Includes[label]; ok {
						return block.Errorf("label redefinition: %q was defined as an include",
							block.Label)
					}
					labelMap[label] = baseAddr + (wordCount * 4)
				}
				wordCount += uint32(block.WordCount())
			}
			if section.Next == org.Sections {
				break
			}
		}
	}
	return nil
}

func convertLocalNames(localLabelMap map[string]uint32, ast *parser.AST) (bool, error) {
	resolve := false
	for _, org := range ast.Orgs {
		if org.Sections == nil {
			continue
		}
		for section := org.Sections; ; section = section.Next {
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
						addr := localLabelMap[block.LabelName(section.IncludeName)] + uint32(i*4)
						for _, op := range ops {
							err := convertLocalOperand(
								statement.Instr.Name, addr, block,
								section.IncludeName, localLabelMap, ast.Consts, op)
							if err != nil {
								return false, statement.Errorf("error processing instruction %q: %v",
									statement.Instr, err)
							}
							statement.ResolveReference =
								statement.ResolveReference || op.Type == parser.OP_EXTERNAL_LABEL
							resolve = resolve || op.Type == parser.OP_EXTERNAL_LABEL
						}
						continue
					}

					if statement.Label != "" {
						// This is a data entry with a label. Get the address if it is a local label.
						addr, ok := localLabelMap[section.IncludeName+"."+statement.Label]
						if !ok {
							return false, statement.Errorf("label does not exist: %q", statement.Label)
						}
						statement.Label = ""
						statement.Value = addr
					}
				}
			}
			if section.Next == org.Sections {
				break
			}

		}
	}
	return resolve, nil
}

func convertLocalOperand(instr string, instrAddr uint32, block parser.Block, inclName string, labelMap map[string]uint32, consts map[string]string, op *parser.Operand) error {
	if op.Type != parser.OP_LABEL {
		return nil
	}
	fullOp := inclName + "." + op.Op
	if value, ok := consts[fullOp]; ok {
		op.Op = value
		op.Type = parser.OP_NUMBER
		return nil
	} /*
		var jumpName string
		if strings.Index(op.Op, ".") != -1 {
			op.Type = parser.OP_EXTERNAL_LABEL
			return nil
		}*/

	// First check if this is a local label i.e a label inside a function.
	jumpName := block.JumpName(inclName, op.Op)
	value, ok := labelMap[jumpName]
	if !ok {
		// It's not, so let's check if it is a free label.
		value, ok = labelMap[fullOp]
		if !ok {
			return fmt.Errorf("operand %q is not a label or a constant", op.Op)
		}
	}

	switch instr {
	case "jmp", "jne", "jeq", "jlt", "jle", "jge", "jgt", "call":
		value -= instrAddr
	case "ldr", "str", "mov":
		if value >= 0x2400 {
			value -= instrAddr
			v := int32(value)
			if instr == "ldr" || instr == "str" {
				if v > (1<<20)-1 || v < -(1<<20) {
					return fmt.Errorf("Operand is out of range.")
				}
			} else {
				if v > (1<<15)-1 || v < -(1<<15) {
					return fmt.Errorf("Operand is out of range.")
				}
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

func resolveExternalReferences(allObjs map[string]*object, ast *parser.AST) error {
	return nil
}

func writeObject(ast *parser.AST, name string, allObjs map[string]*object, buf *bufio.ReadWriter) error {
	obj := &object{
		hash: ast.Hash,
		node: NewNode(name, NT_RELOCATABLE),
	}

	fnTable := map[string]uint32{}
	word := make([]byte, 4)
	for _, org := range ast.Orgs {
		var nb uint32
		if org.Sections == nil {
			continue
		}
		for section := org.Sections; ; section = section.Next {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if section.Type == parser.DATA_SECTION {
						if statement.ArraySize != 0 {
							bytes := make([]byte, statement.ArraySize)
							if _, err := buf.Write(bytes); err != nil {
								return err
							}
							nb += uint32(statement.ArraySize)
						} else if len(statement.Str) > 0 {
							sz := utf8.RuneCountInString(statement.Str) + 1
							sz *= 2
							if sz%4 != 0 {
								sz += 2
							}
							bytes := make([]byte, sz)
							i := 0
							for _, r := range statement.Str {
								binary.LittleEndian.PutUint16(bytes[i:i+2], uint16(r&0xFFFF))
								i += 2
							}
							if _, err := buf.Write(bytes); err != nil {
								return err
							}
							nb += uint32(len(bytes))
						} else if len(statement.Blob) > 0 {
							if _, err := buf.Write(statement.Blob); err != nil {
								return err
							}
							nb += uint32(len(statement.Blob))
						} else {
							if statement.Label != "" {
								return statement.Errorf("unresolved reference to %q", statement.Label)
							}
							binary.LittleEndian.PutUint32(word, statement.Value)
							if _, err := buf.Write(word); err != nil {
								return err
							}
							nb += 4
						}
						continue
					}
					if statement.ResolveReference {
						return statement.Errorf("%q: unresolved reference", statement.Instr)
					}
					w, err := encode(statement.Instr)
					if err != nil {
						return statement.Errorf(err.Error())
					}
					binary.LittleEndian.PutUint32(word, uint32(w))
					if _, err := buf.Write(word); err != nil {
						return err
					}
					nb += 4
				}
			}
			if section.Next == org.Sections {
				break
			}
		}
		if err := buf.Flush(); err != nil {
			return err
		}
		bin, err := ioutil.ReadAll(buf)
		if err != nil {
			return err
		}
		if err := obj.node.AddSpan(org.Addr, bin, fnTable); err != nil {
			return err
		}

	}
	obj.node.LiftFuncTable()
	allObjs[name] = obj
	return nil
}

func writeObjectToFile(objs map[string]*object, buf *bufio.Writer) error {
	word := make([]byte, 4)

	for _, obj := range objs {
		nIter := obj.node.Iterator()
		for addr, code, err := nIter.Next(); err == nil; addr, code, err = nIter.Next() {
			if len(code) == 0 {
				continue
			}
			binary.LittleEndian.PutUint32(word, addr)
			if _, err := buf.Write(word); err != nil {
				return err
			}
			binary.LittleEndian.PutUint32(word, uint32(len(code)/4))
			if _, err := buf.Write(word); err != nil {
				return err
			}
			if _, err := buf.Write(code); err != nil {
				return err
			}
		}
	}
	return nil
}

func rToI(reg string) uint32 {
	if reg == "sp" {
		return 28
	}
	if reg == "fp" {
		return 29
	}
	if reg == "pc" {
		return 30
	}
	if reg == "rZ" {
		return 31
	}
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
	case "wfi":
		return Wfi(), nil
	case "mov":
		return encodeMov(i)
	case "ldr":
		return encodeLoad(i)
	case "ldri":
		return encodeLoadIX(i)
	case "ldrip":
		return encodeLoadIP(i)
	case "ldrpi":
		return encodeLoadPI(i)
	case "ldpip":
		return encodeLoadPairIP(i)
	case "ldppi":
		return encodeLoadPairPI(i)
	case "str":
		return encodeStor(i)
	case "stri":
		return encodeStorIX(i)
	case "strip":
		return encodeStorIP(i)
	case "strpi":
		return encodeStorPI(i)
	case "stpip":
		return encodeStorPairIP(i)
	case "stppi":
		return encodeStorPairPI(i)
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

func encodeMov(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op2.Type == parser.OP_REG {
		return MovRR(rToI(i.Op1.Op), rToI(i.Op2.Op)), nil
	} else if i.Op2.Type == parser.OP_NUMBER {
		return MovRI(rToI(i.Op1.Op), toNum(i.Op2.Op)), nil
	} else {
		return AddRI(rToI(i.Op1.Op), 30, toNum(i.Op2.Op)), nil
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
		// We are convert a ldr -> ldri and doing it pc relative.
		return LoadPC(rToI(i.Op1.Op), toNum(i.Op2.Op)), nil
	}
}

func encodeLoadIX(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op2.Type == parser.OP_LABEL {
		return parser.Word(0),
			fmt.Errorf("%q: label substitution was not performed.", i)
	}
	if i.Op3.Type == parser.OP_NUMBER {
		return LoadIX(rToI(i.Op1.Op), rToI(i.Op2.Op), toNum(i.Op3.Op)), nil
	} else {
		return LoadIXR(rToI(i.Op1.Op), rToI(i.Op2.Op), rToI(i.Op3.Op)), nil
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
func encodeLoadPairPI(i parser.Instruction) (parser.Word, error) {
	if i.Op4.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: index operand must be a constant.", i)
	}
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a register.", i)
	}
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0),
			fmt.Errorf("%q: third operation must be a register.", i)
	}
	if i.Op4.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: index operand must be a constant.", i)
	}
	return LoadPairPI(rToI(i.Op1.Op), rToI(i.Op2.Op), rToI(i.Op3.Op), toNum(i.Op4.Op)), nil
}
func encodeLoadPairIP(i parser.Instruction) (parser.Word, error) {
	if i.Op4.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: index operand must be a constant.", i)
	}
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a register.", i)
	}
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0),
			fmt.Errorf("%q: third operation must be a register.", i)
	}
	if i.Op4.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: index operand must be a constant.", i)
	}
	return LoadPairIP(rToI(i.Op1.Op), rToI(i.Op2.Op), rToI(i.Op3.Op), toNum(i.Op4.Op)), nil
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
		return StorPC(toNum(i.Op1.Op), rToI(i.Op2.Op)), nil
	}
}

func encodeStorIX(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_NUMBER {
		return parser.Word(0),
			fmt.Errorf("%q: second operand must be a number.", i)
	}
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: third operand must be a register", i)
	}
	return StorIX(rToI(i.Op1.Op), rToI(i.Op3.Op), toNum(i.Op2.Op)), nil
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

func encodeStorPairIP(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: third operand must be a register.", i)
	}
	if i.Op4.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: forth operand must be a register.", i)
	}

	return StorPairIP(rToI(i.Op1.Op), rToI(i.Op3.Op), rToI(i.Op4.Op), toNum(i.Op2.Op)), nil
}

func encodeStorPairPI(i parser.Instruction) (parser.Word, error) {
	if i.Op1.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: first operand must be a register.", i)
	}
	if i.Op2.Type != parser.OP_NUMBER {
		return parser.Word(0), fmt.Errorf("%q: second operand must be a constant.", i)
	}
	if i.Op3.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: third operand must be a register.", i)
	}
	if i.Op4.Type != parser.OP_REG {
		return parser.Word(0), fmt.Errorf("%q: forth operand must be a register.", i)
	}

	return StorPairPI(rToI(i.Op1.Op), rToI(i.Op3.Op), rToI(i.Op4.Op), toNum(i.Op2.Op)), nil
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

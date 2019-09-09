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
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"unicode/utf8"

	"github.com/avalonbits/gsm/lexer"
	"github.com/avalonbits/gsm/parser"
)

func Parse(in io.Reader, requireLibrary bool) (*parser.AST, error) {
	lex := lexer.New(in)
	p := parser.New(lex)
	if err := p.Parse(requireLibrary); err != nil {
		return nil, err
	}
	p.Ast.Hash = lex.Hash()
	return p.Ast, nil
}

type object struct {
	hash      string
	code      map[uint32][]byte
	exported  map[string]uint32
	unrefCode map[string]uint32
	unrefData map[string]uint32
}

func generateObject(ast *parser.AST, name string) (map[string]*object, error) {
	var err error
	if err = embedFile(ast); err != nil {
		return nil, err
	}

	includeMap := map[string]*parser.AST{}
	if err = includeFile(includeMap, ast); err != nil {
		return nil, err
	}

	hashInclude := map[string]string{}
	allObjs := map[string]*object{}
	for k, iAST := range includeMap {
		hashInclude[k] = iAST.Hash
		objs, err := generateObject(iAST, k)
		if err != nil {
			return nil, err
		}
		for k, v := range objs {
			allObjs[k] = v
		}
	}

	// At this point we either handled all the includes or this is a leaf file.
	localLabelMap := map[string]uint32{}
	if err := assignLocalAddresses(localLabelMap, ast); err != nil {
	}
	if err := convertLocalNames(localLabelMap, ast); err != nil {
		return nil, err
	}

	var b bytes.Buffer

	obj, err := writeObject(ast, bufio.NewReadWriter(bufio.NewReader(&b), bufio.NewWriter(&b)))
	if err != nil {
		return nil, err
	}
	allObjs[name] = obj
	return allObjs, nil
}

func GenerateFromObject(ast *parser.AST, buf *bufio.Writer) error {
	defer buf.Flush()

	objs, err := generateObject(ast, "__root")
	if err != nil {
		return err
	}

	// For now, assume this is a .bin program.
	buf.Write([]byte("s1987gvm"))
	if err := writeObjectToFile(objs, buf); err != nil {
		return err
	}
	return nil
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

func includeFile(includeMap map[string]*parser.AST, ast *parser.AST) error {
	curWD, err := os.Getwd()
	if err != nil {
		return err
	}
	for incl := range ast.Includes {
		if _, ok := ast.Consts[incl]; ok {
			return fmt.Errorf("include redefinition: %q was defined as a contant")
		}

		// Ok, we have an include. Try to read the file.
		in, err := os.Open(ast.Includes[incl])
		if err != nil {
			return err
		}
		defer in.Close()

		dir, _ := filepath.Split(ast.Includes[incl])
		if dir == "" {
			continue
		}

		if err := os.Chdir(dir); err != nil {
			panic(err)
		}

		// Parse the file, producing an AST.
		ast, err := Parse(in, true)
		if err != nil {
			return err
		}

		// Set the include name on each section.
		for o := range ast.Orgs {
			for s := range ast.Orgs[o].Sections {
				ast.Orgs[o].Sections[s].IncludeName = incl
			}
		}

		// Alright! Add the AST to the include map and we are ready to process the next.
		includeMap[incl] = ast

		// Restore the current working directory.
		if err := os.Chdir(curWD); err != nil {
			return err
		}
	}
	// Return to the saved working directory.
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
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				if block.LocalLabelName() != "" {
					label := block.LocalLabelName()
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
		}
	}
	return nil
}

func convertLocalNames(localLabelMap map[string]uint32, ast *parser.AST) error {
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
						addr := localLabelMap[block.LocalLabelName()] + uint32(i*4)
						for _, op := range ops {
							err := convertLocalOperand(
								statement.Instr.Name, addr, block,
								section.IncludeName, localLabelMap, ast.Consts, op)
							if err != nil {
								return statement.Errorf("error processing instruction %q: %v",
									statement.Instr, err)
							}
						}
						continue
					}

					if statement.Label != "" {
						if strings.Index(statement.Label, ".") != -1 {
							// This is an included label. We can't do anything here yet, let's skip it.
							statement.Value = 0 // just in case it was set to something else.
							continue
						}

						// This is a data entry with a label. Get the address if it is a local label.
						addr, ok := localLabelMap[statement.Label]
						if !ok {
							return statement.Errorf("label does not exist: %q", statement.Label)
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

func convertLocalOperand(instr string, instrAddr uint32, block parser.Block, inclName string, labelMap map[string]uint32, consts map[string]string, op *parser.Operand) error {
	if op.Type != parser.OP_LABEL {
		return nil
	}
	if value, ok := consts[op.Op]; ok {
		op.Op = value
		op.Type = parser.OP_NUMBER
		return nil
	}
	var jumpName string
	if strings.Index(op.Op, ".") != -1 {
		op.Type = parser.OP_EXTERNAL_LABEL
		return nil
	}

	// First check if this is a local label i.e a label inside a function.
	jumpName = block.JumpName("", op.Op)
	value, ok := labelMap[jumpName]
	if !ok {
		// It's not, so let's check if it is a free label.
		value, ok = labelMap[op.Op]
		if !ok {
			return fmt.Errorf("operand %q is not a label or a constant", op.Op)
		}
	}

	switch instr {
	case "jmp", "jne", "jeq", "jlt", "jle", "jge", "jgt", "call":
		value -= instrAddr
	case "ldr", "str", "mov":
		if value >= 0x2100 {
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

func writeObjectToFile(objs map[string]*object, buf *bufio.Writer) error {
	word := make([]byte, 4)

	for _, obj := range objs {
		for addr, code := range obj.code {
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

func writeObject(ast *parser.AST, buf *bufio.ReadWriter) (*object, error) {
	obj := &object{
		hash:      ast.Hash,
		code:      map[uint32][]byte{},
		exported:  map[string]uint32{},
		unrefCode: map[string]uint32{},
		unrefData: map[string]uint32{},
	}
	word := make([]byte, 4)
	for _, org := range ast.Orgs {
		var nb uint32
		for _, section := range org.Sections {
			for _, block := range section.Blocks {
				for _, statement := range block.Statements {
					if section.Type == parser.DATA_SECTION {
						if statement.ArraySize != 0 {
							bytes := make([]byte, statement.ArraySize)
							if _, err := buf.Write(bytes); err != nil {
								return nil, err
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
								return nil, err
							}
							nb += uint32(len(bytes))
						} else {
							if statement.Label != "" {
								// This is an external label that hasn't been resolved. We need to
								// save the point in the code where the address starts so we can
								// update it later. We will store 0 as the address value for now.
								obj.unrefData[statement.Label] = nb
							}
							binary.LittleEndian.PutUint32(word, statement.Value)
							if _, err := buf.Write(word); err != nil {
								return nil, err
							}
							nb += 4
						}
						continue
					}
					w, err := encode(statement.Instr)
					if err != nil {
						return nil, statement.Errorf(err.Error())
					}
					binary.LittleEndian.PutUint32(word, uint32(w))
					if _, err := buf.Write(word); err != nil {
						return nil, err
					}
					nb += 4
				}
			}
		}
		if err := buf.Flush(); err != nil {
			return nil, err
		}
		bin, err := ioutil.ReadAll(buf)
		if err != nil {
			return nil, err
		}
		obj.code[org.Addr] = bin

	}
	return obj, nil
}

func rToI(reg string) uint32 {
	if reg == "rZ" {
		return 28
	}
	if reg == "pc" {
		return 29
	}
	if reg == "sp" {
		return 30
	}
	if reg == "fp" {
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
		return AddRI(rToI(i.Op1.Op), 29, toNum(i.Op2.Op)), nil
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

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

// Code generated by "stringer -type=TokenType"; DO NOT EDIT.

package lexer

import "strconv"

func _() {
	// An "invalid array index" compiler error signifies that the constant values have changed.
	// Re-run the stringer command to generate them again.
	var x [1]struct{}
	_ = x[ILLEGAL-0]
	_ = x[EOF-1]
	_ = x[COMMA-2]
	_ = x[SEMICOLON-3]
	_ = x[NEWLINE-4]
	_ = x[COLON-5]
	_ = x[WHITE_SPACE-6]
	_ = x[L_BRACKET-7]
	_ = x[R_BRACKET-8]
	_ = x[F_SLASH-9]
	_ = x[SECTION-10]
	_ = x[EMBED-11]
	_ = x[INCLUDE-12]
	_ = x[EQUATE-13]
	_ = x[AS-14]
	_ = x[D_QUOTE-15]
	_ = x[S_DATA-16]
	_ = x[S_TEXT-17]
	_ = x[ORG-18]
	_ = x[LABEL-19]
	_ = x[INT_TYPE-20]
	_ = x[ARRAY_TYPE-21]
	_ = x[STRING_TYPE-22]
	_ = x[IDENT-23]
	_ = x[NUMBER-24]
	_ = x[REGISTER-25]
	_ = x[INSTRUCTION-26]
	_ = x[FUNC_START-27]
	_ = x[INFUNC_START-28]
	_ = x[FUNC_END-29]
	_ = x[BIN_FILE-30]
	_ = x[PROGRAM_FILE-31]
	_ = x[LIBRARY_FILE-32]
}

const _TokenType_name = "ILLEGALEOFCOMMASEMICOLONNEWLINECOLONWHITE_SPACEL_BRACKETR_BRACKETF_SLASHSECTIONEMBEDINCLUDEEQUATEASD_QUOTES_DATAS_TEXTORGLABELINT_TYPEARRAY_TYPESTRING_TYPEIDENTNUMBERREGISTERINSTRUCTIONFUNC_STARTINFUNC_STARTFUNC_ENDBIN_FILEPROGRAM_FILELIBRARY_FILE"

var _TokenType_index = [...]uint8{0, 7, 10, 15, 24, 31, 36, 47, 56, 65, 72, 79, 84, 91, 97, 99, 106, 112, 118, 121, 126, 134, 144, 155, 160, 166, 174, 185, 195, 207, 215, 223, 235, 247}

func (i TokenType) String() string {
	if i < 0 || i >= TokenType(len(_TokenType_index)-1) {
		return "TokenType(" + strconv.FormatInt(int64(i), 10) + ")"
	}
	return _TokenType_name[_TokenType_index[i]:_TokenType_index[i+1]]
}

/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "cpu.h"


enum token_parser_tipo {
	TPT_FIN, //fin de expresion
	TPT_NUMERO,
	TPT_VARIABLE, //mra, mrw, etc
	TPT_REGISTRO, //a, bc, de, etc
	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO //+,-,*,/. & (and), | (or), ^ (xor)
};

enum token_parser_indice {
	TPI_FIN, //para indicar final de array
	//de tipo variable
	TPI_V_MRA,
	TPI_V_MRW,
	//de tipo registro
	TPI_R_A,
	TPI_R_BC,
	TPI_R_DE,
	//de tipo operador logico
	TPI_OL_AND,
	TPI_OL_OR,
	TPI_OL_XOR,
	//de tipo condicional
	TPI_OC_IGUAL,
	TPI_OC_MENOR,
	TPI_OC_MAYOR,
	TPI_OC_DIFERENTE,
	//de tipo operador calculo
	TPI_OC_SUMA,
	TPI_OC_RESTA,
	TPI_OC_MULTIPLICACION,
	TPI_OC_DIVISION,
	TPI_OC_AND,	
	TPI_OC_OR,
	TPI_OC_XOR
};

enum token_parser_formato {
	TPF_DECIMAL,
	TPF_HEXADECIMAL,
	TPF_BINARIO,
	TPF_ASCII
};

struct s_token_parser {

	enum token_parser_tipo tipo; //(variable, registro, operador, número, fin, etc)
	enum token_parser_indice indice; //(si es registro , podría ser: 0 para registro A, 1 para registro B, etc)
	enum token_parser_formato formato; //(para números, decir si se muestra como hexadecimal, binario, ascii etc)
	int signo; //(signo del valor almacenado)
	int  valor; //(guarda el valor absoluto en variable int)

};

typedef struct s_token_parser token_parser;



//Usados en la conversion de texto a tokens
#define MAX_PARSER_TEXTOS_INDICE_LENGTH 32
struct s_token_parser_textos_indices {
	enum token_parser_indice indice;
	char texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];
};

typedef struct s_token_parser_textos_indices token_parser_textos_indices;

#define MAX_PARSER_TOKENS_NUM 1000

extern int exp_par_exp_to_tokens(char *expression,token_parser *tokens);
extern void exp_par_tokens_to_exp(token_parser *tokens,char *expression);

#endif

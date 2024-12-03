/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%option prefix="devtree"
%option noyywrap
%option noinput
%option never-interactive

%{
	#include <dtsls/parser/parser.tab.h>
	#include <stdlib.h>
	#include <utils/log.h>


	/* macros */
	// define YY_USER_ACTION, executed for every token, before excuting its action code
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		devtreelloc.first_line = devtreelloc.last_line = devtree_line; \
		devtreelloc.first_column = devtree_col; \
		devtreelloc.last_column = devtree_col + devtreeleng; \
		devtree_col += devtreeleng; \
	}

	#define INC_LINE { devtree_col = 1; devtree_line++; }


	/* local/static prototypes */
	// make yyunput available outside of the lexer
	static void yyunput(int c, char *buf_ptr);


	/* static variables */
	static unsigned int devtree_col = 1;
	static unsigned int devtree_line = 1;
%}

/* start states */
%x PREPROC
%x PP_INC
%x MULTI_LINE_COMMENT


%%


	/* preprocessor */
#[ \t]*							{ BEGIN PREPROC; }
<PREPROC>include[ \t]*			{ BEGIN PP_INC; return INCLUDE; }
<PREPROC>\\\r?\n				{ INC_LINE; /* skip over line concatenations using '\\' */}
<PREPROC>\n						{ INC_LINE; BEGIN INITIAL; }
<PREPROC>.						{ }
<PP_INC>[<>\"]					{ return devtreetext[0]; }
<PP_INC>[a-zA-Z0-9\/\._-]+		{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, HEADER); }
<PP_INC>\n						{ INC_LINE; BEGIN INITIAL; }

	/* typedef */
"memorytype"					{ return TYPEDEF_MEM; }
"devicetype"					{ return TYPEDEF_DEV; }

	/* node attributes */
"addr"							{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_ADDR); }
"int8"							{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_INT8); }
"int16"							{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_INT16); }
"int32"							{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_INT32); }
"int64"							{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_INT64); }
"string"						{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, NA_STRING); }

	/* asserts */
"assert"						{ return ASSERT; }

	/* comments */
\/\/.*\r?\n						{ INC_LINE }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ INC_LINE }

	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]*			{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, IDFR); }

	/* string */
\"[^"]*\"						{ return devtree_parser_strcpy(devtreelval.s, devtreetext + 1, devtreeleng - 2, STRING); }

	/* integer */
(0x)?[0-9a-fA-F]+				{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, INT); }

	/* general patterns */
\r?\n							{ INC_LINE; }
[ \t]							{ }

	/* default */
.								{ return devtreetext[0]; }


%%


void devtree_lex_reset(){
	devtree_col = 1;
	devtree_line = 1;
}

void devtreeunput(char c){
	yyunput(c, (yytext_ptr));
}

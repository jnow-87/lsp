/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%option prefix="dtsls"
%option noyywrap
%option noinput
%option never-interactive

%{
	#include <dtsls/parser/parser.tab.h>
	#include <stdlib.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		dtslslloc.first_line = dtslslloc.last_line = dtsls_line; \
		dtslslloc.first_column = dtsls_col; \
		dtslslloc.last_column = dtsls_col + dtslsleng; \
		dtsls_col += dtslsleng; \
	}

	#define INC_LINE { dtsls_col = 1; ++dtsls_line; }

	static unsigned int dtsls_col = 1;
	static unsigned int dtsls_line = 1;
%}

/* start states */
%x INCLUDE
%x DEFINE
%x MULTI_LINE_COMMENT


%%


	/* keywords */
#[ \t]*include[ \t]*			{ BEGIN INCLUDE; return T_INCLUDE; }
<INCLUDE>[<>\"]					{ return dtslstext[0]; }
<INCLUDE>[a-zA-Z0-9\/\._-]+		{ return dtsls_parser_strcpy(dtslslval.s, dtslstext, dtslsleng, T_HEADER); }
<INCLUDE>\n						{ INC_LINE; BEGIN INITIAL; }

#[ \t]*define					{ BEGIN DEFINE; return T_DEFINE; }
<DEFINE>[ \t]					{ }
<DEFINE>[a-zA-Z0-9_]+			{ return dtsls_parser_strcpy(dtslslval.s, dtslstext, dtslsleng, T_IDFR); }
<DEFINE>[a-zA-Z0-9_\[\]\.]+		{ return dtsls_parser_strcpy(dtslslval.s, dtslstext, dtslsleng, T_VALUE); }
<DEFINE>\([a-zA-Z0-9_,\. ]*\)	{ return dtsls_parser_strcpy(dtslslval.s, dtslstext, dtslsleng, T_SIGNATURE); }
<DEFINE>\\\r?\n.*				{ INC_LINE; /* skip over line concatenations using '\\' */}
<DEFINE>\r?\n					{ INC_LINE; BEGIN INITIAL; }
<DEFINE>.						{ return dtslstext[0]; }

	/* comments */
\/\/.*\r?\n						{ INC_LINE }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ INC_LINE }

	/* general patterns */
\r?\n							{ INC_LINE; }
[ \t]							{ }

	/* default */
.								{ }


%%


void dtsls_lex_reset(){
	dtsls_col = 1;
	dtsls_line = 1;
}


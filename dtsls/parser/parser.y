/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%define api.prefix {dtsls}
%define parse.error verbose
%define parse.lac full
%locations

/* header */
%{
	#include <stdbool.h>
	#include <stdio.h>
	#include <dtsls/parser/lexer.lex.h>
	#include <dtsls/symtab.h>
	#include <utils/log.h>


	/* macros */
	#define YYDEBUG	1


	/* external prototypes */
	extern void dtsls_lex_reset();


	/* local/static prototypes */
	static int dtslserror(file_t *file, char const *s);
	static void cleanup(void);


	/* static variables */
	static file_t *dt_script = NULL;
%}

%code requires{
	#include <stdarg.h>
	#include <dtsls/symtab.h>


	/* prototypes */
	int dtsls_parser_error(char const *fmt, ...);
}

/* parse paramters */
%parse-param { file_t *file }

/* init code */
%initial-action{
	dt_script = file;

	// start lexer
	dtsls_lex_reset();
	dtsls_scan_string(file->text);
}

/* parser union type */
%union{
	struct{
		char *s;
		size_t len;
	} str;
}

/* terminals */
%token T_INCLUDE
%token T_DEFINE
%token T_VALUE
%token <str> T_HEADER
%token <str> T_IDFR
%token <str> T_SIGNATURE


%%


/* start */
start : error											{ cleanup(); YYABORT; }
	  | symbol-lst										{ cleanup(); }
	  ;

symbol-lst : %empty										{ }
		   | symbol-lst T_INCLUDE '<' T_HEADER '>'		{ symtab_file_stage($4.s, $4.len, file, NULL, 0, false); }
		   | symbol-lst T_INCLUDE '"' T_HEADER '"'		{ symtab_file_stage($4.s, $4.len, file, NULL, 0, true); }
		   | symbol-lst T_DEFINE T_IDFR					{ /* ignore */ }
		   | symbol-lst T_DEFINE T_IDFR T_VALUE			{ /* ignore */ }
		   | symbol-lst T_DEFINE T_IDFR	T_IDFR			{ /* ignore */ }
		   | symbol-lst T_DEFINE T_IDFR T_SIGNATURE		{ symtab_symbol_add(file, dtslslloc.first_line, dtslslloc.first_column - $3.len, $3.s, $3.len, $4.s, $4.len); }
		   ;


%%


/* global functions */
int dtsls_parser_error(char const *fmt, ...){
	va_list lst;


	if(dt_script != NULL){
		dprintf(log_filed(), "%s:%d:%d token \"%s\" -- ",
			dt_script->path,
			dtslslloc.first_line,
			dtslslloc.first_column,
			dtslstext
		);
	}

	va_start(lst, fmt);
	vdprintf(log_filed(), fmt, lst);
	va_end(lst);

	dprintf(log_filed(), ": %s\n", (errno ? strerror(errno) : ""));

	return -1;
}


/* local functions */
static int dtslserror(file_t *file, char const *s){
	dtsls_parser_error(s);

	return 0;
}

static void cleanup(void){
	dtslslex_destroy();
	dt_script = NULL;
}

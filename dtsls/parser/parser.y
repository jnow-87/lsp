/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%define api.prefix {devtree}
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
	#include <utils/string.h>
	#include <utils/vector.h>


	/* macros */
	#define YYDEBUG	1

	#define EABORT(expr){ \
		if(expr) \
			YYERROR; \
	}


	/* external prototypes */
	extern void devtree_lex_reset();


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(file_t *file, char const *s);
	static void cleanup(void);


	/* static variables */
	static file_t *dt_script = NULL;
%}

%code requires{
	#include <stdarg.h>
	#include <dtsls/symtab.h>
	#include <utils/vector.h>


	/* macros */
	#define DEVTREE_STRMAX	256


	/* prototypes */
	int devtree_parser_error(char const *fmt, ...);
	int devtree_parser_strcpy(char *dst, char const *src, size_t n, int token);
}

/* parse paramters */
%parse-param { file_t *file }

/* init code */
%initial-action{
	dt_script = file;

	// start lexer
	devtree_lex_reset();
	devtree_scan_string(file->text);
}

/* parser union type */
%union{
	char s[DEVTREE_STRMAX];
	vector_t attrs;
}

/* terminals */
%token INCLUDE
%token <s> HEADER
%token <s> IDFR

// general
%token <s> INT
%token <s> STRING

// typedef
%token TYPEDEF_MEM
%token TYPEDEF_DEV

// node attributes
%token <s> NA_ADDR
%token <s> NA_INT8
%token <s> NA_INT16
%token <s> NA_INT32
%token <s> NA_INT64
%token <s> NA_STRING

// asserts
%token ASSERT

/* non-terminals */
%type <attrs> type-body
%type <s> sattr
%type <s> iattr
%type <s> string
%type <s> int


%%


/* start */
start : error											{ cleanup(); YYABORT; }
	  | section-lst										{ cleanup(); }
	  ;

/* sections */
section-lst : %empty									{ }
			| section-lst ';'							{ }
			| section-lst INCLUDE '<' HEADER '>'		{ symtab_file_stage($4, file, NULL, false); }
			| section-lst INCLUDE '"' HEADER '"'		{ symtab_file_stage($4, file, NULL, true); }
			| section-lst typedef ';'					{ }
			| section-lst device ';'					{ }
			| section-lst attr-update ';'				{ }
			;

/* typedef */
typedef : TYPEDEF_MEM '{' type-body '}' IDFR			{ EABORT(symtab_type_add($5, &$3, file, @1.first_line, @1.first_column)); }
		| TYPEDEF_DEV '{' type-body '}' IDFR			{ EABORT(symtab_type_add($5, &$3, file, @1.first_line, @1.first_column)); }
		;

type-body : %empty										{ $$ = VECTOR_INITIALISER(sizeof(type_attr_t)); }
		  | type-body ';'								{ }
		  | type-body assert ';'						{ }
		  | type-body iattr IDFR ';'					{ EABORT(symtab_type_attr_add(&$$, $3, $2, "")); }
		  | type-body sattr IDFR ';'					{ EABORT(symtab_type_attr_add(&$$, $3, $2, "")); }
		  | type-body iattr IDFR '=' int ';'			{ EABORT(symtab_type_attr_add(&$$, $3, $2, $5)); }
		  | type-body sattr IDFR '=' string ';'			{ EABORT(symtab_type_attr_add(&$$, $3, $2, $5)); }
		  | type-body iattr IDFR '[' int ']' ';'		{ EABORT(symtab_type_attr_add(&$$, $3, $2, $5)); /* TODO handle list type */ }
		  ;

/* nodes */
device : IDFR '=' IDFR '(' type-args ')'				{ EABORT(symtab_node_add($1, $3, file, @1.first_line, @1.first_column)); }
	   | IDFR '=' IDFR '(' type-args ',' ')'			{ EABORT(symtab_node_add($1, $3, file, @1.first_line, @1.first_column)); }
	   ;

/* node bodies */
type-args : %empty										{ devtreeunput(','); }
		  | type-args ',' device						{ }
		  | type-args ',' IDFR '=' int					{ }
		  | type-args ',' IDFR '=' string				{ }
		  | type-args ',' IDFR '=' attr-ref				{ }
		  | type-args ',' IDFR '=' ilist				{ }
		  ;

/* asserts */
assert : ASSERT '(' string ',' string ')'				{ };

/* references */
attr-ref : IDFR '.' IDFR								{ };

/* attribute updates */
attr-update : attr-ref '=' int							{ }
			| attr-ref '+' '=' int						{ }
			| attr-ref '=' string						{ }
			| attr-ref '=' attr-ref						{ }
			| attr-ref '+' '=' attr-ref					{ }
			| attr-inc									{ }
			;

attr-inc : attr-ref '+' '+'								{ }
		 | '+' '+' attr-ref								{ }
		 ;

/* basic types */
ilist : '[' opt-int ']'									{ }
	  | '[' opt-int ',' ']'								{ }
	  ;

opt-int : %empty										{ devtreeunput(','); }
		| opt-int ',' int								{ }
		;

int : INT												{ }
	| int '+' INT										{ }
	| int '+' attr-ref									{ }
	| '(' attr-inc ')'									{ }
	;

string : STRING											{ };

/* node attributes */
sattr : NA_STRING										{ };

iattr : NA_INT8   										{ }
	  | NA_INT16										{ }
	  | NA_INT32										{ }
	  | NA_INT64										{ }
	  | NA_ADDR											{ }
	  ;



%%


/* global functions */
int devtree_parser_error(char const *fmt, ...){
	va_list lst;


	if(dt_script != NULL){
		dprintf(log_filed(), "%s:%d:%d token \"%s\" -- ",
			dt_script->path,
			devtreelloc.first_line,
			devtreelloc.first_column,
			devtreetext
		);
	}

	va_start(lst, fmt);
	vdprintf(log_filed(), fmt, lst);
	va_end(lst);

	if(errno)
		dprintf(log_filed(), ": %s", strerror(errno));

	dprintf(log_filed(), "\n");

	return -1;
}

int devtree_parser_strcpy(char *dst, char const *src, size_t n, int token){
	if(n >= DEVTREE_STRMAX){
		devtree_parser_error("string too long, max=%u", DEVTREE_STRMAX);

		// trigger a parser error
		// this is not the nicest way to trigger an error since it will cause a syntax
		// error even though it is not a syntax error and therefor confuse the user
		return YYSYMBOL_YYEOF;
	}

	strncpy(dst, src, n);
	dst[n] = 0;

	return token;
}


/* local functions */
static int devtreeerror(file_t *file, char const *s){
	devtree_parser_error(s);

	return 0;
}

static void cleanup(void){
	devtreelex_destroy();
	dt_script = NULL;
	errno = 0;
}

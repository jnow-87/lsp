/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DTSLS_SYMTAB_H
#define DTSLS_SYMTAB_H


#include <stdbool.h>
#include <utils/list.h>


/* types */
typedef struct file_t{
	char const *uri,
			   *path,
			   *text;
	size_t text_len;

	list_t *header,
		   *symbols;
} file_t;

typedef struct{
	char const *name,
			   *signature;

	file_t *file;
	size_t line,
		   column;
} symbol_t;


/* prototypes */
int symtab_update(char const *file_name, size_t name_len, char const *text, size_t text_len);
void symtab_free();

list_t const *symtab_files();
int symtab_file_stage(char const *name, size_t name_len, file_t *parent, char const *text, size_t text_len, bool resolve_relative);
file_t *symtab_file_lookup(char const *uri);

list_t const *symtab_symbols();
int symtab_symbol_add(file_t *file, size_t line, size_t column, char const *name, size_t name_len, char const *signature, size_t signature_len);


#endif // DTSLS_SYMTAB_H

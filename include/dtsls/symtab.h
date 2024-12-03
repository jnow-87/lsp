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
#include <utils/vector.h>


/* types */
typedef struct file_t{
	char const *uri,
			   *path,
			   *text;

	list_t *header,
		   *types,
		   *nodes;
} file_t;

typedef struct{
	char const *name,
			   *type,
			   *value;
} type_attr_t;

typedef struct{
	char const *name,
			   *signature,
			   *signature_defaults;
	vector_t attrs;

	file_t *file;
	size_t line,
		   column;
} type_t;

typedef struct{
	char const *name,
			   *type;

	file_t *file;
	size_t line,
		   column;
} node_t;



/* prototypes */
int symtab_update(char const *file_name, char const *text);
void symtab_free();

list_t *symtab_types(void);
list_t *symtab_nodes(void);

int symtab_file_stage(char const *name, file_t *parent, char const *text, bool resolve_relative);
file_t *symtab_file_lookup(char const *uri);

int symtab_type_add(char const *name, vector_t *attrs, file_t *file, size_t line, size_t column);
int symtab_type_attr_add(vector_t *attrs, char const *name, char const *type, char const *value);
int symtab_node_add(char const *name, char const *type, file_t *file, size_t line, size_t column);


#endif // DTSLS_SYMTAB_H

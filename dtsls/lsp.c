/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <string.h>
#include <dtsls/symtab.h>
#include <utils/file.h>
#include <utils/list.h>
#include <lsp.h>


/* local/static prototypes */
static void *doc_update(char const *path, char const *text);
static void *definition(char const *uri, size_t line, size_t column);
static void *symbols(char const *uri);
static void *completion(char const *uri, size_t line, size_t column);

static void match_symbols(char const *name, file_t *file, lsp_list_t *items);


/* global functions */
void lsp_init(){
	lsp_ops_t ops;

	ops.doc_open = doc_update;
	ops.doc_changed = doc_update;
	ops.definition = definition;
	ops.completion = completion;
	ops.symbols = symbols;

	lsp_cmds_register(&ops);
}


/* local functions */
static void *doc_update(char const *path, char const *text){
	symtab_update(path, strlen(path), text, strlen(text));

	return NULL;
}

static void *definition(char const *uri, size_t line, size_t column){
	char name[64];
	file_t *file;
	symbol_t *sym;
	lsp_list_t *items;
	lsp_dict_t *item;


	file = symtab_file_lookup(uri);

	if(file == NULL)
		return lsp_result_error("file not in symbol table");

	if(file_locate_identifier(file->text, line, column, name, sizeof(name), false) != 0)
		return lsp_list_init();

	items = lsp_list_init();

	list_for_each(symtab_symbols(), sym){
		if(strcmp(sym->name, name) == 0){
			item = lsp_dict_init();
			lsp_dict_add_string(item, "uri", sym->file->uri);
			lsp_dict_add_dict(item, "range", lsp_fmt_range(sym->line, sym->column));

			lsp_list_add(items, item);
		}
	}

	return items;
}

static void *symbols(char const *uri){
	file_t *file;
	symbol_t *sym;
	lsp_list_t *items;
	lsp_dict_t *item;


	file = symtab_file_lookup(uri);

	if(file == NULL)
		return lsp_result_error("file not in symbol table");

	items = lsp_list_init();

	list_for_each(file->symbols, sym){
		item = lsp_dict_init();
		lsp_dict_add_string(item, "name", sym->name);
		lsp_dict_add_string(item, "detail", sym->signature);
		lsp_dict_add_int(item, "kind", LSP_SYM_KIND_FUNCTION);
		lsp_dict_add_dict(item, "range", lsp_fmt_range(sym->line, sym->column));
		lsp_dict_add_dict(item, "selectionRange", lsp_fmt_range(sym->line, sym->column));

		lsp_list_add(items, item);
	}

	return items;
}

static void *completion(char const *uri, size_t line, size_t column){
	char name[64];
	file_t *file,
		   *hdr;
	lsp_list_t *items;


	file = symtab_file_lookup(uri);

	if(file == NULL)
		return lsp_result_error("file not in symbol table");

	items = lsp_list_init();

	if(file_locate_identifier(file->text, line, column - 1, name, sizeof(name), true) != 0)
		return items;

	match_symbols(name, file, items);

	list_for_each(file->header, hdr)
		match_symbols(name, hdr, items);

	return items;
}

static void match_symbols(char const *name, file_t *file, lsp_list_t *items){
	size_t name_len = strlen(name);
	symbol_t *sym;
	lsp_dict_t *item;


	list_for_each(file->symbols, sym){
		if(strncmp(sym->name, name, name_len) == 0){
			item = lsp_dict_init();
			lsp_dict_add_string(item, "label", sym->signature);
			lsp_dict_add_string(item, "insertText", sym->name);
			lsp_dict_add_int(item, "kind", LSP_COMPL_KIND_FUNCTION);

			lsp_list_add(items, item);
		}
	}
}

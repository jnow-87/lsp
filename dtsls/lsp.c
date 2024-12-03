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
#include <utils/log.h>
#include <lsp.h>


/* local/static prototypes */
static void *doc_update(char const *path, char const *text);
static void *definition(char const *uri, size_t line, size_t column);
static void *symbols(char const *uri);
static void *completion(char const *uri, size_t line, size_t column);

static void match_definition(lsp_list_t *items, char const *needle, char const *name, file_t *file, size_t line, size_t column);
static void match_completion(lsp_list_t *items, char const *name, file_t *file);
static void add_completion(lsp_list_t *items, char const *label, char const *insert, lsp_completionitem_kind_t kind);
static void add_symbol(lsp_list_t *symbols, char const *name, char const *detail, lsp_symbol_kind_t kind, size_t line, size_t column);


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
	symtab_update(path, text);

	return NULL;
}

static void *definition(char const *uri, size_t line, size_t column){
	char name[64];
	file_t *file;
	type_t *type;
	node_t *node;
	lsp_list_t *items;


	file = symtab_file_lookup(uri);

	if(file == NULL)
		return lsp_result_error("file not in symbol table");

	if(file_locate_identifier(file->text, line, column, name, sizeof(name), false) != 0)
		return lsp_list_init();

	items = lsp_list_init();

	list_for_each(symtab_types(), type)
		match_definition(items, name, type->name, type->file, type->line, type->column);

	list_for_each(symtab_nodes(), node)
		match_definition(items, name, node->name, node->file, node->line, node->column);

	return items;
}

static void *symbols(char const *uri){
	file_t *file;
	type_t *type;
	node_t *node;
	lsp_list_t *items;


	file = symtab_file_lookup(uri);

	if(file == NULL)
		return lsp_result_error("file not in symbol table");

	items = lsp_list_init();

	list_for_each(file->types, type)
		add_symbol(items, type->name, type->signature, LSP_SYM_KIND_FUNCTION, type->line, type->column);

	list_for_each(file->nodes, node)
		add_symbol(items, node->name, node->type, LSP_SYM_KIND_VARIABLE, node->line, node->column);

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

	match_completion(items, name, file);

	list_for_each(file->header, hdr)
		match_completion(items, name, hdr);

	return items;
}

static void match_definition(lsp_list_t *items, char const *needle, char const *name, file_t *file, size_t line, size_t column){
	lsp_dict_t *item;


	if(strcmp(name, needle) != 0)
		return;

	item = lsp_dict_init();
	lsp_dict_add_string(item, "uri", file->uri);
	lsp_dict_add_dict(item, "range", lsp_fmt_range(line, column));

	lsp_list_add(items, item);
}

static void match_completion(lsp_list_t *items, char const *name, file_t *file){
	size_t name_len = strlen(name);
	type_t *type;
	node_t *node;


	list_for_each(file->types, type){
		if(strncmp(type->name, name, name_len) == 0){
			add_completion(items, type->signature, type->name, LSP_COMPL_KIND_FUNCTION);
			add_completion(items, type->signature_defaults, type->name, LSP_COMPL_KIND_FUNCTION);
		}
	}

	list_for_each(file->nodes, node){
		if(strncmp(node->name, name, name_len) == 0)
			add_completion(items, node->type, node->name, LSP_COMPL_KIND_VARIABLE);
	}
}

static void add_completion(lsp_list_t *items, char const *label, char const *insert, lsp_completionitem_kind_t kind){
	lsp_dict_t *item;


	item = lsp_dict_init();
	lsp_dict_add_string(item, "label", label);
	lsp_dict_add_string(item, "insertText", insert);
	lsp_dict_add_int(item, "kind", kind);

	lsp_list_add(items, item);
}

static void add_symbol(lsp_list_t *symbols, char const *name, char const *detail, lsp_symbol_kind_t kind, size_t line, size_t column){
	lsp_dict_t *item;


	item = lsp_dict_init();
	lsp_dict_add_string(item, "name", name);
	lsp_dict_add_string(item, "detail", detail);
	lsp_dict_add_int(item, "kind", kind);
	lsp_dict_add_dict(item, "range", lsp_fmt_range(line, column));
	lsp_dict_add_dict(item, "selectionRange", lsp_fmt_range(line, column));

	lsp_list_add(symbols, item);
}

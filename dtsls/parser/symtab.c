/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <dtsls/config.h>
#include <dtsls/parser/parser.tab.h>
#include <dtsls/symtab.h>
#include <utils/file.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/string.h>


/* static variables */
static list_t *files = NULL,
			  *symbols = NULL;

static list_t *staged_files = NULL;


/* local/static prototypes */
static file_t *file_realloc(file_t *file, char const *uri, size_t uri_len, char const *text, size_t text_len, file_t *parent);
static void file_reset(file_t *file);
static void file_free(file_t *file);
static char const *file_resolve(char const *name, size_t name_len, file_t *parent);

static void sym_free(symbol_t *sym);


/* global functions */
int symtab_update(char const *file_name, size_t name_len, char const *text, size_t text_len){
	if(symtab_file_stage(file_name, name_len, NULL, text, text_len, false) != 0)
		return -1;

	while(staged_files){
		VERBOSE("parse file %s", ((file_t*)staged_files->payload)->path);

		dtslsparse(staged_files->payload);
		list_rm(&staged_files, staged_files);
	}

	return 0;
}

void symtab_free(){
	file_t *file;


	list_for_each(files, file)
		file_free(file);
}

list_t const *symtab_files(){
	return files;
}

int symtab_file_stage(char const *name, size_t name_len, file_t *parent, char const *text, size_t text_len, bool resolve_relative){
	char const *uri;
	file_t *file;


	// ignore non-dts files
	if(name_len <= 4 || strncmp(name + name_len - 4, ".dts", 4) != 0)
		return VERBOSE("ignore non-dts file %.*s", (int)name_len, name);

	uri = file_resolve(name, name_len, (resolve_relative ? parent : NULL));

	if(uri == NULL)
		return dtsls_parser_error("resolving file failed");

	file = symtab_file_lookup(uri);

	// do not stage files that haven't been modified
	if(file != NULL && (text == NULL || (file->text_len == text_len && strncmp(file->text, text, text_len) == 0)))
		return (parent != NULL) ? list_add(&parent->header, file) : 0;

	file = file_realloc(file, uri, strlen(uri), text, text_len, parent);

	if(file == NULL)
		return ERROR("staging file \"%s\"", uri);

	VERBOSE("stage file %s", uri);

	return list_add(&staged_files, file);
}

file_t *symtab_file_lookup(char const *uri){
	file_t *file;


	list_for_each(files, file){
		if(strcmp(file->uri, uri) == 0)
			return file;
	}

	return NULL;
}

list_t const *symtab_symbols(){
	return symbols;
}

int symtab_symbol_add(file_t *file, size_t line, size_t column, char const *name, size_t name_len, char const *signature, size_t signature_len){
	symbol_t *sym;


	sym = malloc(sizeof(symbol_t));

	if(sym == 0x0)
		goto err_0;

	sym->name = stralloc(name, name_len);
	sym->signature = stralloc(signature, signature_len);

	if(sym->name == NULL || sym->signature == NULL)
		goto err_1;

	if(list_add(&file->symbols, sym) != 0)
		goto err_1;

	if(list_add(&symbols, sym) != 0)
		goto err_1;

	sym->file = file;
	sym->line = line;
	sym->column = column;

	return 0;


err_1:
	sym_free(sym);

err_0:
	return dtsls_parser_error("symbol allocation failed");
}


/* local functions */
static file_t *file_realloc(file_t *file, char const *uri, size_t uri_len, char const *text, size_t text_len, file_t *parent){
	if(file == NULL)
		file = calloc(1, sizeof(file_t));

	if(file == NULL)
		goto err_0;

	file_reset(file);

	// only allocate uri when not reusing the file object
	if(file->uri == NULL)
		file->uri = stralloc(uri, uri_len);

	if(file->uri == NULL)
		goto err_1;

	file->path = file->uri + 7;

	if(text != NULL){
		file->text = stralloc(text, text_len);
		file->text_len = text_len;
	}
	else
		file->text = file_read(file->path, &file->text_len);

	if(file->text == NULL)
		goto err_1;

	if(list_add(&files, file) != 0)
		goto err_1;

	if(parent != NULL && list_add(&parent->header, file) != 0)
		goto err_1;

	return file;


err_1:
	file_free(file);

err_0:
	ERROR("file allocation failed \"%s\"", uri);

	return NULL;
}

static void file_reset(file_t *file){
	symbol_t *sym;


	list_rm(&files, list_find(files, file));

	list_for_each(file->symbols, sym)
		sym_free(sym);

	list_free(&file->header);
	list_free(&file->symbols);
	free((void*)file->text);
}

static void file_free(file_t *file){
	file_reset(file);
	free((void*)file->uri);
	free(file);
}

static char const *file_resolve(char const *name, size_t name_len, file_t *parent){
	int fd = -1;
	static char real[PATH_MAX + 8] = "file://";
	static char path[PATH_MAX + 1];
	char const *dir = NULL;
	char ppath[(parent ? strlen(parent->path) : 0) + 1],
		 _name[name_len + 1];


	strncpy(_name, name, name_len);
	_name[name_len] = 0;

	/* open file */
	if(name[0] != '/'){
		if(parent == NULL){
			// search file in include directories
			for(size_t i=0; i<config.nincludes; i++){
				dir = config.include_dirs[i].path;
				fd = openat(config.include_dirs[i].fd, _name, O_RDONLY);

				if(fd >= 0)
					break;
			}
		}
		else{
			// open file relative to parent
			dir = ppath;
			strcpy(ppath, parent->path);
			fd = openat(open(dirname(ppath), O_RDONLY), _name, O_RDONLY);
		}
	}
	else{
		// absolute path
		fd = open(_name, O_RDONLY);
	}

	if(fd < 0)
		return NULL;

	close(fd);

	/* set path */
	if(dir == NULL){
		strncpy(path, _name, sizeof(path) - 1);
		path[sizeof(path) - 1] = 0;
	}
	else
		snprintf(path, sizeof(path), "%s/%s", dir, _name);

	realpath(path, real + 7);

	return real;
}

static void sym_free(symbol_t *sym){
	list_rm(&symbols, list_find(symbols, sym));

	free((void*)sym->name);
	free((void*)sym->signature);
	free(sym);
}

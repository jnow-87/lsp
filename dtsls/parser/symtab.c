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
#include <utils/vector.h>


/* static variables */
static list_t *files = NULL,
			  *types = NULL,
			  *nodes = NULL;

static list_t *staged_files = NULL;


/* local/static prototypes */
static file_t *file_realloc(file_t *file, char const *uri, char const *text, file_t *parent);
static void file_reset(file_t *file);
static void file_free(file_t *file);
static char const *file_resolve(char const *name, size_t name_len, file_t *parent);

static void type_free(type_t *type);
static void type_attr_free(type_attr_t *attr);
static void node_free(node_t *node);

static char const *fmt_signature(vector_t *attrs, bool exclude_defaults);


/* global functions */
int symtab_update(char const *file_name, char const *text){
	if(symtab_file_stage(file_name, NULL, text, false) != 0)
		return -1;

	while(staged_files){
		VERBOSE("parse file %s", ((file_t*)staged_files->payload)->path);

		devtreeparse(staged_files->payload);
		list_rm(&staged_files, staged_files->payload);
	}

	return 0;
}

void symtab_free(){
	file_t *file;


	list_for_each(files, file)
		file_free(file);
}

list_t *symtab_types(void){
	return types;
}

list_t *symtab_nodes(void){
	return nodes;
}

int symtab_file_stage(char const *name, file_t *parent, char const *text, bool resolve_relative){
	size_t name_len = strlen(name);
	char const *uri;
	file_t *file;


	// ignore non-dts files
	if(name_len <= 4 || strncmp(name + name_len - 4, ".dts", 4) != 0)
		return VERBOSE("ignore non-dts file %.*s", (int)name_len, name);

	uri = file_resolve(name, name_len, (resolve_relative ? parent : NULL));

	if(uri == NULL)
		return devtree_parser_error("resolving file \"%s\" failed", name);

	file = symtab_file_lookup(uri);

	// do not stage files that haven't been modified
	if(file != NULL && (text == NULL || (strcmp(file->text, text) == 0)))
		return (parent != NULL) ? list_add(&parent->header, file) : 0;

	file = file_realloc(file, uri, text, parent);

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

// TODO compare speed for list-symbol between new and old dtsls
int symtab_type_add(char const *name, vector_t *attrs, file_t *file, size_t line, size_t column){
	type_t *type;


	type = malloc(sizeof(type_t));

	if(type == NULL)
		goto err_0;

	type->name = stralloc(name);
	type->attrs = *attrs;
	type->file = file;
	type->line = line;
	type->column = column;
	type->signature = fmt_signature(attrs, true);
	type->signature_defaults = fmt_signature(attrs, false);

	if(type->name == NULL || type->signature == NULL || type->signature_defaults == NULL)
		goto err_1;

	if(list_add(&types, type) != 0 || list_add(&file->types, type) != 0)
		goto err_1;

	return 0;


err_1:
	type_free(type);

err_0:
	return devtree_parser_error("type allocation failed");
}

int symtab_type_attr_add(vector_t *attrs, char const *name, char const *type, char const *value){
	type_attr_t attr;


	attr.name = stralloc(name);
	attr.type = stralloc(type);
	attr.value = stralloc(value);

	if(attr.name == NULL || attr.type == NULL || attr.value == NULL)
		goto err;

	if(vector_add(attrs, &attr) != 0)
		goto err;

	return 0;


err:
	type_attr_free(&attr);

	return devtree_parser_error("attribute allocation failed");
}

int symtab_node_add(char const *name, char const *type, file_t *file, size_t line, size_t column){
	node_t *node;


	node = malloc(sizeof(node_t));

	if(node == NULL)
		goto err_0;

	node->name = stralloc(name);
	node->type = stralloc(type);
	node->file = file;
	node->column = column;
	node->line = line;

	if(node->name == NULL || node->type == NULL)
		goto err_1;

	if(list_add(&nodes, node) != 0 || list_add(&file->nodes, node) != 0)
		goto err_1;

	VERBOSE("node: name=%s, type=%s, line=%zu, column=%zu", node->name, node->type, node->line, node->column);

	return 0;


err_1:
	node_free(node);

err_0:
	return devtree_parser_error("node allocation failed");
}


/* local functions */
static file_t *file_realloc(file_t *file, char const *uri, char const *text, file_t *parent){
	if(file == NULL)
		file = calloc(1, sizeof(file_t));

	if(file == NULL)
		goto err_0;

	file_reset(file);

	// only allocate uri when not reusing the file object
	if(file->uri == NULL)
		file->uri = stralloc(uri);

	if(file->uri == NULL)
		goto err_1;

	file->path = file->uri + 7;

	if(text != NULL){
		file->text = stralloc(text);
	}
	else
		file->text = file_read(file->path);

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
	type_t *type;
	node_t *node;


	list_rm(&files, file);

	list_for_each(file->types, type)
		type_free(type);

	list_for_each(file->nodes, node)
		node_free(node);

	list_free(&file->header);
	list_free(&file->types);
	list_free(&file->nodes);

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

static void type_free(type_t *type){
	type_attr_t *attr;


	list_rm(&type->file->types, type);
	list_rm(&types, type);

	vector_for_each(&type->attrs, attr)
		type_attr_free(attr);

	vector_destroy(&type->attrs);
	free((void*)type->signature);
	free((void*)type->signature_defaults);
	free((void*)type->name);
	free(type);
}

static void type_attr_free(type_attr_t *attr){
	free((void*)attr->name);
	free((void*)attr->type);
	free((void*)attr->value);
}

static void node_free(node_t *node){
	list_rm(&node->file->nodes, node);
	list_rm(&nodes, node);

	free((void*)node->name);
	free((void*)node->type);
	free(node);
}

static char const *fmt_signature(vector_t *attrs, bool exclude_defaults){
	size_t len = 2;
	size_t n = 0;
	char *sig;
	type_attr_t *attr;


	vector_for_each(attrs, attr){
		if(attr->value[0] == 0 || !exclude_defaults){
			len += strlen(attr->name) + strlen(attr->type) + 2 + strlen(attr->value) + 2;
			n++;
		}
	}

	sig = malloc(len + 1);

	if(sig == NULL)
		return NULL;

	len = 1;
	sig[0] = '(';

	for(size_t i=0; i<n; i++){
		attr = vector_get(attrs, i);

		if(attr->value[0] == 0 || !exclude_defaults){
			len += sprintf(sig + len, "%s=%s%s%s%s"
				, attr->name
				, attr->type
				, ((attr->value[0] == 0) ? "" : ":")
				, attr->value
				, (i + 1 >= n) ? "" : ", "
			);
		}
	}

	strcpy(sig + len, ")");

	return sig;
}

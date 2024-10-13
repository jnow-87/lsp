/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LSP_H
#define LSP_H


#include <stdbool.h>
#include <json-c/json.h>


/* types */
typedef json_object lsp_dict_t;
typedef json_object lsp_list_t;
typedef void * (*lsp_hdlr_t)(lsp_dict_t *params);

typedef enum{
	LSP_SYM_KIND_FILE = 1,
	LSP_SYM_KIND_MODULE,
	LSP_SYM_KIND_NAMESPACE,
	LSP_SYM_KIND_PACKAGE,
	LSP_SYM_KIND_CLASS,
	LSP_SYM_KIND_METHOD,
	LSP_SYM_KIND_PROPERTY,
	LSP_SYM_KIND_FIELD,
	LSP_SYM_KIND_CONSTRUCTOR,
	LSP_SYM_KIND_ENUM,
	LSP_SYM_KIND_INTERFACE,
	LSP_SYM_KIND_FUNCTION,
	LSP_SYM_KIND_VARIALE,
	LSP_SYM_KIND_CONSTANT,
	LSP_SYM_KIND_STRING,
	LSP_SYM_KIND_NUMBER,
	LSP_SYM_KIND_BOOLEAN,
	LSP_SYM_KIND_ARRAY,
	LSP_SYM_KIND_OBJECT,
	LSP_SYM_KIND_KEY,
	LSP_SYM_KIND_NULL,
	LSP_SYM_KIND_ENUMMEMBER,
	LSP_SYM_KIND_STRUCT,
	LSP_SYM_KIND_EVENT,
	LSP_SYM_KIND_OPERATOR,
	LSP_SYM_KIND_TYPEPARAMETER,
} lsp_symbol_kind_t;

typedef enum{
	LSP_COMPL_KIND_TEXT = 1,
	LSP_COMPL_KIND_METHOD,
	LSP_COMPL_KIND_FUNCTION,
	LSP_COMPL_KIND_CONSTRUCTOR,
	LSP_COMPL_KIND_FIELD,
	LSP_COMPL_KIND_VARIABLE,
	LSP_COMPL_KIND_CLASS,
	LSP_COMPL_KIND_INTERFACE,
	LSP_COMPL_KIND_MODULE,
	LSP_COMPL_KIND_PROPERTY,
	LSP_COMPL_KIND_UNIT,
	LSP_COMPL_KIND_VALUE,
	LSP_COMPL_KIND_ENUM,
	LSP_COMPL_KIND_KEYWORD,
	LSP_COMPL_KIND_SNIPPET,
	LSP_COMPL_KIND_COLOR,
	LSP_COMPL_KIND_FILE,
	LSP_COMPL_KIND_REFERENCE,
	LSP_COMPL_KIND_FOLDER,
	LSP_COMPL_KIND_ENUMMEMBER,
	LSP_COMPL_KIND_CONSTANT,
	LSP_COMPL_KIND_STRUCT,
	LSP_COMPL_KIND_EVENT,
	LSP_COMPL_KIND_OPERATOR,
	LSP_COMPL_KIND_TYPEPARAMETER,
} lsp_completionitem_kind_t;

typedef struct lsp_cmd_t{
	char const *method;
	lsp_hdlr_t hdlr;
} lsp_cmd_t;

typedef struct{
	void * (*doc_open)(char const *path, char const *text);
	void * (*doc_changed)(char const *path, char const *text);

	void * (*definition)(char const *uri, size_t line, size_t column);
	void * (*symbols)(char const *uri);
	void * (*completion)(char const *uri, size_t line, size_t column);
} lsp_ops_t;


/* prototypes */
// command handling
void lsp_cmds_register(lsp_ops_t *ops);
int lsp_cmds_process();

void *lsp_doc_open(lsp_dict_t *params);
void *lsp_doc_changed(lsp_dict_t *params);
void *lsp_completion(lsp_dict_t *params);
void *lsp_definition(lsp_dict_t *params);
void *lsp_symbols(lsp_dict_t *params);
void *lsp_initialize(lsp_dict_t *params);
void *lsp_shutdown(lsp_dict_t *params);
void *lsp_exit(lsp_dict_t *params);

// result wrapper
void *lsp_result_error(char const *msg);
void *lsp_result_exit();

// protocol data conversion
lsp_dict_t *lsp_fmt_position(size_t line, size_t column);
lsp_dict_t *lsp_fmt_range(size_t line, size_t column);
lsp_dict_t *lsp_fmt_textdocument(char const *uri, char const *text, char const *lang, int version);

char const *lsp_params_doc_uri(lsp_dict_t *params);
char const *lsp_params_doc_text(lsp_dict_t *params);
int lsp_params_position(lsp_dict_t * params, size_t *line, size_t *column);

char const *lsp_uri_to_path(char const *uri);

// base type functions (dict)
lsp_dict_t *lsp_dict_init();
void lsp_dict_free(lsp_dict_t *dict);

int lsp_dict_has(lsp_dict_t *dict, char const *key);
int lsp_dict_get_int(lsp_dict_t *dict, char const *key);
bool lsp_dict_get_bool(lsp_dict_t *dict, char const *key);
char const *lsp_dict_get_string(lsp_dict_t *dict, char const *key);
lsp_dict_t *lsp_dict_get_dict(lsp_dict_t *dict, char const *key);
lsp_list_t *lsp_dict_get_list(lsp_dict_t *dict, char const *key);

int lsp_dict_add_int(lsp_dict_t *dict, char const *key, int val);
int lsp_dict_add_bool(lsp_dict_t *dict, char const *key, bool val);
int lsp_dict_add_string(lsp_dict_t *dict, char const *key, char const *val);
int lsp_dict_add_dict(lsp_dict_t *dict, char const *key, lsp_dict_t *val);
int lsp_dict_add_list(lsp_dict_t *dict, char const *key, lsp_list_t *val);

// base type functions (list)
lsp_list_t *lsp_list_init();

size_t lsp_list_get_len(lsp_list_t *lst);
lsp_dict_t *lsp_list_get_idx(lsp_list_t *lst, size_t idx);

int lsp_list_add(lsp_list_t *lst, lsp_dict_t *dict);


#endif // LSP_H

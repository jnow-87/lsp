/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef WIGGLER_H
#define WIGGLER_H


#include <lsp.h>


/* types */
typedef int (*wiggler_hdlr_t)(int argc, char **argv, lsp_dict_t *result);

typedef enum{
	RET_ERROR = -1,
	RET_OK = 0,
	RET_EXIT,
	RET_NOACTION,
} wiggler_retval_t;

typedef struct wiggler_cmd_t{
	char const *cmd;
	wiggler_hdlr_t hdlr;

	size_t nargs;
	char const *args,
			   *descr;
} wiggler_cmd_t;


/* prototypes */
wiggler_retval_t cmd_initialize(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_shutdown(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_exit(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_help(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_doc_open(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_doc_changed(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_completion(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_definition(int argc, char **argv, lsp_dict_t *result);
wiggler_retval_t cmd_symbols(int argc, char **argv, lsp_dict_t *result);


#endif // WIGGLER_H

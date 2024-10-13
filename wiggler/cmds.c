/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <utils/file.h>
#include <utils/log.h>
#include <wiggler/cmds.h>
#include <wiggler/cmds.hash.h>
#include <lsp.h>


/* local/static prototypes */
int file_params(char const *path, char **uri, char **text, char **stem);


/* static variables */
static int lsp_id = 0;


/* global functions */
wiggler_retval_t cmd_help(int argc, char **argv, lsp_dict_t *result){
	wiggler_cmd_t const *cmd;


	LOG(
		"standard command flow:\n"
		"    init\n"
		"    ... doc commands ...\n"
		"    shutdown\n"
		"    exit\n\n"
		"commands:"
	);

	for(size_t i=WIGGLER_MIN_HASH_VALUE; i<=WIGGLER_MAX_HASH_VALUE; i++){
		cmd = wiggler_wordlist + i;

		if(cmd->cmd != NULL && cmd->descr != NULL)
			LOG("  %s %s\n    %s", cmd->cmd, cmd->args, cmd->descr);
	}

	return RET_NOACTION;
}

wiggler_retval_t cmd_initialize(int argc, char **argv, lsp_dict_t *result){
	int r = 0;


	r |= lsp_dict_add_int(result, "id", lsp_id++);
	r |= lsp_dict_add_string(result, "method", "initialize");
	r |= lsp_dict_add_dict(result, "params", lsp_dict_init());

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_shutdown(int argc, char **argv, lsp_dict_t *result){
	int r = 0;


	r |= lsp_dict_add_int(result, "id", lsp_id++);
	r |= lsp_dict_add_string(result, "method", "shutdown");

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_exit(int argc, char **argv, lsp_dict_t *result){
	lsp_dict_add_string(result, "method", "exit");
	lsp_dict_add_dict(result, "params", lsp_dict_init());

	return RET_EXIT;
}

wiggler_retval_t cmd_doc_open(int argc, char **argv, lsp_dict_t *result){
	int r = 0;
	char *uri,
		 *text,
		 *stem;
	lsp_dict_t *params;


	if(file_params(argv[1], &uri, &text, &stem) != 0)
		return -1;

	r |= lsp_dict_add_string(result, "method", "textDocument/didOpen");

	params = lsp_dict_init();
	r |= lsp_dict_add_dict(params, "textDocument", lsp_fmt_textdocument(uri, text, stem, 0));
	r |= lsp_dict_add_dict(result, "params", params);

	free(uri);
	free(text);

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_doc_changed(int argc, char **argv, lsp_dict_t *result){
	int r = 0;
	char *uri,
		 *text,
		 *stem;
	lsp_dict_t *params;
	lsp_list_t *changes;


	if(file_params(argv[1], &uri, &text, &stem) != 0)
		return -1;

	r |= lsp_dict_add_string(result, "method", "textDocument/didChange");

	params = lsp_dict_init();
	changes = lsp_list_init();
	r |= lsp_dict_add_string(params, "text", text);
	lsp_list_add(changes, params);

	params = lsp_dict_init();
	r |= lsp_dict_add_dict(params, "textDocument", lsp_fmt_textdocument(uri, NULL, stem, 0));
	r |= lsp_dict_add_list(params, "contentChanges", changes);
	r |= lsp_dict_add_dict(result, "params", params);

	free(uri);
	free(text);

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_completion(int argc, char **argv, lsp_dict_t *result){
	int r = 0;
	char *uri;
	lsp_dict_t *params;


	if(file_params(argv[1], &uri, NULL, NULL) != 0)
		return -1;

	r |= lsp_dict_add_int(result, "id", lsp_id++);
	r |= lsp_dict_add_string(result, "method", "textDocument/completion");

	params = lsp_dict_init();
	r |= lsp_dict_add_dict(params, "textDocument", lsp_fmt_textdocument(uri, NULL, "", 0));
	r |= lsp_dict_add_list(params, "position", lsp_fmt_position(atoi(argv[2]), atoi(argv[3])));
	r |= lsp_dict_add_dict(result, "params", params);

	free(uri);

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_definition(int argc, char **argv, lsp_dict_t *result){
	int r = 0;
	char *uri;
	lsp_dict_t *params;


	if(file_params(argv[1], &uri, NULL, NULL) != 0)
		return -1;

	r |= lsp_dict_add_int(result, "id", lsp_id++);
	r |= lsp_dict_add_string(result, "method", "textDocument/definition");

	params = lsp_dict_init();
	r |= lsp_dict_add_dict(params, "textDocument", lsp_fmt_textdocument(uri, NULL, "", 0));
	r |= lsp_dict_add_list(params, "position", lsp_fmt_position(atoi(argv[2]), atoi(argv[3])));
	r |= lsp_dict_add_dict(result, "params", params);

	free(uri);

	return (r == 0) ? RET_OK : RET_ERROR;
}

wiggler_retval_t cmd_symbols(int argc, char **argv, lsp_dict_t *result){
	int r = 0;
	char *uri;
	lsp_dict_t *params;


	if(file_params(argv[1], &uri, NULL, NULL) != 0)
		return -1;

	r |= lsp_dict_add_int(result, "id", lsp_id++);
	r |= lsp_dict_add_string(result, "method", "textDocument/documentSymbol");

	params = lsp_dict_init();
	r |= lsp_dict_add_dict(params, "textDocument", lsp_fmt_textdocument(uri, NULL, "", 0));
	r |= lsp_dict_add_dict(result, "params", params);

	free(uri);

	return (r == 0) ? RET_OK : RET_ERROR;
}


/* local functions */
int file_params(char const *path, char **uri, char **text, char **stem){
	*uri = malloc(PATH_MAX + 8);

	if(*uri == NULL)
		goto_err(err_0, "allocating string");

	strcpy(*uri, "file://");

	if(realpath(path, *uri + 7) == NULL)
		goto_err(err_0, "resolving path \"%s\"", path);

	if(stem != NULL)
		*stem = (char*)file_stem(*uri);

	if(text == NULL)
		return 0;

	*text = file_read(path, NULL);

	if(*text == NULL)
		goto_err(err_1, "reading file \"%s\"", path);

	return 0;


err_1:
	free(*uri);

err_0:
	return -1;
}

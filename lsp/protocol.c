/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <ctype.h>
#include <string.h>
#include <json-c/json.h>
#include <lsp/cmds.hash.h>
#include <utils/log.h>
#include <utils/string.h>
#include <lsp.h>


/* macros */
#define LSP_EXIT_INDICATOR	((void*)-1)


/* local/static prototypes */
static int read_header(size_t *len);
static lsp_dict_t *read_content();
static void send_response(void *result, int id);


/* static variables */
static lsp_ops_t lsp_ops = { 0x0 };


/* global functions */
void lsp_cmds_register(lsp_ops_t *ops){
	lsp_ops = *ops;
}

int lsp_cmds_process(){
	char const *method;
	int id;
	void *result;
	lsp_cmd_t const *cmd;
	lsp_dict_t *content,
			   *params;


	content = read_content();

	if(content == NULL)
		return -ERROR("parsing lsp json packet");

	method = lsp_dict_get_string(content, "method");
	params = lsp_dict_get_dict(content, "params");
	id = lsp_dict_has(content, "id") ? lsp_dict_get_int(content, "id") : -1;
	cmd = lsp_cmds_lookup(method, strlen(method));

	VERBOSE("lsp-cmd: id=%d, method=%s, has-hdlr=%d, has-params=%d", id, method, (cmd != NULL), (params != NULL));
	result = (cmd != NULL) ? cmd->hdlr(params) : NULL;

	if(id >= 0 && result != LSP_EXIT_INDICATOR)
		send_response(result, id);

	if(result != LSP_EXIT_INDICATOR)
		lsp_dict_free(result);

	lsp_dict_free(content);

	return (result == LSP_EXIT_INDICATOR);
}

void *lsp_result_error(char const *msg){
	ERROR("lsp handler failed with %s", msg);

	return json_object_new_string(msg);
}

void *lsp_result_exit(){
	return LSP_EXIT_INDICATOR;
}

void *lsp_initialize(lsp_dict_t *params){
	lsp_dict_t *result,
			   *capa,
			   *sync;


	capa = lsp_dict_init();

	sync = lsp_dict_init();
	lsp_dict_add_bool(sync, "openClose", (lsp_ops.doc_open != NULL));
	lsp_dict_add_int(sync, "change", (lsp_ops.doc_changed != NULL) ? 1 : 0);
	lsp_dict_add_dict(capa, "textDocumentSync", sync);

	lsp_dict_add_bool(capa, "definitionProvider", (lsp_ops.definition != NULL));
	lsp_dict_add_bool(capa, "documentSymbolProvider", (lsp_ops.symbols != NULL));

	if(lsp_ops.completion != NULL)
		lsp_dict_add_dict(capa, "completionProvider", lsp_dict_init());

	result = lsp_dict_init();
	lsp_dict_add_dict(result, "capabilities", capa);

	VERBOSE("initialise");

	return result;
}

void *lsp_shutdown(lsp_dict_t *params){
	VERBOSE("shutdown");

	return NULL;
}

void *lsp_exit(lsp_dict_t *params){
	VERBOSE("exit");

	return lsp_result_exit();
}

void *lsp_doc_open(lsp_dict_t *params){
	char const *path,
			   *text;


	path = lsp_uri_to_path(lsp_params_doc_uri(params));
	text = lsp_params_doc_text(params);

	VERBOSE("notification: open file=\"%s\"", path);

	return lsp_ops.doc_open(path, text);
}

void *lsp_doc_changed(lsp_dict_t *params){
	char const *path,
			   *text;


	path = lsp_uri_to_path(lsp_params_doc_uri(params));
	text = lsp_dict_get_string(lsp_list_get_idx(lsp_dict_get_list(params, "contentChanges"), 0), "text");

	VERBOSE("notification: changed file=\"%s\"", path);

	return lsp_ops.doc_changed(path, text);
}

void *lsp_definition(lsp_dict_t *params){
	char const *uri;
	size_t line,
		   column;


	uri = lsp_params_doc_uri(params);

	if(lsp_params_position(params, &line, &column) != 0)
		return lsp_result_error("invalid symbol position");

	VERBOSE("request: definition file=\"%s\", line=%zu, column=%zu", uri, line, column);

	return lsp_ops.definition(uri, line, column);
}

void *lsp_symbols(lsp_dict_t *params){
	char const *uri;


	uri = lsp_params_doc_uri(params);

	VERBOSE("request: symbols file=\"%s\"", uri);

	return lsp_ops.symbols(uri);
}

void *lsp_completion(lsp_dict_t *params){
	char const *uri;
	size_t line,
		   column;


	uri = lsp_params_doc_uri(params);

	if(lsp_params_position(params, &line, &column) != 0)
		return lsp_result_error("invalid symbol position");

	VERBOSE("request: completion file=\"%s\", line=%zu column=%zu", uri, line, column);

	return lsp_ops.completion(uri, line, column);
}


/* local functions */
static int read_header(size_t *len){
	char c;
	int r;


	/* match header start */
	r = strmatch("Content-Length: ", stdin);

	if(r != 0)
		return r;

	/* read length */
	*len = 0;

	while(1){
		if(fread(&c, 1, 1, stdin) != 1)
			return -1;

		if(c == '\r')
			break;

		if(!isdigit(c))
			return 1;

		*len = *len * 10 + c - '0';
	}

	/* match header end */
	return strmatch("\n\r\n", stdin);
}

static lsp_dict_t *read_content(){
	int r = 1;
	static char *content;
	size_t len;


	while(r > 0){
		r = read_header(&len);

		if(r < 0)
			return NULL;
	}

	content = alloca(len + 1);
	len = fread(content, 1, len, stdin);
	(content)[len] = 0;

	VERBOSE("lsp message content: %s", content);

	return json_tokener_parse(content);
}

static void send_response(void *result, int id){
	char const *s;
	lsp_dict_t *err;
	lsp_dict_t *resp;


	resp = lsp_dict_init();
	lsp_dict_add_int(resp, "id", id);
	lsp_dict_add_string(resp, "jsonrpc", "2.0");

	if(json_object_get_type(result) == json_type_string){
		err = lsp_dict_init();

		lsp_dict_add_string(err, "message", json_object_get_string(result));
		lsp_dict_add_int(err, "code", -1);
		lsp_dict_add_dict(resp, "error", err);
	}
	else
		lsp_dict_add_dict(resp, "result", json_object_get(result));

	s = json_object_to_json_string(resp);

	VERBOSE("response: %s", s);
	printf("Content-Length: %zu\r\n\r\n%s", strlen(s), s);
	fflush(stdout);

	lsp_dict_free(resp);
}

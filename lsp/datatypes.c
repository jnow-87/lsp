/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <string.h>
#include <json-c/json.h>
#include <lsp.h>


/* global functions */
lsp_dict_t *lsp_fmt_position(size_t line, size_t column){
	lsp_dict_t *pos;


	pos = lsp_dict_init();
	lsp_dict_add_int(pos, "line", line - 1); // lsp lines are zero-based
	lsp_dict_add_int(pos, "character", column);

	return pos;
}

lsp_dict_t *lsp_fmt_range(size_t line, size_t column){
	lsp_dict_t *range,
			   *pos;


	pos = lsp_fmt_position(line, column);

	range = lsp_dict_init();
	lsp_dict_add_dict(range, "start", pos);
	lsp_dict_add_dict(range, "end", json_object_get(pos));

	return range;
}

lsp_dict_t *lsp_fmt_textdocument(char const *uri, char const *text, char const *lang, int version){
	lsp_dict_t *params;


	params = lsp_dict_init();
	lsp_dict_add_string(params, "uri", uri);
	lsp_dict_add_string(params, "languageID", lang);
	lsp_dict_add_int(params, "version", version);

	if(text != NULL)
		lsp_dict_add_string(params, "text", text);

	return params;
}

char const *lsp_params_doc_uri(lsp_dict_t *params){
	return lsp_dict_get_string(lsp_dict_get_dict(params, "textDocument"), "uri");
}

char const *lsp_params_doc_text(lsp_dict_t *params){
	return lsp_dict_get_string(lsp_dict_get_dict(params, "textDocument"), "text");
}

int lsp_params_position(lsp_dict_t * params, size_t *line, size_t *column){
	lsp_dict_t *pos;


	pos = lsp_dict_get_dict(params, "position");

	if(pos == NULL || !lsp_dict_has(pos, "line") || !lsp_dict_has(pos, "character"))
		return -1;

	*line = lsp_dict_get_int(pos, "line") + 1;			// lsp lines are zero-based
	*column = lsp_dict_get_int(pos, "character") + 1;	// lsp columns are zero-based

	return 0;
}

char const *lsp_uri_to_path(char const *uri){
	if(uri != NULL && strncmp(uri, "file://", 7) == 0)
		return uri + 7;

	return uri;
}

lsp_dict_t *lsp_dict_init(){
	return json_object_new_object();
}

void lsp_dict_free(lsp_dict_t *dict){
	json_object_put(dict);
}

int lsp_dict_has(lsp_dict_t *dict, char const *key){
	return json_object_object_get_ex(dict, key, NULL);
}

int lsp_dict_get_int(lsp_dict_t *dict, char const *key){
	return json_object_get_int(json_object_object_get(dict, key));
}

bool lsp_dict_get_bool(lsp_dict_t *dict, char const *key){
	return json_object_get_boolean(json_object_object_get(dict, key));
}

char const *lsp_dict_get_string(lsp_dict_t *dict, char const *key){
	return json_object_get_string(json_object_object_get(dict, key));
}

lsp_dict_t *lsp_dict_get_dict(lsp_dict_t *dict, char const *key){
	return json_object_object_get(dict, key);
}

lsp_dict_t *lsp_dict_get_list(lsp_dict_t *dict, char const *key){
	return json_object_object_get(dict, key);
}

int lsp_dict_add_int(lsp_dict_t *dict, char const *key, int val){
	return json_object_object_add(dict, key, json_object_new_int(val));
}

int lsp_dict_add_bool(lsp_dict_t *dict, char const *key, bool val){
	return json_object_object_add(dict, key, json_object_new_boolean(val));
}

int lsp_dict_add_string(lsp_dict_t *dict, char const *key, char const *val){
	return json_object_object_add(dict, key, json_object_new_string(val));
}

int lsp_dict_add_dict(lsp_dict_t *dict, char const *key, lsp_dict_t *val){
	return json_object_object_add(dict, key, val);
}

int lsp_dict_add_list(lsp_dict_t *dict, char const *key, lsp_list_t *val){
	return json_object_object_add(dict, key, val);
}

lsp_list_t *lsp_list_init(){
	return json_object_new_array();
}

size_t lsp_list_get_len(lsp_list_t *lst){
	return json_object_array_length(lst);
}

lsp_dict_t *lsp_list_get_idx(lsp_list_t *lst, size_t idx){
		return json_object_array_get_idx(lst, idx);
}

int lsp_list_add(lsp_list_t *lst, lsp_dict_t *dict){
	return json_object_array_add(lst, dict);
}

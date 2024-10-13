/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <config/config.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>
#include <dtsls/config.h>
#include <utils/file.h>
#include <utils/log.h>


/* global variables */
config_t config = {
	.cwd = { .path = NULL, .fd = -1},
	.include_dirs = NULL,
	.nincludes = 0,
	.log_file = NULL,
	.verbose = false,
};


/* global functions */
int config_init(){
	size_t ndir = 0;
	char const *text;
	json_object *jobj,
				*jarr;
	dir_t dir;


	/* set cwd */
	config.cwd.path = getcwd(NULL, 0);

	if(config.cwd.path != NULL)
		config.cwd.fd = open(config.cwd.path, O_RDONLY);

	if(config.cwd.fd < 0)
		goto_err(err_0, "retrieving working directory");

	/* parse config file */
	// read file
	text = file_read(CONFIG_CONFIG_FILE, NULL);

	if(text == NULL){
		// having no config file is fine
		if(errno == ENOENT)
			return 0;

		goto_err(err_0, "reading config file \"%s\" failed", CONFIG_CONFIG_FILE);
	}

	// parse file
	jobj = json_tokener_parse(text);
	free((void*)text);

	if(jobj == NULL)
		goto_err(err_0, "parsing config file \"%s\" failed (json error)", CONFIG_CONFIG_FILE);

	// get config: log-file
	text = json_object_get_string(json_object_object_get(jobj, "log-file"));

	if(text != NULL)
		config.log_file = strdup(text);

	// get config: verbose
	config.verbose = json_object_get_boolean(json_object_object_get(jobj, "verbose"));

	// get config: include-dirs
	jarr = json_object_object_get(jobj, "include-dirs");
	ndir = json_object_array_length(jarr);

	config.include_dirs = calloc(1, sizeof(dir_t) * ndir);

	if(config.include_dirs == NULL)
		goto_err(err_1, "allocating config");

	for(size_t i=0; i<ndir; i++){
		text = json_object_get_string(json_object_array_get_idx(jarr, i));
		dir.path = realpath(text, NULL);

		if(dir.path != NULL)
			dir.fd = open(dir.path, O_RDONLY);

		if(dir.path != NULL || dir.fd >= 0){
			config.include_dirs[config.nincludes] = dir;
			config.nincludes++;
		}
		else
			LOG("ignoring invalid include-dir \"%s\": %s", text, strerror(errno));
	}

	json_object_put(jobj);

	return 0;


err_1:
	json_object_put(jobj);

err_0:
	config_free();

	return -1;
}

void config_free(){
	for(size_t i=0; i<config.nincludes; i++){
		close(config.include_dirs[i].fd);
		free((void*)config.include_dirs[i].path);
	}

	free(config.include_dirs);
	free((void*)config.log_file);

	free((void*)config.cwd.path);
	close(config.cwd.fd);
}

void config_print(){
	LOG("config:");
	LOG("  log-file: %s", config.log_file);
	LOG("  verbose: %d", config.verbose);
	LOG("  cwd: %s", config.cwd.path);

	LOG("  include directories (%zu):", config.nincludes);

	for(size_t i=0; i<config.nincludes; i++)
		LOG("    %s", config.include_dirs[i].path);
}

/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */




#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <utils/log.h>
#include <wiggler/cmds.h>
#include <wiggler/cmds.hash.h>
#include <lsp.h>


/* local/static prototypes */
static int user_input(char **line, int *argc, char ***argv);
static int process_cmd(int argc, char **argv);
static int splitline(char *line, int *argc, char ***argv);


/* global functions */
int main(){
	int argc = 0;
	char *line = NULL,
		 **argv = NULL;
	wiggler_retval_t r = RET_OK;


	LOG("use 'help' for info on available commands\n");
	rl_outstream = stderr;

	while(r != RET_EXIT){
		free(line);
		r = user_input(&line, &argc, &argv);

		if(r == RET_OK && argc > 0)
			r = process_cmd(argc, argv);
	}

	free(line);
	free(argv);

	return 0;
}


/* local functions */
static int user_input(char **line, int *argc, char ***argv){
	*line = readline("$> ");

	if(*line == NULL){
		if(errno != 0)
			return ERROR("readline failed with %s", strerror(errno));

		return RET_EXIT;
	}

	if((*line)[0] == 0)
		return RET_NOACTION;

	if(splitline(*line, argc, argv) != 0)
		return ERROR("split line");

	return 0;
}

static int process_cmd(int argc, char **argv){
	char const *s;
	wiggler_retval_t r;
	wiggler_cmd_t const *cmd;
	lsp_dict_t *result;


	cmd = wiggler_cmds_lookup(argv[0], strlen(argv[0]));

	if(cmd == NULL)
		return ERROR("invalid cmd");

	if(argc != cmd->nargs + 1)
		return ERROR("invalid number of arguments (%zu) expected %zu\n  %s %s", argc - 1, cmd->nargs, cmd->cmd, cmd->args);

	result = lsp_dict_init();
	lsp_dict_add_string(result, "jsonrpc", "2.0");

	r = cmd->hdlr(argc, argv, result);

	if(r != RET_ERROR && r != RET_NOACTION){
		s = json_object_to_json_string(result);

		if(s != NULL){
			LOG("send: \"%s\"", s);
			printf("Content-Length: %zu\r\n\r\n%s", strlen(s), s);
			fflush(stdout);
		}
	}

	lsp_dict_free(result);

	return r;
}

static int splitline(char *line, int *argc, char ***argv){
	size_t len = strlen(line);
	size_t n = 0;


	for(size_t i=0; i<len; i++){
		if(isblank(line[i]))
			continue;

		n++;
		while(i < len && !isblank(line[i + 1])) i++;
	}

	if(*argc < n || *argv == NULL){
		free(*argv);
		*argv = malloc(n * sizeof(char**));

		if(*argv == NULL)
			return -1;
	}

	*argc = n;

	for(size_t i=0, j=0; i<len; i++){
		if(isblank(line[i]))
			continue;

		(*argv)[j++] = line + i;
		while(i < len && !isblank(line[i])) i++;
		line[i] = 0;
	}

	return 0;
}

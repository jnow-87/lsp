/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <dtsls/config.h>
#include <dtsls/lsp.h>
#include <dtsls/symtab.h>
#include <utils/log.h>
#include <lsp.h>


/* global functions */
int main(){
	if(config_init() != 0)
		return 1;

	log_init(config.log_file, config.verbose);
	lsp_init();
	config_print();

	// errors during command processing shall not exit the application,
	// this shall only happen through explicit request, indicated through
	// return values larger than 0
	while(lsp_cmds_process() <= 0);

	log_close();
	config_free();
	symtab_free();

	return 0;
}

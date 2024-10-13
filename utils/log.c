/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/log.h>


/* static variables */
static int log_fd = 2;
static bool log_verbose = false;


/* global functions */
void log_init(char const *file, bool verbose){
	log_verbose = verbose;

	if(file == NULL)
		return;

	log_fd = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if(log_fd >= 0)
		return;

	log_fd = 2;
	ERROR("opening log-file \"%s\", falling back to stderr", file);
}

void log_close(){
	if(log_fd != 2)
		close(log_fd);
}

int log_filed(){
	return log_fd;
}

int log_print(log_lvl_t lvl, char const *fmt, ...){
	int r;
	va_list lst;


	va_start(lst, fmt);
	r = log_vprint(lvl, fmt, lst);
	va_end(lst);

	return r;
}

int log_vprint(log_lvl_t lvl, char const *fmt, va_list lst){
	if(lvl == LOG_VERBOSE && !log_verbose)
		return 0;

	if(lvl == LOG_ERROR)
		dprintf(log_fd, "error: ");

	vdprintf(log_fd, fmt, lst);

	if(lvl == LOG_ERROR && errno)
		dprintf(log_fd, ": %s", strerror(errno));

	dprintf(log_fd, "\n");

	return -(lvl == LOG_ERROR);
}

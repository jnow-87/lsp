/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LOG_H
#define LOG_H


#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>


/* macros */
#define ERROR(fmt, ...)		log_print(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG(fmt, ...)		log_print(LOG_NORMAL, fmt, ##__VA_ARGS__)
#define VERBOSE(fmt, ...)	log_print(LOG_VERBOSE, fmt, ##__VA_ARGS__)

#define goto_err(label, fmt, ...)	{ ERROR(fmt, ##__VA_ARGS__); goto label; }


/* types */
typedef enum{
	LOG_NORMAL = 1,
	LOG_VERBOSE,
	LOG_ERROR,
} log_lvl_t;


/* prototypes */
void log_init(char const *file, bool verbose);
void log_close();
int log_filed();

int log_print(log_lvl_t lvl, char const *fmt, ...);
int log_vprint(log_lvl_t lvl, char const *fmt, va_list lst);


#endif // LOG_H

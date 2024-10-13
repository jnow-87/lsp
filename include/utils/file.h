/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef FILE_H
#define FILE_H


#include <sys/types.h>


/* prototypes */
char *file_read(char const *path, size_t *size);
int file_locate_identifier(char const *text, size_t line, size_t column, char *idfr, size_t idfr_max, bool trunc);
char const *file_stem(char const *path);


#endif // FILE_H

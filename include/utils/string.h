/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef STRUTILS_H
#define STRUTILS_H


#include <stdio.h>


/* prototypes */
char *stralloc(char const *s, size_t n);
int strmatch(char const *s, FILE *fp);


#endif // STRUTILS_H

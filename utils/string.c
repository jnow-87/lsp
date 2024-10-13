/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* global functions */
char *stralloc(char const *s, size_t n){
	char *x;


	x = malloc(n + 1);

	if(x == NULL)
		return NULL;

	memcpy(x, s, n);
	x[n] = 0;

	return x;
}

int strmatch(char const *s, FILE *fp){
	size_t len = strlen(s);
	char c;


	for(size_t i=0; i<len; i++){
		if(fread(&c, 1, 1, fp) != 1)
			return -1; // stream error

		if(c != s[i])
			return 1; // indicate retry
	}

	return 0; // success
}

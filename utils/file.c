/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>


/* local/static prototypes */
static int iskeyword(char c);


/* global functions */
char *file_read(char const *path, size_t *size){
	char *text;
	FILE *fp;
	struct stat st;


	if(stat(path, &st) != 0)
		return NULL;

	fp = fopen(path, "r");

	if(fp == NULL)
		return NULL;

	text = malloc(st.st_size + 1);

	if(text != NULL){
		if(size != NULL)
			*size = st.st_size;

		fread(text, st.st_size, 1, fp);
		text[st.st_size] = 0;
	}

	fclose(fp);

	return text;
}

int file_locate_identifier(char const *text, size_t line, size_t column, char *idfr, size_t idfr_max, bool trunc){
	size_t i = 0;
	size_t start;


	/* locate character in text */
	// move to line
	while(line > 1 && text[i]!=0){
		if(text[i++] == '\n')
			line--;
	}

	// move to column
	for(; column>1 && text[i]!=0; column--){
		if(text[i++] == '\n')
			return -1;
	}

	// check character
	if(!iskeyword(text[i]))
		return -1;

	/* extract identifier */
	// find start
	for(start=i; start>0 && iskeyword(text[start - 1]); start--);

	// find end
	if(!trunc)
		for(; iskeyword(text[i + 1]); i++);

	/* copy text */
	i = MIN(idfr_max - 1, i - start + 1);
	strncpy(idfr, text + start, i);
	idfr[i] = 0;

	return 0;
}

char const *file_stem(char const *path){
	size_t len = strlen(path);


	for(size_t i=len-1; i>=0; i--){
		if(path[i] == '.')
			return path + i + 1;
	}

	return path + len;
}


/* local functions */
static int iskeyword(char c){
	return (isalnum(c) || c == '_');
}

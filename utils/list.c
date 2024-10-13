/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <utils/list.h>


/* global functoins */
int list_add(list_t **head, void *payload){
	list_t *el;


	if(payload == NULL)
		return 0;

	el = malloc(sizeof(list_t));

	if(el == NULL)
		return -1;

	el->payload = payload;

	if(*head != NULL){
		el->prev = (*head)->prev;
		(*head)->prev->next = el;
	}
	else
		*head = el;

	el->next = NULL;
	(*head)->prev = el;

	return 0;
}

void list_rm(list_t **head, list_t *el){
	if(el == NULL)
		return;

	if(el != *head)
		el->prev->next = el->next;

	if(el->next != NULL)	el->next->prev = el->prev;
	else					(*head)->prev = el->prev;

	if(el == *head)			*head = el->next;

	free(el);
}

list_t *list_find(list_t *head, void *payload){
	for(; head!=NULL; head=head->next){
		if(head->payload == payload)
			return head;
	}

	return NULL;
}

void list_free(list_t **head){
	while(*head != NULL)
		list_rm(head, *head);
}

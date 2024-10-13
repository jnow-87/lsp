/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIST_H
#define LIST_H


/* macros */
#define list_for_each(head, obj) \
	for( \
		typeof(head) next=((head) == NULL ? NULL : (head)->next), el=head; \
		el!=NULL && (obj=el->payload); \
		el=next, next=(next == NULL ? 0 : next->next) \
	)


/* types */
typedef struct list_t{
	struct list_t *prev,
				  *next;

	void *payload;
} list_t;


/* prototypes */
int list_add(list_t **head, void *payload);
void list_rm(list_t **head, list_t *el);
list_t *list_find(list_t *head, void *payload);

void list_free(list_t **head);


#endif // LIST_H

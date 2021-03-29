#ifndef _DOUBLE_LINKED_LIST_H
#define _DOUBLE_LINKED_LIST_H

#include "stdint.h"


#if defined( __cplusplus )
extern "C" {
#endif

#define LIST_FOR_EACH(list,type,var) for(var = (type*)list_get_first_node(list); var != NULL; var=(type*)list_get_next_node(&var->node))

typedef struct list_node		
{
	struct list_node *prior, *next;	 
}node_t;

typedef struct 		
{
	node_t  node;	 
	int size;
}list_t;

void list_init(list_t* plist);

int list_get_size(list_t* plist);

void list_add_from_tail(list_t* plist, node_t* pnode);

void list_delete_node(list_t* plist, node_t* pnode);

node_t* list_get_first_node(list_t* plist);

node_t* list_get_next_node(node_t* pnode);


#if defined( __cplusplus )
}
#endif


#endif /* end of _LST_LIB_H */

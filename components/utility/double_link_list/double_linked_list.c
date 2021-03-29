#include <stdlib.h>
#include <stdio.h>

#include "double_linked_list.h"

#define head	node.next	
#define tail	node.prior 


void list_init(list_t* plist)
{
	if(NULL == plist)
	{
		return ;
	}
	plist->head = NULL;
	plist->tail   = NULL;
	plist->size   = 0;
}

int list_get_size(list_t* plist)
{
	if(NULL == plist)
	{
		return -1;
	}

	return plist->size;
}


void list_add_from_tail(list_t* plist, node_t* pnode)
{
	node_t* temp;
	if(NULL == plist||NULL == pnode)
	{
		return ;
	}
   
    if( 0 == plist->size)
	{
        plist->head = pnode;
	}
	else
	{
        temp = plist->tail;
		temp->next = pnode;
		pnode->prior = temp;
	} 

	plist->tail = pnode;
	plist->size++;
}

void list_delete_node(list_t* plist, node_t* pnode)
{
	if(NULL == plist||NULL == pnode)
	{
		return ;
	}
	if(NULL== pnode->prior)
	{
		plist->head = pnode->next;
	}
	else
	{
		pnode->prior->next = pnode->next;
	}

	if(NULL == pnode->next)
	{
		plist->tail = pnode->prior;
	}
	else
	{
		pnode->next->prior = pnode->prior;
	}

	plist->size--;
}

node_t* list_get_first_node(list_t* plist)
{
	if(NULL==plist)
	{
		return NULL;
	}
	return plist->head;
}

node_t* list_get_next_node(node_t* pnode)
{
	if(NULL == pnode)
	{
		return NULL;
	}

	return pnode->next;
}


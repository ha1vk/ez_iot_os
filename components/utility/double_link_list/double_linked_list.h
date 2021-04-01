/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
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

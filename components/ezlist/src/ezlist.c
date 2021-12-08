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
 * 
 * Contributors:
 *    xurongjun - Doubly linked list interface implement
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-04     xurongjun    first version
 *******************************************************************************/

#include "ezlist.h"
#include <stdlib.h>
#include <stdio.h>

void ezlist_init(ez_list_t *plist)
{
    plist->header.prev = NULL;
    plist->header.next = NULL;
    plist->size = 0;
}

ez_node_t *ezlist_get_first(ez_list_t *plist)
{
    return plist->header.next;
}

ez_node_t *ezlist_get_last(ez_list_t *plist)
{
    return plist->header.prev;
}

ez_node_t *ezlist_get_at(ez_list_t *plist, unsigned int index)
{
    ez_node_t *pnode;

    if ((index < 1) || (index > plist->size))
        return (NULL);

    if (index < (plist->size >> 1))
    {
        pnode = plist->header.next;

        while (--index > 0)
            pnode = pnode->next;
    }
    else
    {
        index -= plist->size - index;
        pnode = plist->header.prev;

        while (index-- > 0)
            pnode = pnode->prev;
    }

    return pnode;
}

ez_node_t *ezlist_get_prev(ez_node_t *pnode)
{
    return pnode->prev;
}

ez_node_t *ezlist_get_next(ez_node_t *pnode)
{
    return pnode->next;
}

int ezlist_get_size(ez_list_t *plist)
{
    return plist->size;
}

void ezlist_add_last(ez_list_t *plist, ez_node_t *pnode)
{
    ezlist_add_at(plist, plist->header.prev, pnode);
}

void ezlist_add_at(ez_list_t *plist, ez_node_t *pprev, ez_node_t *pnode)
{
    ez_node_t *pnode_next;

    if (NULL == pprev)
    {
        pnode_next = plist->header.next;
        plist->header.next = pnode;
    }
    else
    {
        pnode_next = pprev->next;
        pprev->next = pnode;
    }

    if (NULL == pnode_next)
        plist->header.prev = pnode;
    else
        pnode_next->prev = pnode;

    pnode->next = pnode_next;
    pnode->prev = pprev;

    plist->size++;
}

void ezlist_delete(ez_list_t *plist, ez_node_t *pnode)
{
    int firstNode = 0, lastNode = 0;

    if (pnode->prev == NULL)
    {
        if (plist->header.next != pnode)
        {
            return;
        }

        firstNode = 1;
    }
    else
    {
        if (pnode->prev->next != pnode)
        {
            return;
        }

        firstNode = 0;
    }

    if (pnode->next == NULL)
    {
        if (plist->header.prev != pnode)
        {
            return;
        }

        lastNode = 1;
    }
    else
    {
        if (pnode->next->prev != pnode)
        {
            return;
        }

        lastNode = 0;
    }

    if (1 == firstNode)
    {
        plist->header.next = pnode->next;
    }
    else
    {
        pnode->prev->next = pnode->next;
    }

    if (1 == lastNode)
    {
        plist->header.prev = pnode->prev;
    }
    else
    {
        pnode->next->prev = pnode->prev;
    }

    plist->size--;
}

void ezlist_clear(ez_list_t *plist)
{
    if (NULL == plist)
    {
        return;
    }

    ez_node_t *p1, *p2;

    if (plist->size > 0)
    {
        p1 = plist->header.next;
        while (p1 != NULL)
        {
            p2 = p1->next;
            free((char *)p1);
            p1 = p2;
        }

        plist->size = 0;
        plist->header.next = plist->header.prev = NULL;
    }
}

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
 *    xurongjun - Doubly linked list interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-04     xurongjun    first version
 *******************************************************************************/

#ifndef _EZLIST_H_
#define _EZLIST_H_

#define LIST_FOR_EACH(type, var, list) for (var = (type *)ezlist_get_first(list); var != NULL; var = (type *)ezlist_get_next(&var->node))

#if defined(__cplusplus)
extern "C"
{
#endif

    typedef struct ez_node
    {
        struct ez_node *next;
        struct ez_node *prev;
    } ez_node_t;

    typedef struct list
    {
        ez_node_t header;
        int size;
    } ez_list_t;

    /**
     * @brief Initialize the list
     * 
     * @param plist 
     */
    void ezlist_init(ez_list_t *plist);

    /**
     * @brief Get first node in this list
     * 
     * @param plist 
     * @return node_t* 
     */
    ez_node_t *ezlist_get_first(ez_list_t *plist);

    /**
     * @brief Get last node in this list
     * 
     * @param plist 
     * @return node_t* 
     */
    ez_node_t *ezlist_get_last(ez_list_t *plist);

    /**
     * @brief Get the node at the specified position in this list
     * 
     * @param plist 
     * @return node_t* 
     */
    ez_node_t *ezlist_get_at(ez_list_t *plist, unsigned int index);

    /**
     * @brief Get previous node in this list
     * 
     * @param pnode 
     * @return node_t* 
     */
    ez_node_t *ezlist_get_prev(ez_node_t *pnode);

    /**
     * @brief  Get next node in this list.
     * 
     * @param pnode 
     * @return node_t* 
     */
    ez_node_t *ezlist_get_next(ez_node_t *pnode);

    /**
     * @brief Get the number of nodes in this list
     * 
     * @param plist 
     * @return int 
     */
    int ezlist_get_size(ez_list_t *plist);

    /**
     * @brief add a node to the end of this list
     * 
     * @param plist 
     * @param pnode 
     */
    void ezlist_add_last(ez_list_t *plist, ez_node_t *pnode);

    /**
     * @brief Inserts a node at the specified position in this list
     * 
     * @param plist 
     * @param pprev 
     * @param pnode 
     */
    void ezlist_add_at(ez_list_t *plist, ez_node_t *pprev, ez_node_t *pnode);

    /**
     * @brief delete the specified node from list
     * 
     * @param plist 
     * @param pnode 
     */
    void ezlist_delete(ez_list_t *plist, ez_node_t *pnode);

    /**
     * @brief free all of the node from this list.
     * 
     * @param plist 
     */
    void ezlist_clear(ez_list_t *plist);

#if defined(__cplusplus)
}
#endif

#endif /* end of _LST_LIB_H */

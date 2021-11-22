
#include <ezos.h>
#include <ezlist.h>

#ifndef _DOXYGEN_SKIP

static void *get_at(ezlist_t *list, int index, size_t *size, int newmem, int remove);
static ezlist_obj_t *get_obj(ezlist_t *list, int index);
static int remove_obj(ezlist_t *list, ezlist_obj_t *obj);

#endif

/**
 * Create new ezlist_t linked-list container
 *
 * @param options   combination of initialization options.
 *
 * @return a pointer of malloced ezlist_t container, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *  ezlist_t *list = ezlist(0);
 * @endcode
 *
 * @note
 *   Available options:
 *   - ezlist_THREADSAFE - make it thread-safe.
 */
ezlist_t *ezlist(int options) {
    ezlist_t *list = (ezlist_t *) ezos_calloc(1, sizeof(ezlist_t));
    if (list == NULL) {
        return NULL;
    }

    // handle options.
    if (options & ezlist_THREADSAFE) {
        list->mutex = ezos_mutex_create();
        if (list->mutex == NULL) {
            ezos_free(list);
            return NULL;
        }
    }

    return list;
}

/**
 * ezlist->setsize(): Limit maximum number of elements allowed in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param max   maximum number of elements. 0 means no limit.
 *
 * @return previous maximum number.
 *
 * @note
 *  The default maximum number of elements is unlimited.
 */
size_t ezlist_setsize(ezlist_t *list, size_t max) {
    ezlist_lock(list);
    size_t old = list->max;
    list->max = max;
    ezlist_unlock(list);
    return old;
}

/**
 * ezlist->addfirst(): Inserts a element at the beginning of this list.
 *
 * @param list  ezlist_t container pointer.
 * @param data  a pointer which points data memory.
 * @param size  size of the data.
 *
 * @return 1 if successful, otherwise returns 0.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 *
 * @code
 *  // create a sample object.
 *  struct my_obj obj;
 *
 *  // create a list and add the sample object.
 *  ezlist_t *list = ezlist();
 *  list->addfirst(list, &obj, sizeof(struct my_obj));
 * @endcode
 */
int ezlist_addfirst(ezlist_t *list, const void *data, size_t size) {
    return ezlist_addat(list, 0, data, size);
}

/**
 * ezlist->addlast(): Appends a element to the end of this list.
 *
 * @param list  ezlist_t container pointer.
 * @param data  a pointer which points data memory.
 * @param size  size of the data.
 *
 * @return 1 if successful, otherwise returns 0.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 */
int ezlist_addlast(ezlist_t *list, const void *data, size_t size) {
    return ezlist_addat(list, -1, data, size);
}

/**
 * ezlist->addat(): Inserts a element at the specified position in this
 * list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be inserted.
 * @param data   a pointer which points data memory.
 * @param size   size of the data.
 *
 * @return 1 if successful, otherwise returns 0.
 * @retval errno will be set in error condition.
 *  - ENOBUFS : List full. Only happens when this list has set to have limited
 *              number of elements.
 *  - ERANGE  : Index out of range.
 *  - EINVAL  : Invalid argument.
 *  - ENOMEM  : Memory allocation failure.
 *
 * @code
 *                     first           last      new
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]?==?[   ]
 *  (positive index)     0       1       2        3
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @code
 *  ezlist_t *list = ezlist();
 *  list->addat(list, 0, &obj, sizeof(obj));  // same as addfirst().
 *  list->addat(list, -1, &obj, sizeof(obj)); // same as addlast().
 * @endcode
 *
 * @note
 *  Index starts from 0.
 */
int ezlist_addat(ezlist_t *list, int index, const void *data, size_t size) {
    // check arguments
    if (data == NULL || size <= 0) {
        return 0;
    }

    ezlist_lock(list);

    // check maximum number of allowed elements if set
    if (list->max > 0 && list->num >= list->max) {
        ezlist_unlock(list);
        return 0;
    }

    // adjust index
    if (index < 0)
        index = (list->num + index) + 1;  // -1 is same as addlast()
    if (index < 0 || index > list->num) {
        // out of bound
        ezlist_unlock(list);
        return 0;
    }

    // duplicate object
    void *dup_data = ezos_malloc(size);
    if (dup_data == NULL) {
        ezlist_unlock(list);
        return 0;
    }
    ezos_memcpy(dup_data, data, size);

    // make new object list
    ezlist_obj_t *obj = (ezlist_obj_t *) ezos_malloc(sizeof(ezlist_obj_t));
    if (obj == NULL) {
        ezos_free(dup_data);
        ezlist_unlock(list);
        return 0;
    }
    obj->data = dup_data;
    obj->size = size;
    obj->prev = NULL;
    obj->next = NULL;

    // make link
    if (index == 0) {
        // add at first
        obj->next = list->first;
        if (obj->next != NULL)
            obj->next->prev = obj;
        list->first = obj;
        if (list->last == NULL)
            list->last = obj;
    } else if (index == list->num) {
        // add after last
        obj->prev = list->last;
        if (obj->prev != NULL)
            obj->prev->next = obj;
        list->last = obj;
        if (list->first == NULL)
            list->first = obj;
    } else {
        // add at the middle of list
        ezlist_obj_t *tgt = get_obj(list, index);
        if (tgt == NULL) {
            // should not be happened.
            ezos_free(dup_data);
            ezos_free(obj);
            ezlist_unlock(list);
            return 0;
        }

        // insert obj
        tgt->prev->next = obj;
        obj->prev = tgt->prev;
        obj->next = tgt;
        tgt->prev = obj;
    }

    list->datasum += size;
    list->num++;

    ezlist_unlock(list);

    return 1;
}

/**
 * ezlist->getfirst(): Returns the first element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  - ENOENT : List is empty.
 *  - ENOMEM : Memory allocation failure.
 *
 * @code
 *  size_t size;
 *  void *data = list->getfirst(list, &size, 1);
 *  if (data != NULL) {
 *    (...omit...)
 *    ezos_free(data);
 *  }
 * @endcode
 */
void *ezlist_getfirst(ezlist_t *list, size_t *size, int newmem) {
    return ezlist_getat(list, 0, size, newmem);
}

/**
 * ezlist->getlast(): Returns the last element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *        ENOENT : List is empty.
 *        ENOMEM : Memory allocation failure.
 */
void *ezlist_getlast(ezlist_t *list, size_t *size, int newmem) {
    return ezlist_getat(list, -1, size, newmem);
}

/**
 * ezlist->getat(): Returns the element at the specified position in this
 * list.
 *
 * @param list    ezlist_t container pointer.
 * @param index   index at which the specified element is to be inserted
 * @param size    if size is not NULL, element size will be stored.
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return a pointer of element, otherwise returns NULL.
 * @retval errno
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *                     first           last
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]
 *  (positive index)     0       1       2
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @note
 *  Negative index can be used for addressing a element from the end in this
 *  stack. For example, index -1 is same as getlast() and index 0 is same as
 *  getfirst();
 */
void *ezlist_getat(ezlist_t *list, int index, size_t *size, int newmem) {
    return get_at(list, index, size, newmem, 0);
}

/**
 * ezlist->popfirst(): Returns and remove the first element in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_popfirst(ezlist_t *list, size_t *size) {
    return ezlist_popat(list, 0, size);
}

/**
 * ezlist->getlast(): Returns and remove the last element in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_poplast(ezlist_t *list, size_t *size) {
    return ezlist_popat(list, -1, size);
}

/**
 * ezlist->popat(): Returns and remove the element at the specified
 * position in this list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be inserted
 * @param size   if size is not NULL, element size will be stored.
 *
 * @return a pointer of malloced element, otherwise returns NULL.
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 *  -ENOMEM : Memory allocation failure.
 *
 * @code
 *                     first           last
 *  Linked-list        [ A ]<=>[ B ]<=>[ C ]
 *  (positive index)     0       1       2
 *  (negative index)    -3      -2      -1
 * @endcode
 *
 * @note
 *  Negative index can be used for addressing a element from the end in this
 *  stack. For example, index -1 is same as poplast() and index 0 is same as
 *  popfirst();
 */
void *ezlist_popat(ezlist_t *list, int index, size_t *size) {
    return get_at(list, index, size, 1, 1);
}

/**
 * ezlist->removefirst(): Removes the first element in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 */
int ezlist_removefirst(ezlist_t *list) {
    return ezlist_removeat(list, 0);
}

/**
 * ezlist->removelast(): Removes the last element in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 */
int ezlist_removelast(ezlist_t *list) {
    return ezlist_removeat(list, -1);
}

/**
 * ezlist->removeat(): Removes the element at the specified position in
 * this list.
 *
 * @param list   ezlist_t container pointer.
 * @param index  index at which the specified element is to be removed.
 *
 * @return a number of removed objects.
 * @retval errno will be set in error condition.
 *  -ERANGE : Index out of range.
 */
int ezlist_removeat(ezlist_t *list, int index) {
    ezlist_lock(list);

    // get object pointer
    ezlist_obj_t *obj = get_obj(list, index);
    if (obj == NULL) {
        ezlist_unlock(list);
        return 0;
    }

    int ret = remove_obj(list, obj);

    ezlist_unlock(list);

    return ret;
}

/**
 * ezlist->getnext(): Get next element in this list.
 *
 * @param list    ezlist_t container pointer.
 * @param obj     found data will be stored in this structure
 * @param newmem  whether or not to allocate memory for the element.
 *
 * @return 1 if found otherwise returns 0
 * @retval errno will be set in error condition.
 *  -ENOENT : No next element.
 *  -ENOMEM : Memory allocation failure.
 *
 * @note
 *  obj should be initialized with 0 by using memset() before first call.
 *  If newmem flag is 1, user should de-allocate obj.name and obj.data
 *  resources.
 *
 * @code
 *  ezlist_t *list = ezlist();
 *  (...add data into list...)
 *
 *  ezlist_obj_t obj;
 *  memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *  list->lock(list);   // can be omitted in single thread model.
 *  while (list->getnext(list, &obj, 0) == 1) {
 *   printf("DATA=%s, SIZE=%zu\n", (char*)obj.data, obj.size);
 *  }
 *  list->unlock(list); // release lock.
 * @endcode
 */
int ezlist_getnext(ezlist_t *list, ezlist_obj_t *obj, int newmem) {
    if (obj == NULL)
        return 0;

    ezlist_lock(list);

    ezlist_obj_t *cont = NULL;
    if (obj->size == 0)
        cont = list->first;
    else
        cont = obj->next;

    if (cont == NULL) {
        ezlist_unlock(list);
        return 0;
    }

    int ret = 0;
    while (cont != NULL) {
        if (newmem == 1) {
            obj->data = ezos_malloc(cont->size);
            if (obj->data == NULL)
                break;

            ezos_memcpy(obj->data, cont->data, cont->size);
        } else {
            obj->data = cont->data;
        }
        obj->size = cont->size;
        obj->prev = cont->prev;
        obj->next = cont->next;

        ret = 1;
        break;
    }

    ezlist_unlock(list);
    return ret;
}

/**
 * ezlist->size(): Returns the number of elements in this list.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return the number of elements in this list.
 */
size_t ezlist_size(ezlist_t *list) {
    return list->num;
}

/**
 * ezlist->size(): Returns the sum of total element size.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return the sum of total element size.
 */
size_t ezlist_datasize(ezlist_t *list) {
    return list->datasum;
}

/**
 * ezlist->reverse(): Reverse the order of elements.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_reverse(ezlist_t *list) {
    ezlist_lock(list);
    ezlist_obj_t *obj;
    for (obj = list->first; obj;) {
        ezlist_obj_t *next = obj->next;
        obj->next = obj->prev;
        obj->prev = next;
        obj = next;
    }

    obj = list->first;
    list->first = list->last;
    list->last = obj;

    ezlist_unlock(list);
}

/**
 * ezlist->clear(): Removes all of the elements from this list.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_clear(ezlist_t *list) {
    ezlist_lock(list);
    ezlist_obj_t *obj;
    for (obj = list->first; obj;) {
        ezlist_obj_t *next = obj->next;
        ezos_free(obj->data);
        ezos_free(obj);
        obj = next;
    }

    list->num = 0;
    list->datasum = 0;
    list->first = NULL;
    list->last = NULL;
    ezlist_unlock(list);
}

/**
 * ezlist->toarray(): Returns the serialized chunk containing all the
 * elements in this list.
 *
 * @param list  ezlist_t container pointer.
 * @param size  if size is not NULL, chunk size will be stored.
 *
 * @return a malloced pointer,
 *  otherwise(if there is no data to merge) returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 */
void *ezlist_toarray(ezlist_t *list, size_t *size) {
    if (list->num <= 0) {
        if (size != NULL)
            *size = 0;
        return NULL;
    }

    ezlist_lock(list);

    void *chunk = ezos_malloc(list->datasum);
    if (chunk == NULL) {
        ezlist_unlock(list);
        return NULL;
    }
    void *dp = chunk;

    ezlist_obj_t *obj;
    for (obj = list->first; obj; obj = obj->next) {
        ezos_memcpy(dp, obj->data, obj->size);
        dp += obj->size;
    }
    ezlist_unlock(list);

    if (size != NULL)
        *size = list->datasum;
    return chunk;
}

/**
 * ezlist->tostring(): Returns a string representation of this list,
 * containing string representation of each element.
 *
 * @param list  ezlist_t container pointer.
 *
 * @return a malloced pointer,
 *  otherwise(if there is no data to merge) returns NULL.
 * @retval errno will be set in error condition.
 *  -ENOENT : List is empty.
 *  -ENOMEM : Memory allocation failure.
 *
 * @note
 *  Return string is always terminated by '\0'.
 */
char *ezlist_tostring(ezlist_t *list) {
    if (list->num <= 0) {
        return NULL;
    }

    ezlist_lock(list);

    void *chunk = ezos_malloc(list->datasum + 1);
    if (chunk == NULL) {
        ezlist_unlock(list);
        return NULL;
    }
    void *dp = chunk;

    ezlist_obj_t *obj;
    for (obj = list->first; obj; obj = obj->next) {
        size_t size = obj->size;
        // do not copy tailing '\0'
        if (*(char *) (obj->data + (size - 1)) == '\0')
            size -= 1;
        ezos_memcpy(dp, obj->data, size);
        dp += size;
    }
    *((char *) dp) = '\0';
    ezlist_unlock(list);

    return (char *) chunk;
}

/**
 * ezlist->lock(): Enters critical section.
 *
 * @param list  ezlist_t container pointer.
 *
 * @note
 *  From user side, normally locking operation is only needed when traverse all
 *  elements using ezlist->getnext().
 */
void ezlist_lock(ezlist_t *list) {
    ezos_mutex_lock(list->mutex);
}

/**
 * ezlist->unlock(): Leaves critical section.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_unlock(ezlist_t *list) {
    ezos_mutex_unlock(list->mutex);
}

/**
 * ezlist->ezos_free(): Free ezlist_t.
 *
 * @param list  ezlist_t container pointer.
 */
void ezlist_ezos_free(ezlist_t *list) {
    ezlist_clear(list);
    ezos_mutex_destory(list->mutex);

    ezos_free(list);
}

#ifndef _DOXYGEN_SKIP

static void *get_at(ezlist_t *list, int index, size_t *size, int newmem,
int remove) {
    ezlist_lock(list);

    // get object pointer
    ezlist_obj_t *obj = get_obj(list, index);
    if (obj == NULL) {
        ezlist_unlock(list);
        return 0;
    }

    // copy data
    void *data;
    if (newmem == 1) {
        data = ezos_malloc(obj->size);
        if (data == NULL) {
            ezlist_unlock(list);
            return 0;
        }
        ezos_memcpy(data, obj->data, obj->size);
    } else {
        data = obj->data;
    }
    if (size != NULL)
        *size = obj->size;

    // remove if necessary
    if (remove == 1) {
        if (remove_obj(list, obj) == 0) {
            if (newmem == 1)
                ezos_free(data);
            data = NULL;
        }
    }

    ezlist_unlock(list);

    return data;
}

static ezlist_obj_t *get_obj(ezlist_t *list, int index) {
    // index adjustment
    if (index < 0)
        index = list->num + index;
    if (index >= list->num) {
        return NULL;
    }

    // detect faster scan direction
    int backward;
    ezlist_obj_t *obj;
    int listidx;
    if (index < list->num / 2) {
        backward = 0;
        obj = list->first;
        listidx = 0;
    } else {
        backward = 1;
        obj = list->last;
        listidx = list->num - 1;
    }

    // find object
    while (obj != NULL) {
        if (listidx == index)
            return obj;

        if (backward == 0) {
            obj = obj->next;
            listidx++;
        } else {
            obj = obj->prev;
            listidx--;
        }
    }

    return NULL;
}

static int remove_obj(ezlist_t *list, ezlist_obj_t *obj) {
    if (obj == NULL)
        return 0;

    // chain prev and next elements
    if (obj->prev == NULL)
        list->first = obj->next;
    else
        obj->prev->next = obj->next;
    if (obj->next == NULL)
        list->last = obj->prev;
    else
        obj->next->prev = obj->prev;

    // adjust counter
    list->datasum -= obj->size;
    list->num--;

    // release obj
    ezos_free(obj->data);
    ezos_free(obj);

    return 1;
}

#endif /* _DOXYGEN_SKIP */

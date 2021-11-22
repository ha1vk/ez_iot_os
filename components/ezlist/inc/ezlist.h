#ifndef EZLIST_H
#define EZLIST_H

#include <stddef.h>

#ifdef __cplusplus
"C"
{
#endif

    /* types */
    typedef struct ezlist_s ezlist_t;
    typedef struct ezlist_obj_s ezlist_obj_t;

    enum
    {
        ezlist_THREADSAFE = (0x01) /*!< make it thread-safe */
    };

    ezlist_t *ezlist(int options); /*!< ezlist constructor */
    size_t ezlist_setsize(ezlist_t * list, size_t max);

    int ezlist_addfirst(ezlist_t * list, const void *data, size_t size);
    int ezlist_addlast(ezlist_t * list, const void *data, size_t size);
    int ezlist_addat(ezlist_t * list, int index, const void *data, size_t size);

    void *ezlist_getfirst(ezlist_t * list, size_t * size, int newmem);
    void *ezlist_getlast(ezlist_t * list, size_t * size, int newmem);
    void *ezlist_getat(ezlist_t * list, int index, size_t *size, int newmem);

    void *ezlist_popfirst(ezlist_t * list, size_t * size);
    void *ezlist_poplast(ezlist_t * list, size_t * size);
    void *ezlist_popat(ezlist_t * list, int index, size_t *size);

    int ezlist_removefirst(ezlist_t * list);
    int ezlist_removelast(ezlist_t * list);
    int ezlist_removeat(ezlist_t * list, int index);

    int ezlist_getnext(ezlist_t * list, ezlist_obj_t * obj, int newmem);

    size_t ezlist_size(ezlist_t * list);
    size_t ezlist_datasize(ezlist_t * list);
    void ezlist_reverse(ezlist_t * list);
    void ezlist_clear(ezlist_t * list);

    void *ezlist_toarray(ezlist_t * list, size_t * size);
    char *ezlist_tostring(ezlist_t * list);

    void ezlist_lock(ezlist_t * list);
    void ezlist_unlock(ezlist_t * list);

    void ezlist_free(ezlist_t * list);

    /**
     * ezlist container object
     */
    struct ezlist_s
    {
        /* private variables - do not access directly */
        void *mutex;    /*!< initialized when ezlist_OPT_THREADSAFE is given */
        size_t num;     /*!< number of elements */
        size_t max;     /*!< maximum number of elements. 0 means no limit */
        size_t datasum; /*!< total sum of data size, does not include name size */

        ezlist_obj_t *first; /*!< first object pointer */
        ezlist_obj_t *last;  /*!< last object pointer */
    };

    /**
     * ezlist node data structure.
     */
    struct ezlist_obj_s
    {
        void *data;  /*!< data */
        size_t size; /*!< data size */

        ezlist_obj_t *prev; /*!< previous link */
        ezlist_obj_t *next; /*!< next link */
    };

#ifdef __cplusplus
}
#endif

#endif /* ezlist_H */

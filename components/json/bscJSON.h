/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef bscJSON__h
#define bscJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* bscJSON Types: */
#define bscJSON_False 0
#define bscJSON_True 1
#define bscJSON_NULL 2
#define bscJSON_Number 3
#define bscJSON_String 4
#define bscJSON_Array 5
#define bscJSON_Object 6
	
#define bscJSON_IsReference 256
#define bscJSON_StringIsConst 512

/* The bscJSON structure: */
typedef struct bscJSON {
	struct bscJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct bscJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==bscJSON_String */
	int valueint;				/* The item's number, if type==bscJSON_Number */
	double valuedouble;			/* The item's number, if type==bscJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} bscJSON;

typedef struct bscJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} bscJSON_Hooks;

/* Supply malloc, realloc and free functions to bscJSON */
extern void bscJSON_InitHooks(bscJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a bscJSON object you can interrogate. Call bscJSON_Delete when finished. */
extern bscJSON *bscJSON_Parse(const char *value);
/* Render a bscJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *bscJSON_Print(bscJSON *item);
/* Render a bscJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *bscJSON_PrintUnformatted(bscJSON *item);
/* Render a bscJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *bscJSON_PrintBuffered(bscJSON *item,int prebuffer,int fmt);
/* Delete a bscJSON entity and all subentities. */
extern void   bscJSON_Delete(bscJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  bscJSON_GetArraySize(bscJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern bscJSON *bscJSON_GetArrayItem(bscJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern bscJSON *bscJSON_GetObjectItem(bscJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when bscJSON_Parse() returns 0. 0 when bscJSON_Parse() succeeds. */
extern const char *bscJSON_GetErrorPtr(void);
	
/* These calls create a bscJSON item of the appropriate type. */
extern bscJSON *bscJSON_CreateNull(void);
extern bscJSON *bscJSON_CreateTrue(void);
extern bscJSON *bscJSON_CreateFalse(void);
extern bscJSON *bscJSON_CreateBool(int b);
extern bscJSON *bscJSON_CreateNumber(double num);
extern bscJSON *bscJSON_CreateString(const char *string);
extern bscJSON *bscJSON_CreateArray(void);
extern bscJSON *bscJSON_CreateObject(void);

/* These utilities create an Array of count items. */
extern bscJSON *bscJSON_CreateIntArray(const int *numbers,int count);
extern bscJSON *bscJSON_CreateFloatArray(const float *numbers,int count);
extern bscJSON *bscJSON_CreateDoubleArray(const double *numbers,int count);
extern bscJSON *bscJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void bscJSON_AddItemToArray(bscJSON *array, bscJSON *item);
extern void	bscJSON_AddItemToObject(bscJSON *object,const char *string,bscJSON *item);
extern void	bscJSON_AddItemToObjectCS(bscJSON *object,const char *string,bscJSON *item);	/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the bscJSON object */
/* Append reference to item to the specified array/object. Use this when you want to add an existing bscJSON to a new bscJSON, but don't want to corrupt your existing bscJSON. */
extern void bscJSON_AddItemReferenceToArray(bscJSON *array, bscJSON *item);
extern void	bscJSON_AddItemReferenceToObject(bscJSON *object,const char *string,bscJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern bscJSON *bscJSON_DetachItemFromArray(bscJSON *array,int which);
extern void   bscJSON_DeleteItemFromArray(bscJSON *array,int which);
extern bscJSON *bscJSON_DetachItemFromObject(bscJSON *object,const char *string);
extern void   bscJSON_DeleteItemFromObject(bscJSON *object,const char *string);
	
/* Update array items. */
extern void bscJSON_InsertItemInArray(bscJSON *array,int which,bscJSON *newitem);	/* Shifts pre-existing items to the right. */
extern void bscJSON_ReplaceItemInArray(bscJSON *array,int which,bscJSON *newitem);
extern void bscJSON_ReplaceItemInObject(bscJSON *object,const char *string,bscJSON *newitem);

/* Duplicate a bscJSON item */
extern bscJSON *bscJSON_Duplicate(bscJSON *item,int recurse);
/* Duplicate will create a new, identical bscJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
extern bscJSON *bscJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated);

extern void bscJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define bscJSON_AddNullToObject(object,name)		bscJSON_AddItemToObject(object, name, bscJSON_CreateNull())
#define bscJSON_AddTrueToObject(object,name)		bscJSON_AddItemToObject(object, name, bscJSON_CreateTrue())
#define bscJSON_AddFalseToObject(object,name)		bscJSON_AddItemToObject(object, name, bscJSON_CreateFalse())
#define bscJSON_AddBoolToObject(object,name,b)	bscJSON_AddItemToObject(object, name, bscJSON_CreateBool(b))
#define bscJSON_AddNumberToObject(object,name,n)	bscJSON_AddItemToObject(object, name, bscJSON_CreateNumber(n))
#define bscJSON_AddStringToObject(object,name,s)	bscJSON_AddItemToObject(object, name, bscJSON_CreateString(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define bscJSON_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))
#define bscJSON_SetNumberValue(object,val)		((object)?(object)->valueint=(object)->valuedouble=(val):(val))

#ifdef __cplusplus
}
#endif

#endif

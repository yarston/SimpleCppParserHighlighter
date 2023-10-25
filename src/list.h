#ifndef VECTOR
#define VECTOR

#include <stddef.h>
#include <stdio.h>

typedef struct {
  size_t size;
  size_t sizeMax;
  size_t sizeOfElement;
  void* data;
} ArrayList;

ArrayList* newArrayList(size_t initSize, size_t sizeOfElement);
ArrayList* mergeArrayList(ArrayList *list1, ArrayList *list2);
void* add2ArrayList(ArrayList* list);
void trimArrayList(ArrayList *vector);
void writeArrayList(ArrayList *vector, FILE *file);
ArrayList *readArrayList(size_t sizeOfElement, FILE *file);
void freeArrayList(ArrayList *list);

#define NEW_ARRAY_LIST(initSize, type) newArrayList(initSize, sizeof(type))
#define FOR(Type, val, vec) for(Type *val = (Type*) vec->data, *end = val + vec->size; val < end; val++)
#define FOR_PTR(Type, val, vec) for(Type **val = (Type**) vec->data, **end = val + vec->size; val < end; val++)
#define FOR_STEP(Type, step, val, vec) for(Type *val = (Type*) vec->data, *end = val + vec->size * step; val < end; val += step)
#define FOR_REV(Type, val, vec) for(Type *start = (Type*) vec->data, *val = start + vec->size - 1; val >= start; val--)
#define LAST(type, vec) ((type*) vec->data) + (vec->size - 1)
#define AT(type, x, vec) (((type*) vec->data) + x)
#define FILE2VEC(type, file) readArrayList(sizeof(type), file)
#define SORT_ARRAY_LIST(type, list, v1, v2, exp) { \
    int fun(const type *v1, const type *v2) { \
        return exp;\
    }\
    qsort(list->data, list->size, list->sizeOfElement, (int(*) (const void *, const void *)) fun);\
}\

#define CONVERT_VEC_TYPE(Type1, val1, vec1, Type2, val2, vec2) \
    ArrayList *vec2 = newArrayList(vec1->size, sizeof(Type2)); \
    vec2->size = vec2->sizeMax; \
    Type1 *val1 = (Type1*) vec1->data; \
    Type2 *val2 = (Type2*) vec2->data; \
    for(Type1 *e = (Type1*) vec1->data + vec1->size; val1 < e; val1++, val2++) \

#endif//VECTOR

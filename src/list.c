#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <bits/stdint-uintn.h>

ArrayList* newArrayList(size_t initSize, size_t sizeOfElement) {
    ArrayList *vector = (ArrayList*) malloc(sizeof(ArrayList));
    if(!vector) return 0;
    vector->sizeOfElement = sizeOfElement;
    vector->sizeMax = initSize;
    vector->size = 0;
    vector->data = malloc(initSize * sizeOfElement);
    if(!vector->data) return 0;
    return vector;
}

void* add2ArrayList(ArrayList *list) {
    list->size++;
    size_t oldCountMax = list->sizeMax;

    if (list->size > oldCountMax) {
        list->sizeMax = oldCountMax + oldCountMax / 2;
        void *p = realloc(list->data, list->sizeMax * list->sizeOfElement);
        if(p) list->data = (char*) p;
        else {
            list->sizeMax = oldCountMax;
            list->size = 0;
            return 0;
        }
    }
    return list->data + list->sizeOfElement * (list->size - 1);
}

ArrayList* mergeArrayList(ArrayList* list1, ArrayList* list2) {
    assert(list1->sizeOfElement == list2->sizeOfElement);
    if(list1->sizeOfElement != list2->sizeOfElement) return 0;
    ArrayList *result = newArrayList(list1->size + list2->size, list1->sizeOfElement);
    if(!result) return 0;
    result->size = result->sizeMax;
    memcpy(result->data, list1->data, list1->size * list1->sizeOfElement);
    memcpy(result->data + list1->size * list1->sizeOfElement, list2->data, list2->size * list2->sizeOfElement);
    return result;
}

void trimArrayList(ArrayList *vector) {
      vector->data = realloc(vector->data, vector->size * vector->sizeOfElement);
}

void writeArrayList(ArrayList *vector, FILE *file) {
    uint32_t size = vector->size;
    fwrite(&size, sizeof(uint32_t), 1, file);
    fwrite(vector->data, vector->size * vector->sizeOfElement, 1, file);
};

ArrayList *readArrayList(size_t sizeOfElement, FILE *file) {
    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, file);
    ArrayList *result = newArrayList(size, sizeOfElement);
    fread(result->data, sizeOfElement * size, 1, file);
    result->size = result->sizeMax;
    return result;
}

void freeArrayList(ArrayList *list) {
    free(list->data);
    free(list);
}

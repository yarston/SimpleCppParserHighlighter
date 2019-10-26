#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "parser.h"
#include "hashmap.h"

char* readFile(const char *fileName, char isString, unsigned int *fileSize) {
    FILE *file = fopen(fileName, "rb");
    unsigned long size;
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char*) malloc(isString ? size + 1 : size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    fread(buffer, size, 1, file);
    if (isString) buffer[size] = 0;
    fclose(file);
    printf("success read %u bytes\n", (unsigned) size);
    if (fileSize) *fileSize = size;
    return buffer;
}

void processTokens(char *start, char *end) {
    putchar('\'');
    for (char *c = start; c < end; c++) putchar(*c);
    putchar('\'');
    putchar('\n');
}

void main(int argc, char **argv) {
    unsigned fileSize;
    char *text = readFile("test_nums.c", true, &fileSize);
    //parseFile(text, &parseExpression);
    //parseFile2(text);
    lexify(text, &processTokens);
    free(text);
    return 0;
}
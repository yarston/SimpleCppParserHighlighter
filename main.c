#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "parser.h"

//Например Pointer<float> nubmers;
//или Array<Array<float>> numbers;
typedef struct Val Val;
typedef struct Type Type;
typedef struct State State;
typedef struct Exp Exp;

//программа представляет из себя список выражений
//выражение начинается с новой строки и заканчивется ;
//каждый класс, который может быть инициализирован через initString, сам парсит эту строку
//оператор может быть составным, напимер, <= или ***. Тогда старший бит байта оператора = 1.
struct Exp {
    Exp *next;
    Exp *inner;
    Val* values;
    char* operators;
    char* initString; // например, 2.0f или "abc" или [3,5]
    uint32_t flags;
};

//значению можно присвоить что-то только 1 раз в своей зоне видимости
//изменение переменной создаёт новое значение
struct Val {
    Exp *next;
    Exp *inner;
    char *name; //хранить не обязательно - может быть обфусцированно
    Type *type;
    uint32_t flags;
};

struct Type {
    Type *inner; //вложенные типы, например Map<List<int>>
    char *name;
    Val* values; //для функций и структур
    size_t chainSize;
    size_t valuesSize;
};

typedef struct State {
    char *varName;
    char *varType;
    char operator;
    unsigned scopeLevel;
    ArrayList stackVars;
    ArrayList heapVars;
    ArrayList simpleTypes; //например float или Array
    ArrayList complexTypes; //например Array<float>
} State;

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

void printExpression(char *s, char *start, char *end) {
    start[end - start] = 0;
    char data[2 + end - start];
    data[end - start] = 0;
    for (char *src = start, *dst = data; src < end; *dst++ = *src++);
    printf(s, data);
}

void parseExpression(char *start, char *end) {
    //printExpression(start, end);
    putchar('\n');
    putchar('[');
    Type *type = NULL;
    char *tokenStart = 0, *tokenEnd = 0;
    char *qouteStart = 0, *qouteEnd = 0;
    for (char *pc = start, c = *pc; pc < end; c = *++pc) {
        putchar(c);
        if (qouteStart) {
            if (c == '"') {
                qouteEnd = pc;
                printExpression("\e[36m[string: %s]\e[39m", qouteStart, qouteEnd);
                qouteStart = 0;
                qouteEnd = 0;
            }
        } else {
            switch (c) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                    if (!tokenStart)  tokenStart = pc;
                    break;
                case ' ':
                    if (tokenStart) {
                        tokenEnd = pc;
                        printExpression("\e[36m[token: %s]\e[39m", tokenStart, tokenEnd);
                        tokenStart = 0;
                        tokenEnd = 0;
                    }
                    break;
                case '"':
                    qouteStart = pc + 1;
                    break;
                case '(':
                    tokenStart = 0;
                    break;

            }
        }
    }
    putchar(']');
}

void parseFile(char *text, void (*expressionParser) (char *start, char *end)) {
    char *start = text, *end = 0;
    bool started = false;
    for (char *pc = text, c = *text; c; c = *pc++) {
        switch (c) {
            case ';':
            case '\n':
                if (started) {
                    end = pc - 1;
                    if (end > start) expressionParser(start, end);
                    start = pc + 1;
                    started = false;
                }
                break;
            case ' ':
            case '  ':
            case '(':
            case ')':
            case '{':
            case '}':
                break;
            default:
                if (!started) {
                    start = pc - 1;
                    started = true;
                }
        }
    }
}

int parseFile2(char *text) {
    //unsigned fileSize;
   // char *text = readFile("main.b", true, &fileSize);
    //Token t = {};
    putchar('\n');
   // putchar('[');
    Type *type = NULL;
    char *tokenStart = 0, *tokenEnd = 0;
    char *qouteStart = 0, *qouteEnd = 0;
    
    for (char *pc = text, c = *text; c; c = *pc++) {
        putchar(c);
        if (qouteStart) {
            if (c == '"') {
                qouteEnd = pc;
                printExpression("\e[36m[string: %s]\e[39m", qouteStart, qouteEnd);
                qouteStart = 0;
                qouteEnd = 0;
            }
        } else {
            switch (c) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                    if (!tokenStart) tokenStart = pc - 1;
                    break;
                case ' ':
                case '(' ... '/':
                case ':' ... '?':
                case '!':
                case '%':
                case '&':
                case '[':
                case ']':
                case '^':
                case '|':
                case '~':
                    if (tokenStart) {
                        tokenEnd = pc;
                        // if(tokenEnd > tokenStart + 2)
                        printExpression("\e[36m[token: %s]\e[39m", tokenStart, tokenEnd - 1);
                        //putchar(*tokenStart);
                        tokenStart = 0;
                        tokenEnd = 0;
                    }
                    break;
                case '"':
                    qouteStart = pc + 1;
                    break;
                /*case '(':
                    tokenStart = 0;
                    break;*/

            }
        }
        /*if (isQuoteOpened) {
            if (c == '\"' || c == '\'') {
                isQuoteOpened = false;
                isStarted = false;
                start = start;
                end = end;
                goto emitToken;
            } else {
                if (isStarted) end++;
                else {
                    isStarted = true;
                    start = pos - 2;
                    end = pos - 1;
                }
                continue;
            }
        }
        switch (c) {
            case '{':
                lvl++;
                //currLvlList = (lvl < listScoupe->size) ? AT(ArrayList, lvl, listScoupe) : initArrayList(add2ArrayList(listScoupe), 4, sizeof (Token));
                continue;
            case '}':
                lvl--;
                //currLvlList->size = 0;
                //currLvlList = AT(ArrayList, lvl, listScoupe);
                goto emitScoupe;\
            case '\"':
            case '\'':
                isQuoteOpened = true;
                isStarted = true;
                start = pos - 2;
                end = pos;
                continue;
            case '(' ... '/':
            case ':' ... '?':
            case '!':
            case '%':
            case '&':
            case '[':
            case ']':
            case '^':
            case '|':
            case '~':
                if (isStarted) {
                    isStarted = false;
                    start = start;
                    end = end;
                    pos--;
                    pc--;
                } else {
                    start = pos - 2;
                    end = pos - 1;
                }
                goto emitToken;
            case '0' ... '9':
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_':
                if (isStarted) end++;
                else {
                    isStarted = true;
                    start = pos - 2;
                    end = pos - 1;
                }
                continue;
            default:
                if (isStarted) {
                    isStarted = false;
                    goto emitToken;
                }
                continue;
        }
emitToken:;
        printf("%d:%d:", start, end);
        for (int i = 0; i < lvl; i++) printf("    ");
        for (char *c1 = text + start, *e = text + end; c1 < e; c1++) putchar(*c1);
        putchar('\n');
        continue;
emitScoupe:;
        //work
        printf("emit scoupe\n");
        //currLvlList->size = 0;
        //currLvlList = AT(ArrayList, lvl, listScoupe);*/
    }
    
    //FOR(ArrayList, list, listScoupe) free(list->data);
    //freeArrayList(listScoupe);
    //free(text);
}

void main(int argc, char **argv) {
    unsigned fileSize;
    char *text = readFile("test_nums.c", true, &fileSize);
    //parseFile(text, &parseExpression);
    //parseFile2(text);
    lexify(text);
    free(text);
    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "parser.h"
//#include "hashmap.h" //https://github.com/petewarden/c_hashmap
#include "map.h" //https://github.com/rxi/map
#include "stb_truetype.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "render.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

unsigned char* bitmap;
int width = 1024; // bitmap width
int height = 1024; // bitmap height
int lineHeight = 22; // line height
int scrollX = 0;
int scrollY = 0;
map_int_t m;
stbtt_fontinfo info;

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

ArrayList* getLines(char *textStart, char *textEnd) {
    ArrayList *lines = newArrayList(16, sizeof (LineText));
    for (char *pc = textStart, *prev = textStart; pc < textEnd; pc++) {
        if (*pc == '\n') {
            size_t size = pc - prev;
            if(size > 0) {
                LineText *line = add2ArrayList(lines);
                line->text = malloc(size + 1);
                line->length = size;
                memcpy(line->text, prev, size);
                line->text[size] = 0;
                printf("line(%d)=%s\n", size, line->text);
            }
            //*((char**) add2ArrayList(lines)) = pc;
            prev = pc + 1;
        }
    }
    return lines;
}

void processTokens(char *start, char *end, LexType token) {
    putchar('\'');
    for (char *c = start; c < end; c++) putchar(*c);
    putchar('\'');
    putchar('\n');
}


int main(int argc, char **argv) {
    
    long size;
    unsigned char* fontBuffer;

    FILE* fontFile = fopen("./DroidSansMono.ttf", "rb");
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile); /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */

    fontBuffer = malloc(size);

    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    /* prepare font */
    //stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, fontBuffer, 0)) {
        printf("failed\n");
    }

    bitmap = malloc(width * height * 4 * 2);
    map_init(&m);
    //map_init(&fonts);
    map_set(&m, "int", 1);
    map_set(&m, "float", 2);
    map_set(&m, "double", 3);
    map_set(&m, "void", 4);

    unsigned fileSize;
    char *text = readFile("main.c", true, &fileSize);
    ArrayList* lines = getLines(text, text + fileSize);
    
    WindowText wt = {(uint32_t*) bitmap, 0, 0, width, height, text, lines};
    drawWindow(&wt);

    //parseFile(text, &parseExpression);
    //parseFile2(text);
    lexify(text, text + fileSize, &processTokens);
    free(text);
    freeArrayList(lines);
    map_deinit(&m);
    free(fontBuffer);
    // test_font();
    return 0;
}
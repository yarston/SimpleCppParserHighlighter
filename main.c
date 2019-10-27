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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

map_int_t m;

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

int drawSymbol(stbtt_fontinfo *info, char *bitmap, int b_w, int x, int word, int next, int ascent, float scale, uint32_t r, uint32_t g, uint32_t b) {
    int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(info, word, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        int y = ascent + c_y1;
        uint32_t *ibmp = ((uint32_t*) bitmap) + (x + y * b_w);
        int cw = c_x2 - c_x1, ch = c_y2 - c_y1;
        unsigned char cbmp[cw * ch];
        stbtt_MakeCodepointBitmap(info, cbmp, cw, ch, cw, scale, scale, word);
        for (int y = 0; y < ch; y++) {
            for (int x = 0; x < cw; x++) {
                uint32_t value = cbmp[x + y * cw];
                ibmp[x + y * b_w] = 0xFF000000 | (((value * r) >> 8) & 0xFF) | ((value * g) & 0xFF00) | ( ((value * b) << 8) & 0xFF0000);
            }
        }
        int ax;
        stbtt_GetCodepointHMetrics(info, word, &ax, 0);
        int kern = stbtt_GetCodepointKernAdvance(info, word, next);
        return (ax + kern) * scale;
}

void drawLine(stbtt_fontinfo *info, char *bitmap, int b_w, char *start, char *end, int ascent, float scale) {
    int x = 0;
    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, bitmap, b_w, x, (int) pc[0], (int) pc[1], ascent, scale, 0x81, 0xff, 0xd5);
    }
}

void drawHighlightedLine(stbtt_fontinfo *info, char *bitmap, int b_w, char *start, char *end, int ascent, float scale) {
    int x = 0;

    void processLine(char *lexStart, char *lexEnd, LexType lexType) {
        int argb = -1;

        for (char *pc = start; pc < lexStart; pc++) {
            x += drawSymbol(info, bitmap, b_w, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
        }
        start = lexEnd;

        switch (lexType) {
            case NAME:
                
                //int *val = map_get(&m, "testkey");
                
                argb = 0xFF00ffe2;
                break;
            case NUM:
            case DOUBLE:
            case FLOAT:
            case HEX:
                argb = 0xFFaa00ff;
                break;
            case STRING: 
                argb = 0xFFff7b2e;
                //lexStart--;
                //lexEnd++;
                break;
            case COMMENT: 
                argb = 0xFFA0A0A0;
                break;
            case OP: argb = 0xFFf4ff1a;
                break;
        }
        int r = (argb >> 16) & 0xFF;
        int g = (argb >> 8) & 0xFF;
        int b = argb & 0xFF;
        for (char *pc = lexStart; pc < lexEnd; pc++) {
            x += drawSymbol(info, bitmap, b_w, x, (int) pc[0], (int) pc[1], ascent, scale, r, g, b);
        }
    }

    lexify(start, end, &processLine);

    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, bitmap, b_w, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
    }
}

ArrayList* getLines(char *textStart, char *textEnd) {
    ArrayList *lines = newArrayList(16, sizeof (void*));
    for (char *pc = textStart; pc < textEnd; pc++) {
        if (*pc == '\n') {
            *((char**) add2ArrayList(lines)) = pc;
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

void drawLines(ArrayList *lines, char *start) {

    /* load font file */
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
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, fontBuffer, 0)) {
        printf("failed\n");
    }

    int b_w = 1024; /* bitmap width */
    int b_h = 1024; /* bitmap height */
    int l_h = 32; /* line height */

    /* create a bitmap for the phrase */
    unsigned char* bitmap = malloc(b_w * b_h * 4);
    for (uint32_t *p = bitmap, *e = p + b_w * b_h; p < e; p++) *p = 0xFF000000;
    /* calculate font scaling */
    float scale = stbtt_ScaleForPixelHeight(&info, l_h);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent *= scale;
    descent *= scale;

    char *lineStart = start;

    FOR(char*, lineEnd, lines) {
        drawHighlightedLine(&info, bitmap, b_w, lineStart, *lineEnd, ascent, scale);
        processTokens(lineStart, *lineEnd, 0);
        lineStart = *lineEnd + 1;
        ascent += l_h;
    }

    stbi_write_png("out.png", b_w, b_h, 4, bitmap, b_w * 4);

    free(fontBuffer);
    free(bitmap);
}

void main(int argc, char **argv) {
    

    map_init(&m);
    map_set(&m, "int", 1);
    map_set(&m, "float", 2);
    map_set(&m, "double", 3);
    map_set(&m, "void", 4);
    
    unsigned fileSize;
    char *text = readFile("test_nums.c", true, &fileSize);
    ArrayList* lines = getLines(text, text + fileSize);
    drawLines(lines, text);
    // processTokens(text, text + fileSize, 0);



    //parseFile(text, &parseExpression);
    //parseFile2(text);
    lexify(text, text + fileSize, &processTokens);
    free(text);
    freeArrayList(lines);
    map_deinit(&m);
    // test_font();
    return 0;
}
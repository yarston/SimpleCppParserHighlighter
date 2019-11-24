#ifndef RENDER
#define RENDER

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "stb_truetype.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "parser.h"
#include "map.h" //https://github.com/rxi/map

typedef struct WindowText {
    uint32_t *pixmap;
    int x, y, w, h;
    char *text;
    ArrayList *lines;
} WindowText;

typedef struct LineText {
    char *text;
    size_t length;
    int x, y, w, h;
    bool isComment;
} LineText;

int drawSymbol(stbtt_fontinfo *info, uint32_t *pixmap, int width, int x, int word, int next, int ascent, float scale, uint32_t r, uint32_t g, uint32_t b);
void drawLine(stbtt_fontinfo *info, uint32_t *bitmap, int width, int b_w, char *start, char *end, int ascent, float scale);
void drawHighlightedLine(stbtt_fontinfo *info, uint32_t *pixmap, int width, char *start, char *end, int ascent, float scale);
void drawLines(WindowText *wt);
void drawWindow(WindowText *windowText);

#endif//RENDER
#include "render.h"
#include <X11/Xlib.h>

extern int height; // bitmap height
extern int lineHeight; // line height
extern int scrollX;
extern int scrollY;
extern map_int_t m;
extern stbtt_fontinfo info;


int drawSymbol(stbtt_fontinfo *info, uint32_t *pixmap, int width, int x, int word, int next, int ascent, float scale, uint32_t r, uint32_t g, uint32_t b) {
    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(info, word, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
    int y = ascent + c_y1;
    uint32_t *ibmp = ((uint32_t*) pixmap) + (x + y * width);
    int cw = c_x2 - c_x1, ch = c_y2 - c_y1;
    unsigned char cbmp[cw * ch];
    stbtt_MakeCodepointBitmap(info, cbmp, cw, ch, cw, scale, scale, word);
    for (int y = 0; y < ch; y++) {
        for (int x = 0; x < cw; x++) {
            uint32_t value = cbmp[x + y * cw];
            ibmp[x + y * width] = 0xFF000000 | (((value * r) >> 8) & 0xFF) | ((value * g) & 0xFF00) | (((value * b) << 8) & 0xFF0000);
        }
    }
    int ax;
    stbtt_GetCodepointHMetrics(info, word, &ax, 0);
    int kern = stbtt_GetCodepointKernAdvance(info, word, next);
    return (ax + kern) * scale + 1;
}

void drawLine(stbtt_fontinfo *info, uint32_t *bitmap, int width, int b_w, char *start, char *end, int ascent, float scale) {
    int x = 0;
    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, bitmap, x, width, (int) pc[0], (int) pc[1], ascent, scale, 0x81, 0xff, 0xd5);
    }
}

void drawHighlightedLine(stbtt_fontinfo *info, uint32_t *pixmap, int width, char *start, char *end, int ascent, float scale) {
    int x = 0;

    void processLine(char *lexStart, char *lexEnd, LexType lexType) {
        int argb = -1;

        for (char *pc = start; pc < lexStart; pc++) {
            x += drawSymbol(info, pixmap, width, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
        }
        start = lexEnd;

        switch (lexType) {
            case NAME: {
                int *val = map_get2(&m, lexStart, lexEnd - lexStart);
                if (val) argb = *val;
                else argb = 0xFF00ffe2;
            }
            break;
            case NUM:
            case DOUBLE:
            case FLOAT:
            case HEX:
                argb = 0xFFFF55FF;
                break;
            case STRING:
            case CHARS:
                argb = 0xFFff7b2e;
                //lexStart--;
                //lexEnd++;
                break;
            case COMMENT:
            case COMMENT_MULTILINE:
                argb = 0xFFA0A0A0;
                break;
            case OP: argb = 0xFFf4ff1a;
                break;
        }
        int r = (argb >> 16) & 0xFF;
        int g = (argb >> 8) & 0xFF;
        int b = argb & 0xFF;
        for (char *pc = lexStart; pc < lexEnd; pc++) {
            x += drawSymbol(info, pixmap, width, x, (int) pc[0], (int) pc[1], ascent, scale, b, g, r);
        }
    }

    lexify(start, end + 1, &processLine);

    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, pixmap, width, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
    }
}



void drawLines(WindowText *wt) {

    for (uint32_t *p = wt->pixmap, *e = p + wt->w * wt->h; p < e; p++) *p = 0xFF000033;
    /* calculate font scaling */
    float scale = stbtt_ScaleForPixelHeight(&info, lineHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent *= scale;
    ascent += + wt->y;
    //descent *= scale;
    int topHeight = wt->y;

    FOR(LineText, l, wt->lines) {
        if (topHeight >= 0 && topHeight + lineHeight < wt->h)
            drawHighlightedLine(&info, wt->pixmap, wt->w, l->text, l->text + l->length, ascent, scale);
        // processTokens(l->text, *lineEnd, 0);
        //printf("line=%s\n", l->text);
        //text = *lineEnd + 1;

        ascent += lineHeight;
        topHeight += lineHeight;
    }
    //stbi_write_png("out.png", b_w, b_h, 4, bitmap, b_w * 4);
    //free(fontBuffer);
}

void drawWindow(WindowText *wt) {

    Display *d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return;
    }

    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, wt->w, wt->h, 1,  BlackPixel(d, s), WhitePixel(d, s));
    //if(wd->name) XStoreName(d, w, wd->name);
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
    GC gc = XCreateGC(d, w, 0, 0);
    XImage *ximage = XCreateImage(d, DefaultVisual(d, s), DefaultDepth(d, s), ZPixmap, 0, (char*) wt->pixmap, wt->w, wt->h, 32, wt->w * 4);

    while (1) {
        XEvent e;
        XNextEvent(d, &e);
        
        fprintf(stderr, "keycode: %d\n", e.xkey.keycode);
        // 116 d 11 u
        if (e.type == KeyPress) switch(e.xkey.keycode) {
            case 9: goto close_window;
            case 111: wt->y -= 10; break; //up
            case 116: wt->y += 10; break; //down
        }
        
        drawLines(wt);
        XPutImage(d, w, gc, ximage, 0, 0, 0, 0, wt->w, wt->h);
    }
    close_window:;
   // free(wd);
    XFree(ximage);
    XFreeGC(d, gc);
    XCloseDisplay(d);
}

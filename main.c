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

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <stdio.h>
#include <stdlib.h>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

unsigned char* bitmap;
int width = 1024; // bitmap width
int height = 1024; // bitmap height
int lineHeight = 32; // line height
int scrollX = 0;
int scrollY = 0;
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

int drawSymbol(stbtt_fontinfo *info, int x, int word, int next, int ascent, float scale, uint32_t r, uint32_t g, uint32_t b) {
    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(info, word, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
    int y = ascent + c_y1;
    uint32_t *ibmp = ((uint32_t*) bitmap) + (x + y * width);
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

void drawLine(stbtt_fontinfo *info, char *bitmap, int b_w, char *start, char *end, int ascent, float scale) {
    int x = 0;
    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, x, (int) pc[0], (int) pc[1], ascent, scale, 0x81, 0xff, 0xd5);
    }
}

void drawHighlightedLine(stbtt_fontinfo *info, char *start, char *end, int ascent, float scale) {
    int x = 0;

    void processLine(char *lexStart, char *lexEnd, LexType lexType) {
        int argb = -1;

        for (char *pc = start; pc < lexStart; pc++) {
            x += drawSymbol(info, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
        }
        start = lexEnd;

        switch (lexType) {
            case NAME: {
                int *val = map_get2(&m, lexStart, lexEnd - lexStart);
                if (val) argb = 0xFF980866;
                else argb = 0xFF00ffe2;
            }
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
            x += drawSymbol(info, x, (int) pc[0], (int) pc[1], ascent, scale, r, g, b);
        }
    }

    lexify(start, end + 1, &processLine);

    for (char *pc = start; pc < end; pc++) {
        x += drawSymbol(info, x, (int) pc[0], (int) pc[1], ascent, scale, 0xFF, 0xFF, 0xFF);
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

#include <X11/Xlib.h>

void showX11() {

    Display *d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 0;
    }

    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, width, height, 1,  BlackPixel(d, s), WhitePixel(d, s));
    //if(wd->name) XStoreName(d, w, wd->name);
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
    GC gc = XCreateGC(d, w, 0, 0);
    XImage *ximage = XCreateImage (d, DefaultVisual(d, s), DefaultDepth(d, s), ZPixmap, 0, (char*) bitmap, width, height, 32, width * 4);

    while (1) {
        XEvent e;
        XNextEvent(d, &e);
        
        fprintf(stderr, "keycode: %d\n", e.xkey.keycode);
        // 116 d 11 u
        if (e.type == KeyPress && e.xkey.keycode == 9) break;
        if (e.type == KeyPress && e.xkey.keycode == 111) {
           for (uint32_t *p = bitmap, *e = p + width * height; p < e; p++) *p = 0xFFFF0000;
        }
        if (e.type == KeyPress && e.xkey.keycode == 116) {
           for (uint32_t *p = bitmap, *e = p + width * height; p < e; p++) *p = 0xFF0000FF;
        }
        XPutImage(d, w, gc, ximage, 0, 0, 0, 0, width, height);
    }
   // free(wd);
    XFree(ximage);
    XFreeGC(d, gc);
    XCloseDisplay(d);
    return 0;
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

    /* create a bitmap for the phrase */
    // unsigned char* bitmap = malloc(width * height * 4);
    for (uint32_t *p = bitmap, *e = p + width * height; p < e; p++) *p = 0xFF000000;
    /* calculate font scaling */
    float scale = stbtt_ScaleForPixelHeight(&info, lineHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent *= scale;
    //descent *= scale;

    char *lineStart = start;

    FOR(char*, lineEnd, lines) {
        drawHighlightedLine(&info, lineStart, *lineEnd, ascent, scale);
        processTokens(lineStart, *lineEnd, 0);
        lineStart = *lineEnd + 1;
        ascent += lineHeight;
    }

   // showWindowXcb();
    showX11();

    //stbi_write_png("out.png", b_w, b_h, 4, bitmap, b_w * 4);

    free(fontBuffer);
    free(bitmap);
}

xcb_grab_keyboard_reply_t * grabc(xcb_connection_t *conn, xcb_screen_t *screen) {
    xcb_grab_keyboard_cookie_t cookie;
    xcb_grab_keyboard_reply_t *reply;

    cookie = xcb_grab_keyboard(
            conn,
            true, /* report events */
            screen->root, /* grab the root window */
            XCB_CURRENT_TIME,
            XCB_GRAB_MODE_ASYNC, /* process events as normal, do not require sync */
            XCB_GRAB_MODE_ASYNC
            );

    if ((reply = xcb_grab_keyboard_reply(conn, cookie, NULL))) {
        if (reply->status == XCB_GRAB_STATUS_SUCCESS)
            printf("successfully grabbed the keyboard\n");

       // free(reply);
    }
    return reply;
}



int showWindowXcb() {
    // int width = 1024, height = 1024;
    xcb_generic_event_t *e;
    int done = 0;
    /* открыть соединение с сервером */
    xcb_connection_t *c = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(c)) {
        printf("Cannot open display\n");
        exit(1);
    }
    /* get the first screen */
    xcb_screen_t *s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

    /* create black graphics context */
    xcb_gcontext_t g = xcb_generate_id(c);
    xcb_window_t w = s->root;
    uint32_t values_gc[] = {s->black_pixel, 0};
    xcb_create_gc(c, g, w, XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES, values_gc);

    /* создать окно */
    w = xcb_generate_id(c);
    uint32_t values_win[] = {s->white_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};
    xcb_create_window(c, s->root_depth, w, s->root, 0, 10, width, height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, values_win);

    xcb_pixmap_t pixmap = xcb_generate_id(c); // = xcb_create_pixmap_from_bitmap_data(c, w, neko_bits, neko_width, neko_height, s->root_depth, s->black_pixel, s->white_pixel, NULL);
    xcb_create_pixmap(c, s->root_depth, pixmap, w, width, height);
    //uint8_t *data = malloc(width * height * 4);
    //for(uint32_t *p = data, *e = p + width * height; p < e; p++) *p = 0xFF00FF00;
    //xcb_image_t *img = xcb_image_create_from_bitmap_data(data, width, height);
    xcb_image_t *image = xcb_image_create_native(c, width, height * 2, XCB_IMAGE_FORMAT_Z_PIXMAP, s->root_depth, bitmap, width * height * 4 * 2, bitmap);
    xcb_image_put(c, pixmap, g, image, 0, 0, 0);
    xcb_map_window(c, w); // отобразить окно
    xcb_flush(c);

    xcb_grab_keyboard_reply_t *r = grabc(c, s);
    
    /* event loop */
    while (!done && (e = xcb_wait_for_event(c))) {
        switch (e->response_type & ~0x80) {
            case XCB_EXPOSE: /* отрисовать или перерисовать окно */
                xcb_copy_area(c, pixmap, w, g, 0, 0, 0, 0, width, height);
                //  xcb_poly_fill_rectangle(c, w, g, 1, &r);
                xcb_flush(c);
                break;
            case XCB_KEY_PRESS: /* выйти по нажатию клавиши */
                //for(uint32_t *p = bitmap, *e = p + width * height; p < e; p++) *p = 0xFFFF0000;
                //xcb_image_put(c, pixmap, g, image, 0, 0, 0);
                scrollY += 10;
                xcb_copy_area(c, pixmap, w, g, 0, scrollY, 0, 0, 1024, 1024);
                xcb_flush(c);
                
              //  xcb_keycode_t code = e->detail;
                 printf("key=%d\n", r->sequence);
                //done = 1;
                break;
        }
        free(e);
    }
    /* закрыть соединение с сервером */
    xcb_disconnect(c);

    return 0;
}

void main(int argc, char **argv) {

    bitmap = malloc(width * height * 4 * 2);
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
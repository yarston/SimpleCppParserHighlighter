#ifndef PARSER_H
#define PARSER_H

typedef enum LexType {
    NONE, NAME, NUM, OP, STRING, COMMENT, COMMENT_MULTILINE, ERROR, DOUBLE, FLOAT, HEX, DOT, CHARS
} LexType;

void lexify(char *text, char *textEnd, void (*callback) (char *start, char *end, LexType token));

#endif 

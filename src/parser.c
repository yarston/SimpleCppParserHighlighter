#include <stdio.h>
#include "parser.h" 

//TODO octal and binary numbers
void lexify(char *text, char *textEnd, void (*callback) (char *start, char *end, LexType token)) {
    char *start = 0;
    char *end = 0;
    LexType type = NONE;
    LexType prevType = NONE;
    for (char *pc = text, c = *text; pc < textEnd; pc++) {
        c = *pc;
        switch (type) {
            case NONE:
                switch (c) {
                    case '\'':
                        type = CHARS;
                        start = pc;
                        break;
                    case '"':
                        type = STRING;
                        start = pc;
                        break;
                    case '0' ... '9':
                        type = NUM;
                        start = pc;
                        break;
                    case 'A' ... 'Z':
                    case 'a' ... 'z':
                    case '_':
                        type = NAME;
                        start = pc;
                        break;
                    case '=':
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case ',':
                    case '?':
                    case '!':
                    case '&':
                    case '|':
                    case '~':
                    case '^':
                    case '<':
                    case '>':
                    case ':':
                    case ';':
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case '(':
                    case ')':
                        type = OP;
                        start = pc;
                        break;
                    case '.':
                        type = DOT;
                        start = pc;
                        break;
                }
                break;
            case NAME:
                switch (c) {
                    case 'A' ... 'Z':
                    case 'a' ... 'z':
                    case '0' ... '9':
                    case '_':
                        break;
                    default:
                        type = NONE;
                        end = pc;
                        pc--;
                        break;
                }
                break;
            case NUM:
                switch (c) {
                    case '0' ... '9':
                        break;
                    case '.':
                        type = DOUBLE;
                        break;
                    case 'f':
                        type = FLOAT;
                        break;
                    case 'x':
                        if (*start == '0' && pc - start == 1) {
                            type = HEX;
                            break;
                        }
                    default:
                        type = NONE;
                        end = pc;
                        pc--;
                        break;
                }
                break;
            case DOUBLE:
                switch (c) {
                    case '0' ... '9':
                        break;
                    case 'f':
                        type = FLOAT;
                        break;
                    default:
                        type = NONE;
                        end = pc;
                        pc--;
                        break;
                }
                break;

            case FLOAT:
                type = NONE;
                end = pc;
                pc--;
                break;

            case HEX:
                switch (c) {
                    case '0' ... '9':
                    case 'A' ... 'F':
                    case 'a' ... 'f':
                        break;
                    default:
                        type = NONE;
                        end = pc;
                        pc--;
                        break;
                }
                break;

            case DOT:
                switch (c) {
                    case '0' ... '9':
                        type = NUM;
                        break;
                    default:
                        type = OP;
                        break;
                }
                break;

            case OP:
                switch (c) {
                    case '*':
                        if (pc[-1] == '/') {
                            type = COMMENT_MULTILINE;
                            start = pc + 1;
                            break;
                        }
                    case '/':
                        if (pc[-1] == '/') {
                            type = COMMENT;
                            start = pc + 1;
                            break;
                        }
                    case '=':
                    case '+':
                    case '-':
                    case ',':
                    case '?':
                    case '!':
                    case '&':
                    case '|':
                    case '~':
                    case '^':
                    case '<':
                    case '>':
                    case ':':
                    case ';':
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case '(':
                    case ')':
                    case '.':
                        break;
                    default:
                        type = NONE;
                        end = pc;
                        pc--;
                        break;
                }
                break;
            case STRING:
                if (c == '"' && pc[-1] != 92) {
                    type = NONE;
                    end = pc + 1;
                }
                break;
            case CHARS:
                if (c == '\'' && pc[-1] != 92) {
                    type = NONE;
                    end = pc + 1;
                }
                break;
            case COMMENT:
                if (c == '\n') {
                    type = NONE;
                    end = pc;
                }
            case COMMENT_MULTILINE:
                if (c == '/' && pc[-1] == '*') {
                    type = NONE;
                    end = pc - 1;
                }
                break;
        }
        if (prevType != type && end - start > 0) callback(start, end, prevType);
        prevType = type;
    }
}
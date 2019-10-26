#ifndef PARSER_H
#define PARSER_H

void lexify(char *text, void (*callback) (char *start, char *end));

#endif 

#ifndef PARSE_H
#define PARSE_H

extern bool isDelimiter(char c);
extern bool isEndOfLine(char c);
extern int parse(int &i, const char *line, char *&returnstring);

#endif

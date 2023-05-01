#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stdbool.h>


bool parse_log_line(char *str, char **url, char **refer, unsigned long *trafic);

#endif // LOG_PARSER_H

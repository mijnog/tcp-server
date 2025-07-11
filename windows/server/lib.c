#include <ctype.h>
#include <string.h>
#include "lib.h"

char *strtolower(const char *src, char *dest) {
    char *original_dest = dest;
    while (*src != '\0') {
        *dest = tolower(*src);
        src++;
        dest++;
    }
    *dest = '\0';
    return original_dest;
}

char *trim_white_space(char *str) {
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}
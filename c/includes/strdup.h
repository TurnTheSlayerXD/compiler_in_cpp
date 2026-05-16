#ifndef MY_STRDUP_H

#define MY_STRDUP_H

#include "macros.h"


char *strdup_n(const char *str, int n) {
    if (str == NULL) {
        assert(false && "str == NULL at char *strdup_n(const char *str, const char *n)");
    }
    if (n < 0) {
        assert(false && "n < 0 at char *strdup_n(const char *str, const char *n)");
    }
    int len = MIN(strlen(str), n);
    char *p = malloc(len + 1);
    memcpy(p, str, len);
    p[len] = '\0';
    return p;
}


#endif

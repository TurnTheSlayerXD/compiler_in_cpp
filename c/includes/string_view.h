#ifndef MY_STRING_VIEW_H 
#define MY_STRING_VIEW_H

#include <math.h>
#include "macros.h"


typedef struct String_View {
    const char *ptr;
    int len;
} String_View;


#define FMT_SV "%.*s"
#define ARGS_SV(str) (str).len,(str).ptr


#define SV_FROM_CSTR(arg_str) {.ptr = (arg_str), .len = COUNT_OF(arg_str) - 1 }

String_View left_substr_sv(String_View str, int p) {
    if (p > str.len) {
        assert(false && "String_View  from_left(String_View str, int p)");
    }
    return (String_View) { .ptr = str.ptr + p, .len = str.len - p};
}



String_View substr_sv(String_View str, int l, int r) {
    if (r > str.len || l > r) {
        assert(false && "substr_View");
    }
    return (String_View){ .ptr = str.ptr + l, .len = r - l};
}


bool startswith_sv(String_View search, String_View comp) {
    int i;
    for (i = 0; i < MIN(search.len, comp.len); ++i) {
        if (search.ptr[i] != comp.ptr[i]) {
            return false;
        }
    }
    if (i != comp.len) {
        return false;
    }

    return true;
}


bool comp_eq_sv(String_View lhs, String_View rhs) {
    if (lhs.len != rhs.len) {
        return false;
    }
    for (int i = 0; i < lhs.len; ++i) {
        if (lhs.ptr[i] != rhs.ptr[i]) {
            return false;
        }
    }
    return true;
}


#endif

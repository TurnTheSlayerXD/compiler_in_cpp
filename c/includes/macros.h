#ifndef MY_MACROS_H
#define MY_MACROS_H

#define MIN(a, b) ((int)(a) < (int)(b) ? (int)(a) : (int)(b))

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif
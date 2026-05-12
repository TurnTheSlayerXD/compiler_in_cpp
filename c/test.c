


#include <stdio.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

int main() {
    printf("sizeof str = %d\n", COUNT_OF("hello"));
}

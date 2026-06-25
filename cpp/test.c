


#include <stdio.h>
#include <string.h>


typedef struct {
    char buf[8];
} Small;

typedef struct {
    char buf[69];
} Big;

Big fuu(Big b1, const char *str, int b, char c, Big b2) {

    for (int i = 0; i < sizeof(b1.buf); ++i) {
        b1.buf[i] = i % 8;
        printf("%c", b1.buf[i]);
    }
    for (int i = 0; i < sizeof(b2.buf); ++i) {
        b2.buf[i] = i % 8;
        printf("%c", b2.buf[i]);
    }
    printf("%p, %p, %p, %p, %p", &b1, &str, &b, &c, &b2);

    if (c > 0) {
        return b1;
    }
    else {
        return b2;
    }
}

Big bar(Big b1, Small b2, int j) {
    for (int i = 0; i < sizeof(b1.buf); ++i) {
        b1.buf[i] = b1.buf[i] + i % 8;
        printf("%c", b1.buf[i]);
    }

    for (int i = 0; i < sizeof(b2.buf); ++i) {
        b2.buf[i] = b2.buf[i] + i % 8;
        printf("%c", b2.buf[i]);
    }
    printf("%p, %p", &b1, &b2);

    if (j > 0) {
        return b1;
    }
    else {
        return b1;
    }
}



int main(int argc, char **argv) {

    fuu((Big){0}, 0, 0, 0, (Big){0});


    bar((Big){0}, (Small){0}, -1);
}
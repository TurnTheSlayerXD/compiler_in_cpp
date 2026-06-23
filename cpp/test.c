


#include <stdio.h>
#include <string.h>

void print_str(const char *str) {
    printf("%s, %d", str, strlen(str));
}


int main() {


    print_str("Hello, world");

}
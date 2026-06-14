
#include <stdio.h>
#include <string.h>
#include <locale.h>

int main() {



    const char* str = "сука";

    printf("%llu, %s\n", strlen(str), str);

    return 0;
}
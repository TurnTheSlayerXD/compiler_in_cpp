
int printf(const char *fmt, ...);


int fuu(int a, int b, int c) {
    printf("%d, %d, %d", a, b, c);

    return a + b + c;
}

int main(int argc, char** argv) { 
    int i = 0;
    while(i < argc) {
       if(i % 2 == 0) { 
           printf("odd\n"); 
       }
       else { 
           printf("even\n");
       } 
       i = i + 1;
    }


    fuu(1, 2, 3);
 }

int printf(const char *fmt, ...);
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
 }
#include <iostream>


int *fuu() {
    int a;
    return &a;
}

int main() {
    *fuu()+1 = 1;
}
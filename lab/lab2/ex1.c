#include <stdio.h>

int main(){
    signed char a = 127;
    unsigned char b = 0xff;
    unsigned char c = 0;

    a++;
    b++;
    c--;

    printf("a = %d\n", a);
    printf("b = %d\n", b);
    printf("c = %d\n", c);
}
#include <stdio.h>
union data{
int n;
char ch;
short m;
};
int main()
{
    union data a;
    printf("%ld, %ld\n", sizeof(a), sizeof(union data) );
    a.n = 0x40;// 64, which is @ in ASCII
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);//%X means print int in hex, %c means print as character, %hX means print short in hex
    a.ch = '9';
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    a.m = 0x2059;// 8281, which is 20 59 in hex
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    a.n = 0x3E25AD54;
    printf("%X, %c, %hX\n", a.n, a.ch, a.m);
    return 0;
}
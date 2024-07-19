#include <iostream>

using namespace std;
void swap(int *a, int *b);
int main(){
    int a = 10;
    int b = 20;
    cout << "Before swap: a = " << a << " b = " << b << endl;
    swap(&a, &b);
    cout << "After swap: a = " << a << " b = " << b << endl;
    return 0;
}
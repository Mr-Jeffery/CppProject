#include <iostream>
#include <cmath>

using namespace std;

template <typename T>
bool vabs(T* p, size_t n);

int main(){
    int p[] = {1, -2, 3, -4, 5};
    vabs(p, 5);
    for (int i = 0; i < 5; i++) {
        cout << p[i] << " ";
    }
    cout << endl;
}
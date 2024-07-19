#include "fib.hpp"
#include <iostream> 

int main() {
    int n = -1;
    while (n <= 0){
        std::cout << "Enter the value of n: ";
        std::cin >> n;
    }
    std::cout << "Fibonacci numbers from 1 to " << n << ":" << std::endl;
    for (int i = 1; i <= n; i++) {
        std::cout << fibonacci(i) << " ";
        if (i % 10 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    return 0;
}
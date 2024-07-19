// fib.cpp

unsigned long long fibonacci(int n) {
    if (n <= 0) {
        return 0;
    } else if (n == 1 || n == 2) {
        return 1;
    } else {
        unsigned long long prev = 1;
        unsigned long long curr = 1;
        unsigned long long fib = 0;
        
        for (int i = 3; i <= n; i++) {
            fib = prev + curr;
            prev = curr;
            curr = fib;
        }
        
        return fib;
    }
}
// #include <stdio.h>
// #include <stdint.h>

// #define MULTIPLIER 0x5DEECE66DLL
// #define ADDEND 0xBLL
// #define MASK ((1LL << 48) - 1)

// uint64_t seed = 1; // Initialize seed with 1

// uint32_t next(int bits) {
//     seed = (seed * MULTIPLIER + ADDEND) & MASK;
//     return (uint32_t)(seed >> (48 - bits));
// }

// double nextDouble() {
//     return (((uint64_t)next(24) << 40) >> 40) / (double)(1LL << 24);
// }

// int main() {
//     int i;
//     for(i = 0; i < 5; i++) {
//         printf("%f\n", nextDouble());
//     }
//     return 0;
// }
#include <stdio.h>
#include <stdint.h>

#define MULTIPLIER 0x5DEECE66DL
#define ADDEND 0xBL
#define MASK ((1L << 48) - 1)

uint32_t seed = 1; // Initialize seed with 1

int next(int bits) {
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
    printf("seed: %d\n", seed);
    return (int)(seed >> (48 - bits));
}

float nextFloat() {
    return next(24) / ((float)(1 << 24));
}

int nextInt() {
    return next(32);
}

int main() {
    int i;
    seed = 12345;
    printf("seed: %d\n", seed);
    for(i = 0; i < 5; i++) {
        printf("%d\n", nextInt());
    }
    // for(i = 0; i < 5; i++) {
    //     printf("%f\n", nextFloat());
    // }
    return 0;
}
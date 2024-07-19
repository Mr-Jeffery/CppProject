#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <immintrin.h> // for AVX
// #include <time.h>
#include <sys/time.h>


// inline float sum_m256(__m256 v) {
//     __m128 vlow = _mm256_castps256_ps128(v);
//     __m128 vhigh = _mm256_extractf128_ps(v, 1); // high 128
//     vlow = _mm_add_ps(vlow, vhigh); // add the low 128
//     __m128 shuf = _mm_movehdup_ps(vlow); // broadcast elements 3,1 to 2,0
//     __m128 sums = _mm_add_ps(vlow, shuf);
//     shuf = _mm_movehl_ps(shuf, sums); // high half -> low half
//     sums = _mm_add_ss(sums, shuf);
//     return  _mm_cvtss_f32(sums);
// }

// dot product of two __m256 vectors
inline float dp_m256(__m256 a, __m256 b) {
    const int imm8 = 0b11110001;
    // four high-order bits set to 1 so that all 8 floats are multiplied, 
    // and 0001 means only keep the result in the loest 32 bits of each 128-bit lane.
    __m256 c = _mm256_dp_ps(a, b, imm8);
    c = _mm256_add_ps(c, _mm256_permute2f128_ps(c, c, 1)); // add the two 128-bit lanes
    // cvtss then add the two 32-bit floats in the 128-bit lane to get the final result.
    return _mm256_cvtss_f32(c);
}

inline float sum_m256(__m256 v) {
    // dot product of v with a vector of 1.0f is the sum of all elements in v
    return dp_m256(v, _mm256_set1_ps(1.0f));
}

// void optimized_multiply(float* A, float* B, float* C, size_t N) {
//     size_t N_pad = (M + 7) & ~7; // size of matrices padded to multiple of 8
//     size_t N_pad = (N + 7) & ~7; 
//     #pragma omp parallel for
//     for (size_t i = 0; i < N; i++) {
//         for (size_t j = 0; j < N; j+=8) {
//             __m256 sum = _mm256_setzero_ps(); // initialize sum to 0
//             for (size_t k = 0; k < N; k++) {
//                 __m256 a = _mm256_set1_ps(A[i*N_pad + k]); // load A[i][k] into a
//                 __m256 b = _mm256_load_ps(&B[k*N_pad + j]); // load B[k][j:j+8] into b
//                 sum = _mm256_add_ps(sum, _mm256_mul_ps(a, b)); // multiply and add (SIMD)
//             }
//             if (i < N && j < N) {
//                 _mm256_storeu_ps(&C[i*N_pad + j], sum); // store sum in C[i][j:j+8]
//             }
//         }
//     }
// }

void optimized_multiply(float* A, float* B, float* C, size_t N) {
    size_t N_pad = (N + 7) & ~7; // size of matrices padded to multiple of 8
    size_t ib = N_pad >256 ? 256 : N_pad;
    size_t jb = N_pad >512 ? 512 : N_pad;
    size_t kb = N_pad >16 ? 16 : N_pad;
    #pragma omp parallel for
    for (size_t ii = 0; ii<N; ii += ib) {
        for (size_t jj = 0; jj<N; jj += jb) {
            for (size_t kk = 0; kk<N; kk += kb) {
                for (size_t i = ii; i < ii + ib; i+=2) {
                    for (size_t j = jj; j < jj + jb; j+=16) {
                        size_t b_index = (j-jj) * kb + kk * jb + jj * N_pad;
                        __m256 sumA_1, sumB_1, sumA_2, sumB_2;
                        if (kk == 0) {
                            sumA_1 = _mm256_setzero_ps();
                            sumA_2 = _mm256_setzero_ps();
                            sumB_1 = _mm256_setzero_ps();
                            sumB_2 = _mm256_setzero_ps();
                        } else {
                            sumA_1 = _mm256_loadu_ps(&C[i*N_pad + j]);
                            sumA_2 = _mm256_loadu_ps(&C[i*N_pad + j]);
                            sumB_1 = _mm256_loadu_ps(&C[(i+1)*N_pad + j+8]);
                            sumB_2 = _mm256_loadu_ps(&C[(i+1)*N_pad + j+8]);
                        }
                        for (size_t k = kk; k < kk + kb; k++) {
                            __m256 a = _mm256_set1_ps(A[i*N_pad + k]); // load A[i][k] into a
                            __m256 b1 = _mm256_load_ps(&B[k*N_pad + j]); // load B[k][j:j+8] into b
                            __m256 b2 = _mm256_load_ps(&B[k*N_pad + j+8]); // load B[k][j+8:j+16] into b
                            sum = _mm256_add_ps(sum, _mm256_mul_ps(a, b)); // multiply and add (SIMD)
                        }
                        if (i < N && j < N) {
                            _mm256_storeu_ps(&C[i*N_pad + j], sum); // store sum in C[i][j:j+8]
                        }
                    }
                }
            }
        }
    
    }
}


inline void transpose(float* A, float* At, int N) {
    int N_pad = (N + 7) & ~7; // size of matrices padded to multiple of 8
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j+=8) {
            __m256 a0 = _mm256_set_ps(A[(j+7)*N_pad + i], A[(j+6)*N_pad + i], A[(j+5)*N_pad + i], A[(j+4)*N_pad + i], A[(j+3)*N_pad + i], A[(j+2)*N_pad + i], A[(j+1)*N_pad + i], A[j*N_pad + i]);
            _mm256_store_ps(&At[i*N_pad + j], a0);
        }
    }
}

// void optimized_multiply(float* A, float* B, float* C, int M,int N) {
//     int N_pad = (N + 7) & ~7; // size of matrices padded to multiple of 8
//     float* Bt = (float*) aligned_alloc(32, N_pad * N_pad * sizeof(float)); // allocate memory for Bt
//     transpose(B, Bt, N);
//     #pragma omp parallel for
//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < N; j+=8) {
//             __m256 sum = _mm256_setzero_ps(); // initialize sum to 0
//             for (int k = 0; k < N; k++) {
//                 __m256 a = _mm256_set1_ps(A[i*N_pad + k]); // load A[i][k] into a
//                 __m256 b = _mm256_load_ps(&Bt[j*N_pad + k]); // load Bt[j:j+8][k] into b
//                 sum = _mm256_add_ps(sum, _mm256_mul_ps(a, b)); // multiply and add (SIMD)
//             }
//             if (i < N && j < N) {
//                 _mm256_storeu_ps(&C[i*N_pad + j], sum); // store sum in C[i][j:j+8]
//             }
//         }
//     }
//     free(Bt);
// }

int main() {
    size_t N = (int)1024; // original size of matrices
    size_t N_pad = (N + 7) & ~7; // size of matrices padded to multiple of 8

    float *A = (float*) aligned_alloc(32, N_pad * N_pad * sizeof(float)); // allocate memory for A
    float *B = (float*) aligned_alloc(32, N_pad * N_pad * sizeof(float)); // allocate memory for B
    float *C = (float*) aligned_alloc(32, N_pad * N_pad * sizeof(float)); // allocate memory for C

    // Initialize A and B matrices
    for (size_t i = 0; i < N_pad; i++) {
        for (size_t j = 0; j < N_pad; j++) {
            A[i*N_pad + j] = (i < N && j < N) ? 1.0f + 1.0f*i: 0.0f; // A[i][j]
            B[i*N_pad + j] = (i < N && j < N) ? 1.0f + 1.0f*j: 0.0f; // B[i][j]
        }
    }

    struct timeval start, end;
    int t = 1; // number of times to multiply A and B
    printf("Starting multiplication, t = %d\n", t);
    gettimeofday(&start, NULL);
    for (size_t i = 0; i < t; i++){
        for (size_t j = 0; j < N*N; j++) {
            C[j] = 0;
        }
        optimized_multiply(A, B, C, N);
        // block_multiply(A, B, C, N, block_size);

    }
    gettimeofday(&end, NULL);

    // transpose(A, B, N);

    double elapsed = (end.tv_sec - start.tv_sec) + 
                     ((end.tv_usec - start.tv_usec)/1000000.0);
    printf("Time spent: %f\n", elapsed);

    // // Use result in C...
    // for (int i = 0; i < N_pad; i++) {
    //     for (int j = 0; j < N_pad; j++) {
    //         printf("%f ",C[i*N_pad + j]);
    //     }
    //     printf("\n");
    // }

    free(A);
    free(B);
    free(C);

    return 0;
}

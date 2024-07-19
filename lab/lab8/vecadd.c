#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <immintrin.h> // for AVX
#include <sys/time.h>

int main(){
    struct timeval start, end;
    int N = (int)1e8;
    float* vecA = (float*)malloc(N * sizeof(float));
    float* vecB = (float*)malloc(N * sizeof(float));
    float* vecC = (float*)malloc(N * sizeof(float));
    for (int i = 0; i<N; i++){
        vecA[i]=1.0f;
        vecB[i]=2.0f;
    }
    gettimeofday(&start,NULL); 

    #pragma omp parallel for
    for (int i = 0;i < N; i++){
        vecC[i] = vecA[i] + vecB[i];
    }
    gettimeofday(&end,NULL); 
    double elapsed_time = (end.tv_usec-start.tv_usec);
    printf("pure C time: %lf us\n",elapsed_time);

    gettimeofday(&start,NULL);  
    
    // for (int k = 0; k < thread; k++)
    // {
    //     // #pragma omp parallel for  
    //     for (int i = k * M; i < (k+1)* M; i += 8){
    //         __m256 a = _mm256_set1_ps(1.0f);
    //         __m256 b = _mm256_set1_ps(2.0f);
    //         __m256 c = _mm256_add_ps(a, b);
    //         vecC[i] = _mm256_cvtss_f32(c);
    //     }
    // }
    #pragma omp parallel for
    for (int i = 0; i < N ; i += 8){
        __m256 a = _mm256_set1_ps(1.0f);
        __m256 b = _mm256_set1_ps(2.0f);
        __m256 c = _mm256_add_ps(a, b);
        vecC[i] = _mm256_cvtss_f32(c);
    }
    gettimeofday(&end,NULL);
    elapsed_time = (end.tv_usec-start.tv_usec);
    printf("SIMD time: %lf us\n",elapsed_time);
}
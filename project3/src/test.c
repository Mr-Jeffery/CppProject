# include <stdio.h>
# include "matrix.h"
#include <sys/time.h>

int main() {
    size_t m = 8e3;
    size_t n = m;
    size_t p = m;

    Matrix* A = createMatrix(m, n);
    Matrix* B = createMatrix(n, p);
    Matrix* C = createMatrix(B->m,A->n);
    setZero(C);

    // Initialize A and B
    for (size_t i = 0; i < A->m; i++) {
        for (size_t j = 0; j < A->n; j++) {
            A->array[j*A->m_pad + i] = 1.0f;
        }
    }
    for (size_t i = 0; i < B->m; i++) {
        for (size_t j = 0; j < B->n; j++) {
            B->array[j*B->m_pad + i] = 1.0f;
        }
    }
    for (size_t i = 0; i < C->m; i++) {
        for (size_t j = 0; j < C->n; j++) {
            C->array[j*C->m_pad + i] = 0.0f;
        }
    }

    // Multiply A and B, store result in C

    struct timeval start, end;
    double elapsed;
    // gettimeofday(&start, NULL);
    // multiply0(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply0 Time spent: %f\n", elapsed);


    // gettimeofday(&start, NULL);
    // multiply1(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply1 Time spent: %f\n", elapsed);

    // gettimeofday(&start, NULL);
    // multiply2(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply2 Time spent: %f\n", elapsed);    

    // gettimeofday(&start, NULL);
    // multiply3(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply3 Time spent: %f\n", elapsed);    

    // gettimeofday(&start, NULL);
    // multiply4(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply4 Time spent: %f\n", elapsed); 

    // gettimeofday(&start, NULL);
    // multiply5(A, B, C);
    // gettimeofday(&end, NULL);
    // elapsed = (end.tv_sec - start.tv_sec) + 
    //                 ((end.tv_usec - start.tv_usec)/1000000.0);
    // printf("multiply5 Time spent: %f\n", elapsed);

    gettimeofday(&start, NULL);
    multiply6(A, B, C);
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + 
                    ((end.tv_usec - start.tv_usec)/1000000.0);
    printf("multiply6 Time spent: %f\n", elapsed);
    // // Print the result
    // printf("Matrix A:\n");
    // printMatrix(A);
    // printf("Matrix B:\n");
    // printMatrix(B);
    // printf("Matrix C:\n");
    // printMatrix(C);

    free(A);
    free(B);
    free(C);

    return 0;
}
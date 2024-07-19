#include <stdlib.h>
#include <omp.h>
#include <immintrin.h> // for AVX

#define BLOCK_SIZE 64
typedef struct {
    size_t m;           // Number of rows
    size_t n;           // Number of columns
    size_t m_pad;       // Number of rows padded to multiple of BLOCK_SIZE
    size_t n_pad;       // Number of columns padded to multiple of BLOCK_SIZE
    float* array;    // Pointer to the matrix elements
    // floats are stored in column-major order
} Matrix;

Matrix* createMatrix(size_t m, size_t n) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    matrix->m = m;
    matrix->n = n;
    matrix->m_pad = (m + BLOCK_SIZE-1) & ~(BLOCK_SIZE-1);
    matrix->n_pad = (n + BLOCK_SIZE-1) & ~(BLOCK_SIZE-1);
    matrix->array = (float*)aligned_alloc(32, matrix->m_pad * matrix->n_pad * sizeof(float));
    return matrix;
}

void freeMatrix(Matrix* matrix) {
    free(matrix->array);
    free(matrix);
}

void printMatrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->m; i++) {
        for (size_t j = 0; j < matrix->n; j++) {
            printf("%.2f\t", matrix->array[j*matrix->m_pad + i]);
        }
        printf("\n");
    }
}

void setZero(Matrix* matrix) {
    for (size_t i = 0; i < matrix->m; i++) {
        for (size_t j = 0; j < matrix->n; j++) {
            matrix->array[i + j*matrix->m_pad] = 0.0f;
        }
    }
}



int multiply(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    for (size_t i = 0; i < C->m; i++) {
        for (size_t j = 0; j < C->n; j++) {
            for (size_t k = 0; k < A->m; k++) {
                C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
            }
        }
    }
    return 0;
}

int openmp_multiply(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    
    for (size_t j = 0; j < C->n; j++) {
         
        for (size_t k = 0; k < A->m; k++) {
        
            // # pragma omp parallel for   
            for (size_t i = 0; i < C->m; i++) {
                C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
            }
        }
    }
    return 0;
}

int simd_openmp_multiply(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }

    # pragma omp parallel for
    for (size_t j = 0; j < C->n; j++) {
        
        for (size_t k = 0; k < A->m; k++) {
            __m256 a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
            
            for (size_t i = 0; i < C->m; i+=16) {
                // C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
                __m256 b1 = _mm256_load_ps(&B->array[i + k*B->m_pad]);
                __m256 b2 = _mm256_load_ps(&B->array[i + 8 + k*B->m_pad]);
                __m256 c1 = _mm256_load_ps(&C->array[i + j*C->m_pad]);
                __m256 c2 = _mm256_load_ps(&C->array[i + 8 + j*C->m_pad]);
                _mm256_store_ps(&C->array[i + j*C->m_pad], _mm256_fmadd_ps(a, b1, c1));
                _mm256_store_ps(&C->array[i + 8 + j*C->m_pad], _mm256_fmadd_ps(a, b2, c2));
            }
        }
    }
    return 0;
}

//step by step
int multiply0(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i++) {
        for (size_t j = 0; j < C->n; j++) {
            for (size_t k = 0; k < A->m; k++) {
                C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
            }
        }
    }
    return 0;
}
// 1 change order and 8 floats at a time
int multiply1(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
        for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
            for (size_t k = 0; k < A->m; k++) {
                __m256 a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
                // C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
                __m256 b = _mm256_load_ps(&B->array[i/*:i+7*/ + k*B->m_pad]);
                __m256 c = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
                _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], _mm256_fmadd_ps(a, b, c));
            }
        }
    }
    return 0;
}
// 2 use register to store the values
int multiply2(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
        for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
            __m256 c = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
            for (size_t k = 0; k < A->m; k++) {
                __m256 a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
                __m256 b = _mm256_load_ps(&B->array[i/*:i+7*/ + k*B->m_pad]);
                c = _mm256_fmadd_ps(a, b, c);
            }
            _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
        }
    }
    return 0;
}
// 3 calculate the pointers for B in advance
int multiply3(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
        for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
            __m256 c = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
            size_t b_ptr = i;
            size_t bmpad = B->m_pad;
            for (size_t k = 0; k < A->m; k++) {
                __m256 a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
                __m256 b = _mm256_load_ps(&B->array[b_ptr]);// b_ptr = i + k*B->m_pad
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;
            }
            _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
        }
    }
    return 0;
}
// 4 expand loop
int multiply4(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
        for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
            __m256 c = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
            size_t b_ptr = i;
            size_t bmpad = B->m_pad;
            __m256 a,b;
            for (size_t k = 0; k < A->m; k+=8) {
                a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 1 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 2 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 3 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 4 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 5 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 6 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;

                a = _mm256_set1_ps(A->array[k + 7 + j*A->m_pad]);
                b = _mm256_load_ps(&B->array[b_ptr]);
                c = _mm256_fmadd_ps(a, b, c);
                b_ptr += bmpad;
            }
            _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
        }
    }
    return 0;
}

int multiply5(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
        for (size_t j = 0; j < C->n; j+=8) {/*Loow over the rows of C */
            size_t b0_ptr = i;// b_ptr = i + k*B->m_pad
            size_t b1_ptr = i + 1*B->m_pad;
            size_t b2_ptr = i + 2*B->m_pad;
            size_t b3_ptr = i + 3*B->m_pad;
            size_t b4_ptr = i + 4*B->m_pad;
            size_t b5_ptr = i + 5*B->m_pad;
            size_t b6_ptr = i + 6*B->m_pad;

            size_t bmpad = B->m_pad;
            __m256 b0, b1, b2, b3, b4, b5, b6, b7;
            __m256 a0, a1, a2, a3, a4, a5, a6, a7;
            __m256 c0 = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
            __m256 c1 = _mm256_load_ps(&C->array[i + (j+1)*C->m_pad]);
            __m256 c2 = _mm256_load_ps(&C->array[i + (j+2)*C->m_pad]);
            __m256 c3 = _mm256_load_ps(&C->array[i + (j+3)*C->m_pad]);
            __m256 c4 = _mm256_load_ps(&C->array[i + (j+4)*C->m_pad]);
            __m256 c5 = _mm256_load_ps(&C->array[i + (j+5)*C->m_pad]);
            __m256 c6 = _mm256_load_ps(&C->array[i + (j+6)*C->m_pad]);
            __m256 c7 = _mm256_load_ps(&C->array[i + (j+7)*C->m_pad]);
            for (size_t k = 0; k < A->m; k+=8) {
                size_t a_ptr;
                b0 = _mm256_load_ps(&B->array[i + k*B->m_pad]);// b_ptr = i + k*B->m_pad
                b1 = _mm256_load_ps(&B->array[i + (k+1)*B->m_pad]);
                b2 = _mm256_load_ps(&B->array[i + (k+2)*B->m_pad]);
                b3 = _mm256_load_ps(&B->array[i + (k+3)*B->m_pad]);
                b4 = _mm256_load_ps(&B->array[i + (k+4)*B->m_pad]);
                b5 = _mm256_load_ps(&B->array[i + (k+5)*B->m_pad]);
                b6 = _mm256_load_ps(&B->array[i + (k+6)*B->m_pad]);
                b7 = _mm256_load_ps(&B->array[i + (k+7)*B->m_pad]);

                a_ptr = k + j*A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b0, c1);
                c2 = _mm256_fmadd_ps(a2, b0, c2);
                c3 = _mm256_fmadd_ps(a3, b0, c3);
                c4 = _mm256_fmadd_ps(a4, b0, c4);
                c5 = _mm256_fmadd_ps(a5, b0, c5);
                c6 = _mm256_fmadd_ps(a6, b0, c6);
                c7 = _mm256_fmadd_ps(a7, b0, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);

                a_ptr += A->m_pad;
                a0 = _mm256_set1_ps(A->array[a_ptr]);
                a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                c0 = _mm256_fmadd_ps(a0, b0, c0);
                c1 = _mm256_fmadd_ps(a1, b1, c1);
                c2 = _mm256_fmadd_ps(a2, b2, c2);
                c3 = _mm256_fmadd_ps(a3, b3, c3);
                c4 = _mm256_fmadd_ps(a4, b4, c4);
                c5 = _mm256_fmadd_ps(a5, b5, c5);
                c6 = _mm256_fmadd_ps(a6, b6, c6);
                c7 = _mm256_fmadd_ps(a7, b7, c7);
            }
            _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c0);
            _mm256_store_ps(&C->array[i + (j+1)*C->m_pad], c1);
            _mm256_store_ps(&C->array[i + (j+2)*C->m_pad], c2);
            _mm256_store_ps(&C->array[i + (j+3)*C->m_pad], c3);
            _mm256_store_ps(&C->array[i + (j+4)*C->m_pad], c4);
            _mm256_store_ps(&C->array[i + (j+5)*C->m_pad], c5);
            _mm256_store_ps(&C->array[i + (j+6)*C->m_pad], c6);
            _mm256_store_ps(&C->array[i + (j+7)*C->m_pad], c7);
        }
    }
    return 0;
}
int multiply6(Matrix* A, Matrix* B, Matrix* C) {
    if (A->m != B->n) {
        printf("Error: Matrix dimensions do not match\n");
        return 1;
    }
    # pragma omp parallel for
    for (size_t jj = 0; jj <  C->n_pad; jj+=BLOCK_SIZE) {
    for (size_t ii = 0; ii < C->m_pad; ii+=BLOCK_SIZE*4) {
    for (size_t kk = 0; kk < A->m_pad; kk+=BLOCK_SIZE) {
        for (size_t j = jj; j < jj + BLOCK_SIZE; j+=8) {/*Loow over the rows of C */
        for (size_t i = ii; i < ii + BLOCK_SIZE*4; i+=8) {/*Loop over the columns of C */
            __m256 b0, b1, b2, b3, b4, b5, b6, b7;
            __m256 a0, a1, a2, a3, a4, a5, a6, a7;
            # pragma unroll
            for (size_t k = kk; k < kk + BLOCK_SIZE; k+=8) {
                size_t a_ptr;
                b0 = _mm256_load_ps(&B->array[i + k*B->m_pad]);// b_ptr = i + k*B->m_pad
                b1 = _mm256_load_ps(&B->array[i + (k+1)*B->m_pad]);
                b2 = _mm256_load_ps(&B->array[i + (k+2)*B->m_pad]);
                b3 = _mm256_load_ps(&B->array[i + (k+3)*B->m_pad]);
                b4 = _mm256_load_ps(&B->array[i + (k+4)*B->m_pad]);
                b5 = _mm256_load_ps(&B->array[i + (k+5)*B->m_pad]);
                b6 = _mm256_load_ps(&B->array[i + (k+6)*B->m_pad]);
                b7 = _mm256_load_ps(&B->array[i + (k+7)*B->m_pad]);
                # pragma unroll
                for (size_t l = 0; l < 8; l++) {
                    a_ptr = k + (j+l)*A->m_pad;
                    __m256 c = _mm256_load_ps(&C->array[i + (l + j)*C->m_pad]);
                    a0 = _mm256_set1_ps(A->array[a_ptr]);
                    a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
                    a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
                    a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
                    a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
                    a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
                    a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
                    a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
                    c = _mm256_fmadd_ps(a0, b0, c);
                    c = _mm256_fmadd_ps(a1, b1, c);
                    c = _mm256_fmadd_ps(a2, b2, c);
                    c = _mm256_fmadd_ps(a3, b3, c);
                    c = _mm256_fmadd_ps(a4, b4, c);
                    c = _mm256_fmadd_ps(a5, b5, c);
                    c = _mm256_fmadd_ps(a6, b6, c);
                    c = _mm256_fmadd_ps(a7, b7, c);
                    _mm256_store_ps(&C->array[i + (l + j)*C->m_pad], c);
                }
            }
        }
        }
    }                          
    }
    }
    return 0;
}

// int multiply6(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t jj = 0; jj <  C->n_pad; jj+=BLOCK_SIZE) {
//     for (size_t ii = 0; ii < C->m_pad; ii+=BLOCK_SIZE*16) {
//     for (size_t kk = 0; kk < A->m_pad; kk+=BLOCK_SIZE) {
//         for (size_t i = ii; i < ii + BLOCK_SIZE*16; i+=8) {/*Loop over the columns of C */
//         for (size_t j = jj; j < jj + BLOCK_SIZE; j+=8) {/*Loow over the rows of C */
//             size_t b0_ptr = i;// b_ptr = i + k*B->m_pad
//             size_t b1_ptr = i + 1*B->m_pad;
//             size_t b2_ptr = i + 2*B->m_pad;
//             size_t b3_ptr = i + 3*B->m_pad;
//             size_t b4_ptr = i + 4*B->m_pad;
//             size_t b5_ptr = i + 5*B->m_pad;
//             size_t b6_ptr = i + 6*B->m_pad;

//             size_t bmpad = B->m_pad;
//             __m256 b0, b1, b2, b3, b4, b5, b6, b7;
//             __m256 a0, a1, a2, a3, a4, a5, a6, a7;
//             __m256 c0 = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//             __m256 c1 = _mm256_load_ps(&C->array[i + (j+1)*C->m_pad]);
//             __m256 c2 = _mm256_load_ps(&C->array[i + (j+2)*C->m_pad]);
//             __m256 c3 = _mm256_load_ps(&C->array[i + (j+3)*C->m_pad]);
//             __m256 c4 = _mm256_load_ps(&C->array[i + (j+4)*C->m_pad]);
//             __m256 c5 = _mm256_load_ps(&C->array[i + (j+5)*C->m_pad]);
//             __m256 c6 = _mm256_load_ps(&C->array[i + (j+6)*C->m_pad]);
//             __m256 c7 = _mm256_load_ps(&C->array[i + (j+7)*C->m_pad]);
//             for (size_t k = kk; k < kk + BLOCK_SIZE; k+=8) {
//                 size_t a_ptr;
//                 b0 = _mm256_load_ps(&B->array[i + k*B->m_pad]);// b_ptr = i + k*B->m_pad
//                 b1 = _mm256_load_ps(&B->array[i + (k+1)*B->m_pad]);
//                 b2 = _mm256_load_ps(&B->array[i + (k+2)*B->m_pad]);
//                 b3 = _mm256_load_ps(&B->array[i + (k+3)*B->m_pad]);
//                 b4 = _mm256_load_ps(&B->array[i + (k+4)*B->m_pad]);
//                 b5 = _mm256_load_ps(&B->array[i + (k+5)*B->m_pad]);
//                 b6 = _mm256_load_ps(&B->array[i + (k+6)*B->m_pad]);
//                 b7 = _mm256_load_ps(&B->array[i + (k+7)*B->m_pad]);

//                 a_ptr = k + j*A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a0, b0, c0);
//                 c1 = _mm256_fmadd_ps(a0, b1, c1);
//                 c2 = _mm256_fmadd_ps(a0, b2, c2);
//                 c3 = _mm256_fmadd_ps(a0, b3, c3);
//                 c4 = _mm256_fmadd_ps(a0, b4, c4);
//                 c5 = _mm256_fmadd_ps(a0, b5, c5);
//                 c6 = _mm256_fmadd_ps(a0, b6, c6);
//                 c7 = _mm256_fmadd_ps(a0, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a1, b0, c0);
//                 c1 = _mm256_fmadd_ps(a1, b1, c1);
//                 c2 = _mm256_fmadd_ps(a1, b2, c2);
//                 c3 = _mm256_fmadd_ps(a1, b3, c3);
//                 c4 = _mm256_fmadd_ps(a1, b4, c4);
//                 c5 = _mm256_fmadd_ps(a1, b5, c5);
//                 c6 = _mm256_fmadd_ps(a1, b6, c6);
//                 c7 = _mm256_fmadd_ps(a1, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a2, b0, c0);
//                 c1 = _mm256_fmadd_ps(a2, b1, c1);
//                 c2 = _mm256_fmadd_ps(a2, b2, c2);
//                 c3 = _mm256_fmadd_ps(a2, b3, c3);
//                 c4 = _mm256_fmadd_ps(a2, b4, c4);
//                 c5 = _mm256_fmadd_ps(a2, b5, c5);
//                 c6 = _mm256_fmadd_ps(a2, b6, c6);
//                 c7 = _mm256_fmadd_ps(a2, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a3, b0, c0);
//                 c1 = _mm256_fmadd_ps(a3, b1, c1);
//                 c2 = _mm256_fmadd_ps(a3, b2, c2);
//                 c3 = _mm256_fmadd_ps(a3, b3, c3);
//                 c4 = _mm256_fmadd_ps(a3, b4, c4);
//                 c5 = _mm256_fmadd_ps(a3, b5, c5);
//                 c6 = _mm256_fmadd_ps(a3, b6, c6);
//                 c7 = _mm256_fmadd_ps(a3, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a4, b0, c0);
//                 c1 = _mm256_fmadd_ps(a4, b1, c1);
//                 c2 = _mm256_fmadd_ps(a4, b2, c2);
//                 c3 = _mm256_fmadd_ps(a4, b3, c3);
//                 c4 = _mm256_fmadd_ps(a4, b4, c4);
//                 c5 = _mm256_fmadd_ps(a4, b5, c5);
//                 c6 = _mm256_fmadd_ps(a4, b6, c6);
//                 c7 = _mm256_fmadd_ps(a4, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a5, b0, c0);
//                 c1 = _mm256_fmadd_ps(a5, b1, c1);
//                 c2 = _mm256_fmadd_ps(a5, b2, c2);
//                 c3 = _mm256_fmadd_ps(a5, b3, c3);
//                 c4 = _mm256_fmadd_ps(a5, b4, c4);
//                 c5 = _mm256_fmadd_ps(a5, b5, c5);
//                 c6 = _mm256_fmadd_ps(a5, b6, c6);
//                 c7 = _mm256_fmadd_ps(a5, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a6, b0, c0);
//                 c1 = _mm256_fmadd_ps(a6, b1, c1);
//                 c2 = _mm256_fmadd_ps(a6, b2, c2);
//                 c3 = _mm256_fmadd_ps(a6, b3, c3);
//                 c4 = _mm256_fmadd_ps(a6, b4, c4);
//                 c5 = _mm256_fmadd_ps(a6, b5, c5);
//                 c6 = _mm256_fmadd_ps(a6, b6, c6);
//                 c7 = _mm256_fmadd_ps(a6, b7, c7);

//                 a_ptr += A->m_pad;
//                 a0 = _mm256_set1_ps(A->array[a_ptr]);
//                 a1 = _mm256_set1_ps(A->array[a_ptr + 1]);
//                 a2 = _mm256_set1_ps(A->array[a_ptr + 2]);
//                 a3 = _mm256_set1_ps(A->array[a_ptr + 3]);
//                 a4 = _mm256_set1_ps(A->array[a_ptr + 4]);
//                 a5 = _mm256_set1_ps(A->array[a_ptr + 5]);
//                 a6 = _mm256_set1_ps(A->array[a_ptr + 6]);
//                 a7 = _mm256_set1_ps(A->array[a_ptr + 7]);
//                 c0 = _mm256_fmadd_ps(a7, b0, c0);
//                 c1 = _mm256_fmadd_ps(a7, b1, c1);
//                 c2 = _mm256_fmadd_ps(a7, b2, c2);
//                 c3 = _mm256_fmadd_ps(a7, b3, c3);
//                 c4 = _mm256_fmadd_ps(a7, b4, c4);
//                 c5 = _mm256_fmadd_ps(a7, b5, c5);
//                 c6 = _mm256_fmadd_ps(a7, b6, c6);
//                 c7 = _mm256_fmadd_ps(a7, b7, c7);
//             }
//             _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c0);
//             _mm256_store_ps(&C->array[i + (j+1)*C->m_pad], c1);
//             _mm256_store_ps(&C->array[i + (j+2)*C->m_pad], c2);
//             _mm256_store_ps(&C->array[i + (j+3)*C->m_pad], c3);
//             _mm256_store_ps(&C->array[i + (j+4)*C->m_pad], c4);
//             _mm256_store_ps(&C->array[i + (j+5)*C->m_pad], c5);
//             _mm256_store_ps(&C->array[i + (j+6)*C->m_pad], c6);
//             _mm256_store_ps(&C->array[i + (j+7)*C->m_pad], c7);
//         }
//         }
//     }                          
//     }
//     }
// return 0;
// }
// //avx 512
// //step by step
// int multiply0(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i++) {
//         for (size_t j = 0; j < C->n; j++) {
//             for (size_t k = 0; k < A->m; k++) {
//                 C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
//             }
//         }
//     }
//     return 0;
// }
// // 1 change order and 8 floats at a time
// int multiply1(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
//         for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
//             for (size_t k = 0; k < A->m; k++) {
//                 __m512 a = _mm512_set1_ps(A->array[k + j*A->m_pad]);
//                 // C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
//                 __m512 b = _mm512_load_ps(&B->array[i/*:i+7*/ + k*B->m_pad]);
//                 __m512 c = _mm512_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//                 _mm512_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], _mm512_fmadd_ps(a, b, c));
//             }
//         }
//     }
//     return 0;
// }
// // 2 use register to store the values
// int multiply2(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
//         for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
//             register __m512 c = _mm512_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//             for (size_t k = 0; k < A->m; k++) {
//                 __m512 a = _mm512_set1_ps(A->array[k + j*A->m_pad]);
//                 __m512 b = _mm512_load_ps(&B->array[i/*:i+7*/ + k*B->m_pad]);
//                 c = _mm512_fmadd_ps(a, b, c);
//             }
//             _mm512_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
//         }
//     }
//     return 0;
// }
// // 3 calculate the pointers for B in advance
// int multiply3(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
//         for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
//             register __m512 c = _mm512_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//             size_t b_ptr = i;
//             size_t bmpad = B->m_pad;
//             for (size_t k = 0; k < A->m; k++, b_ptr += bmpad) {
//                 __m512 a = _mm512_set1_ps(A->array[k + j*A->m_pad]);
//                 __m512 b = _mm512_load_ps(&B->array[b_ptr]);// b_ptr = i + k*B->m_pad
//                 c = _mm512_fmadd_ps(a, b, c);
//             }
//             _mm512_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
//         }
//     }
//     return 0;
// }
// // 4 expand loop
// int multiply4(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
//         for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
//             register __m256 c = _mm256_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//             size_t b_ptr = i;
//             size_t bmpad = B->m_pad;
//             for (size_t k = 0; k < A->m; k+=8, b_ptr += bmpad) {
//                 __m256 b = _mm256_load_ps(&B->array[b_ptr]);// b_ptr = i + k*B->m_pad
//                 register __m256 a = _mm256_set1_ps(A->array[k + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 1 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 2 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 3 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 4 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 5 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 6 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//                 a = _mm256_set1_ps(A->array[k + 7 + j*A->m_pad]);
//                 c = _mm256_fmadd_ps(a, b, c);
//             }
//             _mm256_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
//         }
//     }
//     return 0;
// }

// // abstract the adddot1x16 function for further improvement later
// inline __m512 adddot1x16(Matrix* A, Matrix* B, Matrix* C, size_t b_ptr, size_t j, size_t k, __m512 c) {
//     __m512 b = _mm512_load_ps(&B->array[b_ptr]);// b_ptr = i + k*B->m_pad
//     size_t a_ptr = k + j*A->m_pad;
//     register __m512 a;
//     for (size_t i = 0; i < 16; i++) {
//         a = _mm512_set1_ps(A->array[a_ptr++]);
//         c = _mm512_fmadd_ps(a, b, c);
//     }
//     return c;
// }

// int multiply4_nested(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t i = 0; i < C->m; i+=8) {/*Loop over the columns of C */
//         for (size_t j = 0; j < C->n; j++) {/*Loow over the rows of C */
//             register __m512 c = _mm512_load_ps(&C->array[i/*:i+7*/ + j*C->m_pad]);
//             size_t b_ptr = i;
//             size_t bmpad = B->m_pad;
//             for (size_t k = 0; k < A->m; k+=16, b_ptr += bmpad) {
//                 c = adddot1x16(A, B, C, b_ptr, j, k, c);
//             }
//             _mm512_store_ps(&C->array[i/*:i+7*/ + j*C->m_pad], c);
//         }
//     }
//     return 0;
// }

// // 5 block version
// int multiply5(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
//     # pragma omp parallel for
//     for (size_t jj = 0; jj <  C->n_pad; jj+=BLOCK_SIZE) {
//     for (size_t ii = 0; ii < C->m_pad; ii+=BLOCK_SIZE*8) {
//     for (size_t kk = 0; kk < A->m_pad; kk+=BLOCK_SIZE*2) {
    
//         for (size_t j = jj; j < jj + BLOCK_SIZE; j+=2) {
//         for (size_t i = ii; i < ii + BLOCK_SIZE*8; i+=16) {
//         register __m512 c1 = _mm512_load_ps(&C->array[i + j*C->m_pad]);
//         register __m512 c2 = _mm512_load_ps(&C->array[i + (j+1)*C->m_pad]);
//         for (size_t k = kk; k < kk + BLOCK_SIZE*2; k+=16) {
//             c1 = adddot1x16(A, B, C, i + k*B->m_pad, j, k, c1);
//             c2 = adddot1x16(A, B, C, i + k*B->m_pad, j+1, k, c2);
//         }
//         _mm512_store_ps(&C->array[i + j*C->m_pad], c1);
//         _mm512_store_ps(&C->array[i + (j+1)*C->m_pad], c2);
//         }
//         }
//     }                       
//     }
//     }
//     return 0;
// }

// // block version
// int openmp_block_multiply(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }
    
//     for (size_t j = 0; j < C->n_pad; j+=BLOCK_SIZE) {
//         for (size_t k = 0; k < A->m_pad; k+=BLOCK_SIZE) {
//             # pragma omp parallel for
//             for (size_t i = 0; i < C->m_pad; i+=BLOCK_SIZE) {
//                 for (size_t jj = j; jj < j + BLOCK_SIZE; jj++) {
//                     for (size_t ii = i; ii < i + BLOCK_SIZE; ii+=16) {
//                         __m256 c1 = _mm256_load_ps(&C->array[i + j*C->m_pad]);
//                         __m256 c2 = _mm256_load_ps(&C->array[ii + 8 + jj*C->m_pad]);
//                             for (size_t kk = k; kk < k + BLOCK_SIZE; kk++) {
//                                 // for (size_t ii = i; ii < i + BLOCK_SIZE; ii++) {
//                                 //     C->array[ii + jj*C->m_pad] += A->array[kk + jj*A->m_pad] * B->array[ii + kk*B->m_pad];
//                                 // }
//                                 __m256 a = _mm256_set1_ps(A->array[kk + jj*A->m_pad]);
//                                 __m256 b1 = _mm256_load_ps(&B->array[ii + kk*B->m_pad]);
//                                 __m256 b2 = _mm256_load_ps(&B->array[ii + 8 + kk*B->m_pad]);
//                                 c1 = _mm256_fmadd_ps(a, b1, c1);
//                                 c2 = _mm256_fmadd_ps(a, b2, c2);
//                             }                  
//                         _mm256_store_ps(&C->array[ii + jj*C->m_pad], c1);
//                         _mm256_store_ps(&C->array[i + 8 + jj*C->m_pad], c2);      
//                     }
//                 }
//             }
//         }
//     }
//     return 0;
// }

// avx512
// 
// int simd_openmp_multiply(Matrix* A, Matrix* B, Matrix* C) {
//     if (A->m != B->n) {
//         printf("Error: Matrix dimensions do not match\n");
//         return 1;
//     }

//     // # pragma omp parallel for
//     for (size_t j = 0; j < C->n; j++) {
        
//         for (size_t k = 0; k < A->m; k++) {
//             __m512 a = _mm512_set1_ps(A->array[k + j*A->m_pad]);
            
//             for (size_t i = 0; i < C->m; i+=16) {
//                 // C->array[i + j*C->m_pad] += A->array[k + j*A->m_pad] * B->array[i + k*B->m_pad];
//                 __m512 b1 = _mm512_load_ps(&B->array[i + k*B->m_pad]);
//                 __m512 b2 = _mm512_load_ps(&B->array[i + 16 + k*B->m_pad]);
//                 __m512 c1 = _mm512_load_ps(&C->array[i + j*C->m_pad]);
//                 __m512 c2 = _mm512_load_ps(&C->array[i + 16 + j*C->m_pad]);
//                 _mm512_store_ps(&C->array[i + j*C->m_pad], _mm512_fmadd_ps(a, b1, c1));
//                 _mm512_store_ps(&C->array[i + 16 + j*C->m_pad], _mm512_fmadd_ps(a, b2, c2));
//             }
//         }
//     }
//     return 0;
// }


// // dot product of two __m256 vectors
// inline float dp_m256(__m256 a, __m256 b) {
//     const int imm8 = 0b11110001;
//     // four high-order bits set to 1 so that all 8 floats are multiplied, 
//     // and 0001 means only keep the result in the loest 32 bits of each 128-bit lane.
//     __m256 c = _mm256_dp_ps(a, b, imm8);
//     c = _mm256_add_ps(c, _mm256_permute2f128_ps(c, c, 1)); // add the two 128-bit lanes
//     // cvtss then add the two 32-bit floats in the 128-bit lane to get the final result.
//     return _mm256_cvtss_f32(c);
// }

// inline float sum_m256(__m256 v) {
//     // dot product of v with a vector of 1.0f is the sum of all elements in v
//     return dp_m256(v, _mm256_set1_ps(1.0f));
// }


// Matrix* optimized_multiply(Matrix* A, Matrix* B) {
//     Matrix* C = createMatrix(A->m, B->n);
//     for (size_t i = 0; i < A->m; i++) {
//         for (size_t j = 0; j < B->n; j+=8) {
//             __m256 sum = _mm256_setzero_ps(); // initialize sum to 0
//             for (size_t k = 0; k < A->n; k++) {
//                 __m256 a = _mm256_set1_ps(A->array[i*A->n + k]); // load A[i][k] into a
//                 __m256 b = _mm256_load_ps(&B->array[k*B->n + j]); // load B[k][j:j+8] into b
//                 sum = _mm256_add_ps(sum, _mm256_mul_ps(a, b)); // multiply and add (SIMD)
//             }
//             if (i < A->m && j < B->n) {
//                 _mm256_storeu_ps(&C->array[i*C->n + j], sum); // store sum in C[i][j:j+8]
//             }
//         }
//     }
//     return C;
// }




// Matrix* transpose(Matrix* matrix) {
//     Matrix* transposed = createMatrix(matrix->n, matrix->m);
//     for (size_t i = 0; i < matrix->m; i++) {
//         for (size_t j = 0; j < matrix->n; j++) {
//             transposed->array[j + i*transposed->m_pad] = matrix->array[i + j*matrix->m_pad];
//         }
//     }
//     return transposed;
// }

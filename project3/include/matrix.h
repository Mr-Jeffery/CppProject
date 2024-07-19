#include <omp.h>
#include <immintrin.h> // for AVX

typedef struct {
    size_t m;
    size_t n;
    float* data;
} matrix_t;

matrix_t* create_matrix(size_t m, size_t n) {
    m = (m+7) & ~7;
    n = (n+7) & ~7;// make sure m and n are multiples of 8
    matrix_t* matrix = (matrix_t*) malloc(sizeof(matrix_t));
    matrix->m = m;
    matrix->n = n;
    matrix->data = (float*) malloc(m*n*sizeof(float));
    return matrix;
}

void free_matrix(matrix_t* matrix) {
    free(matrix->data);
    free(matrix);
}

typedef struct {
    __m256 m0;
    __m256 m1;
    __m256 m2;
    __m256 m3;
} __m256x4;

inline __m256x4 load_1x8(matrix_t* a, size_t i, size_t j) {
    return __mm256_set_ps(a->data[(i+7)*a->n + j], a->data[(i+6)*a->n + j], a->data[(i+5)*a->n + j], a->data[(i+4)*a->n + j], a->data[(i+3)*a->n + j], a->data[(i+2)*a->n + j], a->data[(i+1)*a->n + j], a->data[i*a->n + j]);
}

inline __m256x4 load_4x8(matrix_t* a, size_t i, size_t j) {
    __m256x4 result;
    result.m0 = load_1x8(a, i, j);
    result.m1 = load_1x8(a, i+1, j);
    result.m2 = load_1x8(a, i+2, j);
    result.m3 = load_1x8(a, i+3, j);
    return result;
}

inline void m256_8x8_mul_m256(matrix_t* a, matrix_t* b,matrix_t* result, size_t i, size_t j ){// multiply the result for result[i:i+7][j:j+7]

}

void multiply(matrix_t* a, matrix_t* b, matrix_t* result) {
    for (size_t i = 0; i < a->m; i++) {
        for (size_t k = 0; k < a->n; k++){ //for (int j = 0; j < n; j++) {
            for (size_t j = 0; j < b->n; j++) {// for (int k = 0; k < n; k++){
                result->data[i*result->n + j] += a->data[i*a->n + k] * b->data[k*b->n + j];
            }
                
        }
    }
}



void block_multiply(matrix_t* a, matrix_t* b, matrix_t* result, int block_size) {
    for (size_t i = 0; i < a->m; i += block_size) {
        for (size_t j = 0; j < b->n; j += block_size) {
            for (size_t k = 0; k < a->n; k += block_size) {
                for (size_t ii = i; ii < i + block_size; ii++) {
                    for (size_t jj = j; jj < j + block_size; jj++) {
                        for (size_t kk = k; kk < k + block_size; kk++) {
                            result->data[ii*result->n + jj] += a->data[ii*a->n + kk] * b->data[kk*b->n + jj];
                        }
                    }
                }
            }
        }
    }
}

inline float dp_m256(__m256 a, __m256 b) {
    const int imm8 = 0b11110001;
    // four high-order bits set to 1 so that all 8 floats are multiplied, 
    // and 0001 means only keep the result in the loest 32 bits of each 128-bit lane.
    return _mm256_cvtss_f32(_mm256_dp_ps(a, b, imm8));
}

inline __m256 crs_m256(__m256 a, __m256 b) {
    __m256 a0 = _mm256_permute_ps(a, 0b11011000); // 0, 2, 1, 3
    __m256 b0 = _mm256_permute_ps(b, 0b11011000); // 0, 2, 1, 3
    __m256 a1 = _mm256_permute_ps(a, 0b01001110); // 2, 0, 3, 1
    __m256 b1 = _mm256_permute_ps(b, 0b01001110); // 2, 0, 3, 1
    return _mm256_sub_ps(_mm256_mul_ps(a0, b1), _mm256_mul_ps(a1, b0));
}

void multiply_simd(matrix_t* a, matrix_t* b, matrix_t* result, size_t i, size_t j) {
    for (int i = 0; i < a->m; i++) {
        for (int k = 0; k < a->n; k++){ //for (int j = 0; j < n; j++) {
            #pragma omp parallel for
            for (int j = 0; j < b->n; j++) {// for (int k = 0; k < n; k++){
                result->data[i*result->n + j] += a->data[i*a->n + k] * b->data[k*b->n + j];
            }
        }
    }
}

void block_simd_multiply(matrix_t* a, matrix_t* b, matrix_t* result, int block_size) {
    for (size_t i = 0; i < a->m; i += block_size) {
        for (size_t j = 0; j < b->n; j += block_size) {
            for (size_t k = 0; k < a->n; k += block_size) {
                // for (size_t ii = i; ii < i + block_size; ii++) {
                //     for (size_t jj = j; jj < j + block_size; jj++) {
                //         for (size_t kk = k; kk < k + block_size; kk++) {
                //             result->data[ii*result->n + jj] += a->data[ii*a->n + kk] * b->data[kk*b->n + jj];
                //         }
                //     }
                // }

            }
        }
    }
}


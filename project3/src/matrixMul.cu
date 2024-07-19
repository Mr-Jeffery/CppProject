#include <stdio.h>
#include <sys/time.h>

typedef struct {
    size_t m;           // Number of rows
    size_t n;           // Number of columns
    size_t m_pad;       // Number of rows padded to multiple of 8
    size_t n_pad;       // Number of columns padded to multiple of 8
    float* array;    // Pointer to the matrix elements
    // floats are stored in column-major order
} Matrix;

Matrix* createMatrix(size_t m, size_t n) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    matrix->m = m;
    matrix->n = n;
    matrix->m_pad = (m + 7) & ~7;
    matrix->n_pad = (n + 7) & ~7;
    matrix->array = (float*)aligned_alloc(32, matrix->m_pad * matrix->n_pad * sizeof(float));
    return matrix;
}

void freeMatrix(Matrix* matrix) {
    free(matrix->array);
    free(matrix);
}

void printMatrix(Matrix* matrix) {
    for (size_t i = 0; i < matrix->m_pad; i++) {
        for (size_t j = 0; j < matrix->n_pad; j++) {
            printf("%.2f\t", matrix->array[j*matrix->m_pad + i]);
        }
        printf("\n");
    }
}

__global__ void matrixMulCUDA(float *C, float *A, float *B, int m, int n, int k)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row < m && col < k) {
        float sum = 0;
        for(int i = 0; i < n; i++) {
            sum += A[row * n + i] * B[i * k + col];
        }
        C[row * k + col] = sum;
    }
}

void matMul(const Matrix A, const Matrix B, Matrix C)
{
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((C.m_pad + threadsPerBlock.x - 1) / threadsPerBlock.x, (C.n_pad + threadsPerBlock.y - 1) / threadsPerBlock.y);

    matrixMulCUDA<<<numBlocks, threadsPerBlock>>>(C.array, A.array, B.array, A.m, A.n, B.n);
}

int main() {
    // Define the dimensions of your matrices
    size_t m = 8000, n = 8000, k = 8000;

    // Create your matrices using your createMatrix function
    Matrix* A = createMatrix(m, n);
    Matrix* B = createMatrix(n, k);
    Matrix* C = createMatrix(m, k);

    // Initialize your matrices with data
    for (size_t i = 0; i < A->m; i++) {
        for (size_t j = 0; j < A->n; j++) {
            A->array[j*A->m_pad + i] = i*A->n + j;
        }
    }
    for (size_t i = 0; i < B->m; i++) {
        for (size_t j = 0; j < B->n; j++) {
            B->array[j*B->m_pad + i] = i*B->n + j;
        }
    }
    struct timeval start, end;
    gettimeofday(&start, NULL);
    // Allocate memory for your matrices on the device
    float *d_A, *d_B, *d_C;
    cudaMalloc(&d_A, A->m_pad * A->n_pad * sizeof(float));
    cudaMalloc(&d_B, B->m_pad * B->n_pad * sizeof(float));
    cudaMalloc(&d_C, C->m_pad * C->n_pad * sizeof(float));

    // Copy your matrices from the host to the device
    cudaMemcpy(d_A, A->array, A->m_pad * A->n_pad * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B->array, B->m_pad * B->n_pad * sizeof(float), cudaMemcpyHostToDevice);

    // Perform the matrix multiplication on the device

    matMul(*A, *B, *C);

    // Copy the result matrix from the device back to the host
    cudaMemcpy(C->array, d_C, C->m_pad * C->n_pad * sizeof(float), cudaMemcpyDeviceToHost);
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                    ((end.tv_usec - start.tv_usec)/1000000.0);
    printf("Time spent: %f\n", elapsed);
    // Print the result matrix
    // printMatrix(C);

    // Clean up any allocated memory
    freeMatrix(A);
    freeMatrix(B);
    freeMatrix(C);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}

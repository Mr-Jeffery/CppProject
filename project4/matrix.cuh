#include <cuda_runtime.h>
#include "sharedMatrix.hpp"

template <typename T>
__global__ void matrixMulCUDA(T *C, T *A, T *B, int m, int n, int k)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row < m && col < k) {
        T sum = 0;
        for(int i = 0; i < n; i++) {
            sum += A[row * n + i] * B[i * k + col];
        }
        C[row * k + col] = sum;
    }
}

template <typename T>
void matrixMulWrapper(Matrix<T>& C, const Matrix<T>& A, const Matrix<T>& B)
{
    T *d_A, *d_B, *d_C;
    size_t size_A = A.m * A.n * sizeof(T);
    size_t size_B = B.m * B.n * sizeof(T);
    size_t size_C = C.m * C.n * sizeof(T);

    // Allocate memory on the GPU
    cudaMalloc((void**)&d_A, size_A);
    cudaMalloc((void**)&d_B, size_B);
    cudaMalloc((void**)&d_C, size_C);

    // Copy matrices from the host to the device
    cudaMemcpy(d_A, A.array.get(), size_A, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B.array.get(), size_B, cudaMemcpyHostToDevice);

    // Define the dimensions of the grid and blocks
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((B.n + threadsPerBlock.x - 1) / threadsPerBlock.x, (A.m + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Call the matrix multiplication function
    matrixMulCUDA<<<numBlocks, threadsPerBlock>>>(d_C, d_A, d_B, A.m, A.n, B.n);

    // Copy the result matrix from the device to the host
    cudaMemcpy(C.array.get(), d_C, size_C, cudaMemcpyDeviceToHost);

    // Free the memory allocated on the GPU
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}

template <typename T>
Matrix<T> Matrix<T>::operator*(const Matrix<T>& B) const {
    assert(n == B.m); // Ensure the matrices can be multiplied
    Matrix<T> C(m, B.n);
    matrixMulWrapper(C, *this, B);
    return C;
}

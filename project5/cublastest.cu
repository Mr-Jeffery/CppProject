#include <iostream>
#include <cublas_v2.h>

int main() {
    cublasHandle_t handle;
    float* d_A;
    float* d_B;
    float* d_C;
    int n = 2;

    // Initialize cuBLAS
    cublasCreate(&handle);

    // Allocate device memory
    cudaMalloc((void**)&d_A, n * n * sizeof(float));
    cudaMalloc((void**)&d_B, n * n * sizeof(float));
    cudaMalloc((void**)&d_C, n * n * sizeof(float));

    // Initialize matrices A and B
    float h_A[4] = {1, 2, 3, 4};
    float h_B[4] = {5, 6, 7, 8};

    // Copy matrices A and B to the device
    cudaMemcpy(d_A, h_A, n * n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, n * n * sizeof(float), cudaMemcpyHostToDevice);

    // Perform matrix multiplication C = A * B
    float alpha = 1.0f;
    float beta = 0.0f;
    cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, n, n, n, &alpha, d_A, n, d_B, n, &beta, d_C, n);

    // Copy the result back to host
    float h_C[4];
    cudaMemcpy(h_C, d_C, n * n * sizeof(float), cudaMemcpyDeviceToHost);

    // Print the result
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << h_C[i * n + j] << " ";
        }
        std::cout << "\n";
    }

    // Cleanup
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cublasDestroy(handle);

    return 0;
}

#include <iostream>
#include "sharedMatrix.cu"
#include <sys/time.h>

int main() {
    // Test the default constructor
    Matrix<float> A(8, 8);

    // Test the constructor with an existing array
    float array[9] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    Matrix B(3, 3, array);
    std::cout << "B:\n" << B << "\n";

    // Test the assignment operator with a single value
    A = 1.5f;
    std::cout << "A:\n" << A << "\n";

    Matrix<float> C = A(3,3,6,6);
    C.copy(B*B);
    A(1,1,4,4) += C;
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;
    }
    std::cout << "C:\n" << C << "\n";
    std::cout << "A:\n" << A << "\n";
    std::cout << "A(3,3):\n" << A(3,3) << "\n";

    Matrix<unsigned char> D(3, 3);
    D = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << "D:\n" << D << "\n";
    D(1,1,3,3) = 0;
    std::cout << "D:\n" << D << "\n";
    D += D;
    std::cout << "D:\n" << D << "\n";

    Matrix<double> E(8000, 8000);
    E = 1.0;
    Matrix<double> F(8000, 8000);
    F = 2.0;

    struct timeval start, end;
    gettimeofday(&start, NULL);
    E = E*F;
    gettimeofday(&end, NULL);
    std::cout << "Time taken: " << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6 << "s\n";

    return 0;
}
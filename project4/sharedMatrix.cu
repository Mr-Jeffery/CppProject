#pragma once

#include <stdlib.h>
#include <omp.h>
// #include <immintrin.h> // for AVX
#include <vector>
#include <cassert> // for assert
#include <memory> // for std::shared_ptr

template <typename T>
__global__ void matrixMulCUDA(T *C, T *A, T *B, int m, int n, int k){
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row < m && col < k) {
        T sum = 0;
        for(int i = 0; i < n; i++) {
            sum += B[row * n + i] * A[i * k + col];
        }
        C[row * k + col] = sum;
    }
}

template <typename T>
__global__ void matrixAddCUDA(T *C, T *A, T *B, int m, int n, int k){
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row < m && col < n) {
        C[row * n + col] = A[row * n + col] + B[row * n + col];
    }
}

template <typename T>
__global__ void matrixSubCUDA(T *C, T *A, T *B, int m, int n, int k){
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if(row < m && col < n) {
        C[row * n + col] = A[row * n + col] - B[row * n + col];
    }
}

template <typename T>
void printElement(T element) {
    std::cout << element << "\t";
}

template <>
void printElement<unsigned char>(unsigned char element) {
    std::cout << static_cast<int>(element) << "\t";
}

#define PRINT_AS_INT(x) static_cast<int>(x)
template <typename T>
class Matrix {
public:
    size_t m;           // Number of rows
    size_t n;           // Number of columns
    size_t m_offset;    // Offset for submatrix
    size_t n_offset;    // Offset for submatrix
    size_t m_pad;       // Number of rows padded 
    size_t n_pad;       // Number of columns padded 
    std::shared_ptr<T> array;           // Pointer to the matrix elements
    // elements are stored in column-major order

    // Default constructor
    Matrix(size_t m, size_t n) {
        this->m = m;
        this->n = n;
        this->m_offset = 0;
        this->n_offset = 0;
        this->m_pad = m;
        this->n_pad = n;
        this->array = std::shared_ptr<T>((T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T)));
    }

    // Empty constructor
    Matrix() = default;


    // Assign the matrix with a single value
    Matrix& operator=(const T& value) {
        assert(this->m * this->n > 0 && "Error: Matrix dimensions are not set");
        for (size_t j = 0; j < this->n; j++) {
            for (size_t i = 0; i < this->m; i++) {
                (*this)(i,j) = value;
            }
        }
        return *this;
    }

    // Assign the matrix with a list of values
    Matrix& operator=(std::initializer_list<T> list) {
        assert(this->m * this->n == list.size() && "Error: Matrix dimensions do not match initializer list size");
        auto it = list.begin();
        for (size_t j = 0; j < this->n; j++) {
            for (size_t i = 0; i < this->m; i++) {
                (*this)(i,j) = *it++;
            }
        }
        return *this;
    }

    // Assignment operator that takes a vector to T
    Matrix& operator=(std::vector<T> vec) {
        assert(this->m * this->n > 0 && "Error: Matrix dimensions are not set");
        assert(vec.size() == this->m * this->n && "Error: The number of values does not match the size of the matrix");
        std::copy(vec.begin(), vec.end(), this->array.get());
        return *this;
    }

    // Assignment operator
    Matrix& operator=(const Matrix& other) {
        this->m = other.m;
        this->n = other.n;
        this->m_offset = other.m_offset;
        this->n_offset = other.n_offset;
        this->m_pad = other.m_pad;
        this->n_pad = other.n_pad;
        this->array = other.array;
        return *this;
    }

    // Hard Copy matrix
    void copy(const Matrix& other) {
        assert(this->m == other.m && this->n == other.n);
        # pragma omp parallel for
        for (size_t j = 0; j < this->n; j++) {
            for (size_t i = 0; i < this->m; i++) {
                (*this)(i,j) = other(i,j);
            }
        }
    }

    Matrix(size_t m, size_t n, T* array) : 
        m(m), 
        n(n), 
        m_offset(0),
        n_offset(0),
        m_pad(m),
        n_pad(n)
    {
        this->array = std::shared_ptr<T>((T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T)));
        # pragma omp parallel for
        for (size_t j = 0; j < this->n; j++) {
            for (size_t i = 0; i < this->m; i++) {
                (*this)(i,j) = array[i + j*m];
            }
        }
    }

    // Copy constructor
    Matrix(const Matrix& other) : 
        m(other.m), 
        n(other.n), 
        m_offset(other.m_offset),
        n_offset(other.n_offset),
        m_pad(other.m_pad), 
        n_pad(other.n_pad)
    {
        this->array = other.array;
    }

    // Region of interest (ROI) operator
    Matrix operator()(size_t m_start, size_t n_start, size_t m_end, size_t n_end) {
        assert(m_start < m_end && m_end <= this->m);
        assert(n_start < n_end && n_end <= this->n);
        Matrix roi;
        roi.m = m_end - m_start;
        roi.n = n_end - n_start;
        roi.m_offset = m_start;
        roi.n_offset = n_start;
        roi.m_pad = this->m_pad;
        roi.n_pad = this->n_pad;
        roi.array = this->array;
        return roi;
    }

    // Reload the [] for submatrix
    const T operator[](size_t possition) const {
        return array.get()[(possition % m + m_offset) + (possition / m + n_offset) * m_pad];
    }

    // get value of the matrix
    const T& operator()(size_t m_index, size_t n_index) const {
        assert(m_index < m && n_index < n);
        return array.get()[(m_index + m_offset) + (n_index + n_offset) * m_pad];
    }

    T& operator()(size_t m_index, size_t n_index) {
        assert(m_index < m && n_index < n);
        return array.get()[(m_index + m_offset) + (n_index + n_offset) * m_pad];
    }

    // Get matrix from cin
    Matrix& operator>>(std::istream& is) {
        std::cout << "Enter the matrix size (m n36): ";
        is >> m >> n;
        this->m = m;
        this->n = n;
        this->m_offset = 0;
        this->n_offset = 0;
        this->m_pad = m;
        this->n_pad = n;
        this->array = std::shared_ptr<T>((T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T)));
        for (size_t j = 0; j < n; j++) {
            for (size_t i = 0; i < m; i++) {
                is >> (*this)(i,j);
            }
        }
        return *this;
    }

    // Print the matrix
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
        for (size_t j = 0; j < matrix.m; j++) {
            for (size_t i = 0; i < matrix.n; i++) {
                printElement(matrix(i,j));
            }
            os << "\n";
        }
        return os;
    }
    
    // Matrix multiplication
    Matrix<T> operator*(const Matrix<T>& B) {
        assert(n == B.m); // Ensure the matrices can be multiplied
        Matrix<T> C(m, B.n);
        matrixOpWrapper(C, *this, B, matrixMulCUDA<T>);
        return C;
    }

    // Matrix addition
    Matrix<T> operator+(const Matrix<T>& B) {
        assert(m == B.m && n == B.n); // Ensure the matrices have the same dimensions
        Matrix<T> C(m, n);
        // # pragma omp parallel for
        // for (size_t i = 0; i < m; i++) {
        //     for (size_t j = 0; j < n; j++) {
        //         C(i,j) = (*this)(i,j) + B(i,j);
        //     }
        // }
        matrixOpWrapper(C, *this, B, matrixAddCUDA<T>);
        return C;
    }

    // Matrix self-addition
    Matrix<T>& operator+=(const Matrix<T>& B) {
        assert(m == B.m && n == B.n); // Ensure the matrices have the same dimensions
        matrixOpWrapper(*this, *this, B, matrixAddCUDA<T>);
        return *this;
    }

    // Matrix subtraction
    Matrix<T> operator-(const Matrix<T>& B) {
        assert(m == B.m && n == B.n); // Ensure the matrices have the same dimensions
        Matrix<T> C(m, n);
        matrixOpWrapper(C, *this, B, matrixSubCUDA<T>);
        return C;
    }

    // Matrix self-subtraction
    Matrix<T>& operator-=(const Matrix<T>& B) {
        assert(m == B.m && n == B.n); // Ensure the matrices have the same dimensions
        matrixOpWrapper(*this, *this, B, matrixSubCUDA<T>);
        return *this;
    }

    // Matrix equality
    bool operator==(const Matrix<T>& B) {
        if (m != B.m || n != B.n) {
            return false;
        }
        if (this->array == B.array) {
            return true;
        }
        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                if ((*this)(i,j) != B(i,j)) {
                    return false;
                }
            }
        }
        return true;
    }
};


template <typename T, typename F>
void matrixOpWrapper(Matrix<T>& C, const Matrix<T>& A, const Matrix<T>& B, F f){
    T *d_A, *d_B, *d_C;
    size_t size_A = A.m * A.n * sizeof(T);
    size_t size_B = B.m * B.n * sizeof(T);
    size_t size_C = C.m * C.n * sizeof(T);

    // Allocate memory on the GPU
    // cudaMalloc((void**)&d_A, size_A);
    // cudaMalloc((void**)&d_B, size_B);
    // cudaMalloc((void**)&d_C, size_C);
    size_t pitch_A, pitch_B, pitch_C;

    cudaMallocPitch((void**)&d_A, &pitch_A, A.m * sizeof(T), A.n);
    cudaMallocPitch((void**)&d_B, &pitch_B, B.m * sizeof(T), B.n);
    cudaMallocPitch((void**)&d_C, &pitch_C, C.m * sizeof(T), C.n);

    // Copy matrices from the host to the device
    // cudaMemcpy(d_A, A.array.get(), size_A, cudaMemcpyHostToDevice);
    // cudaMemcpy(d_B, B.array.get(), size_B, cudaMemcpyHostToDevice);
    cudaMemcpy2D(d_A, A.m * sizeof(T), A.array.get() + A.m_offset + A.n_offset * A.m_pad, A.m_pad * sizeof(T), A.m * sizeof(T), A.n, cudaMemcpyHostToDevice);
    cudaMemcpy2D(d_B, B.m * sizeof(T), B.array.get() + B.m_offset + B.n_offset * B.m_pad, B.m_pad * sizeof(T), B.m * sizeof(T), B.n, cudaMemcpyHostToDevice);

    // Define the dimensions of the grid and blocks
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((B.n + threadsPerBlock.x - 1) / threadsPerBlock.x, (A.m + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Call the matrix operation function
    f<<<numBlocks, threadsPerBlock>>>(d_C, d_A, d_B, A.m, A.n, B.n);

    // Copy the result matrix from the device to the host
    // cudaMemcpy(C.array.get(), d_C, size_C, cudaMemcpyDeviceToHost);
    cudaMemcpy2D(C.array.get() + C.m_offset + C.n_offset * C.m_pad, C.m_pad * sizeof(T), d_C, C.m * sizeof(T), C.m * sizeof(T), C.n, cudaMemcpyDeviceToHost);
    // Free the memory allocated on the GPU
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}
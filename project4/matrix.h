#include <stdlib.h>
#include <omp.h>
#include <immintrin.h> // for AVX
#include <cassert> // for assert
#include <memory> // for std::shared_ptr

#define BLOCK_SIZE 8
template <typename T>
class Matrix {
public:
    size_t m;           // Number of rows
    size_t n;           // Number of columns
    size_t m_pad;       // Number of rows padded to multiple of BLOCK_SIZE
    size_t n_pad;       // Number of columns padded to multiple of BLOCK_SIZE
    bool owns_memory;   // Whether the matrix owns the memory
    T* array;           // Pointer to the matrix elements
    // std::shared_ptr<T> array;           // Pointer to the matrix elements
    // elements are stored in column-major order

    // Default constructor
    Matrix(size_t m, size_t n) {
        this->m = m;
        this->n = n;
        this->m_pad = (m + BLOCK_SIZE-1) & ~(BLOCK_SIZE-1);
        this->n_pad = (n + BLOCK_SIZE-1) & ~(BLOCK_SIZE-1);
        this->array = (T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T));
    }

    // Empty constructor
    Matrix() {
        this->m = 0;
        this->n = 0;
        this->m_pad = 0;
        this->n_pad = 0;
        this->array = nullptr;
    }

    // Destructor
    ~Matrix() {
        if (owns_memory && array != nullptr) {
            free(array);
            // array = nullptr;
        }
    }

    // Initialize the matrix with a single value
    Matrix& operator=(const T& value) {
        std::fill(this->array, this->array + this->m_pad * this->n_pad, value);
        return *this;
    }

    // Initialize the matrix with a list of values
    Matrix& operator=(std::initializer_list<T> list) {
        assert(list.size() == this->m * this->n && "Error: The number of values does not match the size of the matrix");
        auto it = list.begin();
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                this->array[i + j*this->m_pad] = *it++;
            }
        }
        return *this;
    }

    // Copy matrix
    Matrix& operator=(const Matrix& other) {
        if (this->m != other.m || this->n != other.n) {
            if (owns_memory && array != nullptr) {
                free(array);
            }
            this->m = other.m;
            this->n = other.n;
            this->m_pad = other.m_pad;
            this->n_pad = other.n_pad;
            this->array = (T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T));
        }
        // std::copy(other.array, other.array + this->m_pad * this->n_pad, this->array);
        # pragma omp parallel for
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                this->array[i + j*this->m_pad] = other.array[i + j*other.m_pad];
            }
        }
        return *this;
    }

    Matrix(size_t m, size_t n, T* array) : 
        m(m), 
        n(n), 
        m_pad((m + 7) & ~7ULL), 
        n_pad((n + 7) & ~7ULL) 
    {
        this->array = (T*)aligned_alloc(64, this->m_pad * this->n_pad * sizeof(T));
        std::fill(this->array, this->array + this->m_pad * this->n_pad, 0);
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                this->array[i + j*this->m_pad] = array[i*this->n + j];
            }
        }
    }
    // Copy constructor
    Matrix(const Matrix& other) : 
        m(other.m), 
        n(other.n), 
        m_pad(other.m_pad), 
        n_pad(other.n_pad),
        owns_memory(true) 
    {
        array = (T*)aligned_alloc(64, m_pad * n_pad * sizeof(T));
        std::copy(other.array, other.array + m_pad * n_pad, array);
    }

    // Region of interest (ROI) operator
    Matrix operator()(size_t m_start, size_t n_start, size_t m_end, size_t n_end) {
        assert(m_start < m_end && m_end <= this->m);
        assert(n_start < n_end && n_end <= this->n);
        Matrix roi;
        roi.m = m_end - m_start;
        roi.n = n_end - n_start;
        roi.m_pad = this->m_pad;
        roi.n_pad = this->n_pad;
        roi.owns_memory = false;// The ROI does not own the memory, but only use a copy of it
        roi.array = this->array + m_start + n_start*this->m_pad;
        return roi;
    }

    // Print the matrix
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
        for (size_t i = 0; i < matrix.m; i++) {
            for (size_t j = 0; j < matrix.n; j++) {
                os << matrix.array[j*matrix.m_pad + i] << "\t";
            }
            os << "\n";
        }
        return os;
    }

    // Matrix multiplication
    Matrix<T> operator*(const Matrix<T>& B) {
        assert(this->m == B.n && "Error: Matrix dimensions do not match");
        Matrix<T> C(this->m, B.n);
        # pragma omp parallel for
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < B.n; j++) {
                for (size_t k = 0; k < this->n; k++) {
                    C.array[i + j*C.m_pad] += this->array[k + j*this->m_pad] * B.array[i + k*B.m_pad];
                }
            }
        }
        return C;
    }

    // Matrix addition
    Matrix<T> operator+(const Matrix<T>& B) {
        assert(this->m == B.m && this->n == B.n && "Error: Matrix dimensions do not match");
        Matrix<T> C(this->m, this->n);
        # pragma omp parallel for
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                C.array[i + j*C.m_pad] = this->array[i + j*this->m_pad] + B.array[i + j*B.m_pad];
            }
        }
        return C;
    }

    // Matrix subtraction
    Matrix<T> operator-(const Matrix<T>& B) {
        assert(this->m == B.m && this->n == B.n && "Error: Matrix dimensions do not match");
        Matrix<T> C(this->m, this->n);
        # pragma omp parallel for
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                C.array[i + j*C.m_pad] = this->array[i + j*this->m_pad] - B.array[i + j*B.m_pad];
            }
        }
        return C;
    }

    // Matrix equality
    bool operator==(const Matrix<T>& B) {
        if (this->array == B.array) {
            return true;
        }
        if (this->m != B.m || this->n != B.n) {
            return false;
        }
        for (size_t i = 0; i < this->m; i++) {
            for (size_t j = 0; j < this->n; j++) {
                if (this->array[i + j*this->m_pad] != B.array[i + j*B.m_pad]) {
                    return false;
                }
            }
        }
        return true;
    }
};


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






// Matrix* transpose(Matrix* matrix) {
//     Matrix* transposed = createMatrix(matrix->n, matrix->m);
//     for (size_t i = 0; i < matrix->m; i++) {
//         for (size_t j = 0; j < matrix->n; j++) {
//             transposed->array[j + i*transposed->m_pad] = matrix->array[i + j*matrix->m_pad];
//         }
//     }
//     return transposed;
// }

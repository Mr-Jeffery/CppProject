#include <cmath>
#include <cstddef>

template <typename T>
bool vabs(T* p, size_t n) {
    if (p == nullptr) return false;
    for (size_t i = 0; i < n; ++i) {
        p[i] = std::abs(p[i]);
    }
    return true;
}

// Explicit instantiation
template bool vabs<int>(int* p, size_t n);
template bool vabs<float>(float* p, size_t n);
template bool vabs<double>(double* p, size_t n);
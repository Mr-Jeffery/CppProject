[ref1](https://codereview.stackexchange.com/questions/177616/avx-simd-in-matrix-multiplication)

```c
size_t ib = std::min(256, (int)N);
size_t jb = std::min(512, (int)N);
size_t kb = std::min(16, (int)N);

int *mat2 = (int*)_aligned_malloc(N * N * sizeof(int), 32);
size_t m2idx = 0;
for (size_t jj = 0; jj < N; jj += jb)
{
    for (size_t kk = 0; kk < N; kk += kb)
    {           
        for (size_t j = jj; j < jj + jb; j += 16)
        {
            for (size_t k = kk; k < kk + kb; k++)
            {
                auto vecA_mat2 = _mm256_loadu_si256((__m256i*)&mat2in[k][j]);
                auto vecB_mat2 = _mm256_loadu_si256((__m256i*)&mat2in[k][j + 8]);
                _mm256_storeu_si256((__m256i*)&mat2[m2idx], vecA_mat2);
                _mm256_storeu_si256((__m256i*)&mat2[m2idx + 8], vecB_mat2);
                m2idx += 16;
            }
        }
    }
}


for (size_t ii = 0; ii < N; ii += ib) {
    for (size_t jj = 0; jj < N; jj += jb) {
        for (size_t kk = 0; kk < N; kk += kb) {
            for (size_t i = ii; i < ii + ib; i += 2) {
                for (size_t j = jj; j < jj + jb; j += 16) {
                    size_t m2idx = (j - jj) * kb + kk * jb + jj * N;
                    __m256i sumA_1, sumB_1, sumA_2, sumB_2;
                    if (kk == 0) {
                        sumA_1 = sumB_1 = sumA_2 = sumB_2 = _mm256_setzero_si256();
                    }
                    else {
                        sumA_1 = _mm256_load_si256((__m256i*)&result[i][j]);
                        sumB_1 = _mm256_load_si256((__m256i*)&result[i][j + 8]);
                        sumA_2 = _mm256_load_si256((__m256i*)&result[i + 1][j]);
                        sumB_2 = _mm256_load_si256((__m256i*)&result[i + 1][j + 8]);
                    }

                    for (size_t k = kk; k < kk + kb; k++) {
                        auto bc_mat1_1 = _mm256_set1_epi32(mat1[i][k]);
                        auto vecA_mat2 = _mm256_loadu_si256((__m256i*)(mat2 + m2idx));
                        auto vecB_mat2 = _mm256_loadu_si256((__m256i*)(mat2 + m2idx + 8));
                        sumA_1 = _mm256_add_epi32(sumA_1, _mm256_mullo_epi32(bc_mat1_1, vecA_mat2));
                        sumB_1 = _mm256_add_epi32(sumB_1, _mm256_mullo_epi32(bc_mat1_1, vecB_mat2));
                        auto bc_mat1_2 = _mm256_set1_epi32(mat1[i + 1][k]);
                        sumA_2 = _mm256_add_epi32(sumA_2, _mm256_mullo_epi32(bc_mat1_2, vecA_mat2));
                        sumB_2 = _mm256_add_epi32(sumB_2, _mm256_mullo_epi32(bc_mat1_2, vecB_mat2));
                        m2idx += 16;
                    }
                    _mm256_storeu_si256((__m256i*)&result[i][j], sumA_1);
                    _mm256_storeu_si256((__m256i*)&result[i][j + 8], sumB_1);
                    _mm256_storeu_si256((__m256i*)&result[i + 1][j], sumA_2);
                    _mm256_storeu_si256((__m256i*)&result[i + 1][j + 8], sumB_2);
                }
            }
        }
    }
}

_aligned_free(mat2);
```
#include <stdio.h>
// #include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <immintrin.h> // for AVX
// all matrix are 1d array to maximize performance

static void multiply(int n, float a[n][n], float b[n][n], float result[n][n] ) {
    for (int i = 0; i < n; i++) {
        // #pragma omp parallel for
        for (int k = 0; k < n; k++){//for (int j = 0; j < n; j++) {
            for (int j = 0; j < n; j++) {//for (int k = 0; k < n; k++){
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

static void block_multiply(int n, float a[n][n], float b[n][n], float result[n][n], int block_size) {
    // #pragma omp parallel for
    for (int i = 0; i < n; i += block_size) {
        for (int j = 0; j < n; j += block_size) {
            for (int k = 0; k < n; k += block_size) {
                for (int ii = i; ii < i + block_size; ii++) {
                    for (int jj = j; jj < j + block_size; jj++) {
                        for (int kk = k; kk < k + block_size; kk++) {
                            result[ii][jj] += a[ii][kk] * b[kk][jj];
                        }
                    }
                }
            }
        }
    }
}



// TODO
// Strassen's algorithm for matrix multiplication
// void mul2(float[n][n] a, float[n][n] b, float[n][n] result, int n) {
//     for (int i = 0; i < n; i += 2) {
//         for (int j = 0; j < n; j += 2) {
            
//         }
//     }
// }

int main(int argc, char* argv[]) {
    int n = 2;
    char* matpath = NULL;
    float maximum = 1.0f;
    float minimum = 0.0f;
    int t = 1;
    int block_size = 16;
    for (int i = 1; i < argc; i++) {
        /* Check for a switch (leading "-"). */
        if (argv[i][0] == '-') {
            char *arg = argv[i] + 1;
            //help on flags
            if (strcmp(arg, "h") == 0||strcmp(arg, "-help") == 0) {
                printf("Usage: matmul [flags]\n");
                printf("Flags:\n");
                printf("  -h, --help: show this help message\n");
                printf("  -n: specify the size of the matrix\n");
                printf("  -p, --path: specify the file containing the matrices\n");
                printf("  -max: specify the maximum value for the matrix\n");
                printf("  -min: specify the minimum value for the matrix\n");
                printf("  -t: specify the number of times to multiply the matrices\n");
                return 0;
            }
            // specify n
            else if (strcmp(arg,  "n") == 0) {
                if (i + 1 < argc) {
                    i++;
                    n = atoi(argv[i]);
                    if (n <= 0) {
                        printf("Error: n must be a positive integer\n");
                        return 1;
                    }
                } else {
                    printf("Error: n must be specified\n");
                    return 1;
                }
            
            } else if (strcmp(arg,  "b") == 0||strcmp(arg, "-block_size") == 0)
            {
                i++;
                block_size = (size_t)atoi(argv[i]);
            } else 
            if (strcmp(arg, "p") == 0||strcmp(arg, "-path") == 0) {// specify matrix file
                if (i + 1 < argc) {
                    i++;
                    matpath = argv[i];
                    printf("file for matrices: %s\n", matpath);
                } else {
                    printf("Error: file for martrices must be specified\n");
                    return 1;
                }
            // specify matrix generation method
            } 
            else if (strcmp(arg, "max")==0) {
                if (i + 1 >= argc) {
                    printf("Error: max must be specified\n");
                    return 1;
                }
                i++;
                maximum = atof(argv[i]);
            } 
            else if (strcmp(arg, "min")==0) {
                if (i + 1 >= argc) {
                    printf("Error: min must be specified\n");
                    return 1;
                }
                i++;
                minimum = atof(argv[i]);
            }  else if (strcmp(arg, "t")==0) {
                i++;
                t = atoi(argv[i]);
            } else {
                printf("Error: Unknown Flag %s\n", arg);
                return 1;
            }


        }
    }

    printf("n: %d\n", n);
    float (*a)[n] = _aligned_malloc(sizeof(float[n][n]));
    float (*b)[n] = _aligned_malloc(sizeof(float[n][n]));
    float (*result)[n] = _aligned_malloc(sizeof(float[n][n]));

    // Check if memory allocation was successful
    if (a == NULL || b == NULL || result == NULL) {
        printf("Error: Could not allocate memory for matrices\n");
        return 1;
    }

    if (matpath == NULL) {
        // generate matrix values
        // printf("path for matrices not specified, generating new matrices with rand()\n");
        // printf("maximum: %f, minimum: %f\n", maximum, minimum);
        srand(0);
        // for (int i = 0; i < n*n; i++){
        //     a[i] = ((float)rand()/(float)(RAND_MAX)) * (maximum - minimum) + minimum;
        //     b[i] = ((float)rand()/(float)(RAND_MAX)) * (maximum - minimum) + minimum;
        // }
        for (int i = 0; i < n; i++){
            for (int j = 0; j < n; j++){
                a[i][j] = ((float)rand()/(float)(RAND_MAX)) * (maximum - minimum) + minimum;
                b[i][j] = ((float)rand()/(float)(RAND_MAX)) * (maximum - minimum) + minimum;
            }
        }

        // write to file
        FILE* file = fopen("matrices.txt", "w");
        // printf("writing to matrices.txt\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(file, "%a ", a[i][j]);
            }
            fprintf(file, "\n");
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(file, "%a ", b[i][j]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }

    struct timeval start, end;
    printf("Starting multiplication, t = %d\n", (int)t);
    gettimeofday(&start, NULL);
    for (int i = 0; i < t; i++){
        for ( int j = 0; j < n * n; j++) {
            result[j / n][j % n] = 0;
        }
        // multiply(n, a, b, result);
        block_multiply(n, a, b, result, block_size);
    }
    gettimeofday(&end, NULL);

    double time_spent = (double)(end.tv_usec - start.tv_usec) / 1000000.0 + (double)(end.tv_sec - start.tv_sec);
    printf("Time spent: %f\n", time_spent);

    free(a);
    free(b);
    free(result);

    return 0;
}
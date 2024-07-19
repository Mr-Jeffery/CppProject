
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <time.h>

const int N = 1000; // Size of the matrices
const int NUM_THREADS = 1; // Number of threads to use

// Function to perform matrix multiplication for a given range of rows
void multiplyRows(const std::vector<float>& A, const std::vector<float>& B, std::vector<float>& C, int startRow, int endRow) {
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

int main() {
    // Initialize matrices A, B, and C with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    std::vector<float> A(N * N);
    std::vector<float> B(N * N);
    std::vector<float> C(N * N, 0.0f);

    for (int i = 0; i < N * N; i++) {
        A[i] = dis(gen);
        B[i] = dis(gen);
    }

    // Start the total timer
    clock_t totalStart = clock();

    // Run the test 10 times
    for (int test = 0; test < 10; test++) {
        // Reset matrix C
        std::fill(C.begin(), C.end(), 0.0f);

        // Create threads
        std::vector<std::thread> threads;
        int rowsPerThread = N / NUM_THREADS;
        int startRow = 0;
        int endRow = rowsPerThread;

        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back(multiplyRows, std::ref(A), std::ref(B), std::ref(C), startRow, endRow);
            startRow = endRow;
            endRow += rowsPerThread;
        }

        // Wait for all threads to finish
        for (auto& thread : threads) {
            thread.join();
        }
    }

    // Stop the total timer
    clock_t totalEnd = clock();

    // Calculate the total time
    double totalTime = (double)(totalEnd - totalStart) / CLOCKS_PER_SEC;

    // Print the total time taken
    std::cout << "Total time taken: " << totalTime << " seconds" << std::endl;

    return 0;
}
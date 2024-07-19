#include <iostream>
#include "matrix.h"

int main() {
    // Test the default constructor
    Matrix<float> A(1000, 1000);
    // std::cout << "A:\n" << A << "\n";

    // Test the constructor with an existing array
    float array[9] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    Matrix B(3, 3, array);
    std::cout << "B:\n" << B << "\n";

    // Test the assignment operator with a single value
    A = 2.0f;
    std::cout << "A after assignment with a single value:\n" << A << "\n";

    // Test the assignment operator with a list of values
    A = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << "A after assignment with a list of values:\n" << A << "\n";

    // Test the copy constructor
    Matrix C = B;
    std::cout << "C after copy from B:\n" << C << "\n";

    // Test the ROI operator
    Matrix D1 = B(0, 0, 2, 2);
    std::cout << "D1 after ROI (0, 0, 2, 2) from B:\n" << D1 << "\n";
    Matrix D2 = D1(1, 1, 2, 2);
    std::cout << "D2 after nested ROI (1, 1, 2, 2) from D:\n" << D2 << "\n";

    // Test the multiplication operator
    Matrix E = A * B;
    std::cout << "E after multiplication of A and B:\n" << E << "\n";

    D1 = D1 * D1;
    std::cout << "D1 = D1 * D1:\n" << D1 << "\n";

    std::cout << "B:\n" << B << "\n";

    B = A * A;
    // std::cout << "B:\n" << B << "\n";
    

    return 0;
}
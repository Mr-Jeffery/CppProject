#include <iostream>
#include "stuinfo.hpp"

int main(){
    int n;
    std::cout << "Enter the number of students: ";
    std::cin >> n;
    stuinfo stu[n];
    inputstu(stu, n);
    showstu(stu, n);
    return 0;
}
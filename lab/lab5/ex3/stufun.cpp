#include <iostream>
#include "stuinfo.hpp"

void inputstu(stuinfo stu[], int n) {
    for(int i = 0; i < n; i++) {
        std::cout << "Enter name for student " << i+1 << ": ";
        std::cin >> stu[i].name;
        stu[i].ave = 0;
        for(int j = 0; j < 3; j++) {
            std::cout << "Enter score " << j+1 << " for student " << i+1 << ": ";
            std::cin >> stu[i].score[j];
            stu[i].ave += stu[i].score[j];
        }
        stu[i].ave /= 3.0;
    }
}

void showstu(const stuinfo stu[], int n) {
    for(int i = 0; i < n; i++) {
        std::cout << "Student " << i+1 << ": " << stu[i].name << "\n";
        for(int j = 0; j < 3; j++) {
            std::cout << "Score " << j+1 << ": " << stu[i].score[j] << "\n";
        }
        std::cout << "Average: " << stu[i].ave << "\n";
    }
}
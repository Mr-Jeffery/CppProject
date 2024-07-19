#include <iostream>
#include <string>

template <typename T>
int Compare(const T &a, const T &b) {
    if (a > b) return 1;
    else if (a < b) return -1;
    else return 0;
}

// Explicit specialization for struct studentInfo
struct studentInfo {
    std::string name;
    int age;
};

bool operator>(const studentInfo &a, const studentInfo &b) {
    return a.age > b.age;
}

bool operator<(const studentInfo &a, const studentInfo &b) {
    return a.age < b.age;
}

int main() {
    std::cout << "Compare two integers: " << Compare(5, 3) << std::endl; 
    std::cout << "Compare two floats: " << Compare(5.6f, 5.6f) << std::endl; 
    std::cout << "Compare two characters: " << Compare('a', 'b') << std::endl;

    studentInfo alice{"Alice", 25};
    studentInfo bob{"Bob", 30};
    
  	std::cout << "Compare two structs: " << Compare(alice, bob) << std::endl;

  	return 0;
}

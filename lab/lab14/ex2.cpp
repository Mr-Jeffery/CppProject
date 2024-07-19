#include <iostream>
#include <cmath>

class OutOfRangeException : public std::exception
{
public:
    const char* what() const throw()
    {
        return "OutOfRangeException: Marks are out of range (0-100)";
    }
};

float calculateAverage(float mark1, float mark2, float mark3, float mark4)
{
    float marks[] = {mark1, mark2, mark3, mark4};
    int size = sizeof(marks) / sizeof(marks[0]);

    for (int i = 0; i < size; i++)
    {
        if (marks[i] < 0 || marks[i] > 100)
        {
            throw OutOfRangeException();
        }
    }

    float sum = mark1 + mark2 + mark3 + mark4;
    return sum / size;
}

int main()
{
    char choice = 'y';
    while (choice == 'y')
    {
        try
        {
            float mark1, mark2, mark3, mark4;
            std::cout << "Enter four marks: ";
            std::cin >> mark1 >> mark2 >> mark3 >> mark4;
            
            float average = calculateAverage(mark1, mark2, mark3, mark4);
            std::cout << "Average: " << average << std::endl;
        }
        catch (const OutOfRangeException& e)
        {
            std::cout << e.what() << std::endl;
        }
        std::cout << "Do you want to continue? (y/n): ";
        std::cin >> choice;
    }
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
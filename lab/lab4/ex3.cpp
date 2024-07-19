#include <iostream>

enum Day { Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };
enum Weather { Sunny, Rainy, Cloudy, Snowy };

struct DayInfo {
    Day day;
    Weather weather;
};

bool canTravel(DayInfo dayInfo) {
    return (dayInfo.day == Sunday || dayInfo.day == Saturday) && dayInfo.weather == Sunny;
}

int main() {
    DayInfo today = { Sunday, Sunny };
    if (canTravel(today)) {
        std::cout << "You can travel today!\n";
    } else {
        std::cout << "You cannot travel today.\n";
    }
    return 0;
}
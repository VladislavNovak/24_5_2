#include <windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <limits> // numeric_limits
#include <iomanip>
#include <algorithm>


using std::vector;
using std::cout;
using std::endl;

void reloadStream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

time_t getUserTime(char timeType, time_t basisTime) {
    std::string fmtDate = { '%', timeType };

    std::tm *localTime = localtime(&basisTime);
    time_t result;

    while(true) {
        std::cin >> std::get_time(localTime, fmtDate.c_str());

        if (std::cin.fail()) {
            std::cout << "Неверный формат. Попробуйте снова: ";
            reloadStream();
            continue;
        }

        result = mktime(localTime);
        if (result < 0) {
            std::cout << "Дата должна быть не ранее January 1, 1970. Попробуйте снова: ";
            reloadStream();
            continue;
        }

        reloadStream();
        return mktime(localTime);
    }
}

// в format нужно передать корректную для std::get_time строку типа 'YYYY/mm/dd' или 'HH:MM:SS'
time_t getTimeByFormat(const std::string &format, char delim = '/') {
    vector<std::string> parts;
    std::stringstream ss(format);
    std::string temp;

    while(std::getline(ss, temp, delim)) parts.emplace_back(temp);

    time_t date = time(nullptr);

    for (const auto &dateType : parts) {
        cout << "Введите " << dateType << ": ";
        date = getUserTime(dateType[0], date);
    }

    return date;
}

// сравнивает даты исходя из положения compareDate относительно baseDate
int compareDays(const time_t baseDate, const time_t comparedDate) {
    std::tm base = *localtime(&baseDate);
    std::tm compared = *localtime(&comparedDate);

    if (compared.tm_mon == base.tm_mon) {
        if (compared.tm_mday == base.tm_mday) return 0;
        return (compared.tm_mday < base.tm_mday) ? -1 : 1;
    }
    return (compared.tm_mon < base.tm_mon) ? -1 : 1;
}

bool compareToSortByDay (const time_t baseDate, const time_t comparedDate) {
    std::tm base = *localtime(&baseDate);
    std::tm compared = *localtime(&comparedDate);

    return (compared.tm_mon == base.tm_mon) ? compared.tm_mday >= base.tm_mday : compared.tm_mon >= base.tm_mon;
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    vector<time_t> listOfBirthdays = {};

    vector<time_t> listOfPast = {};
    vector<time_t> listOfToday = {};
    vector<time_t> listOfNearby = {};

    listOfBirthdays.emplace_back(getTimeByFormat("YYYY/mm/dd"));
    listOfBirthdays.emplace_back(getTimeByFormat("YYYY/mm/dd"));
    listOfBirthdays.emplace_back(getTimeByFormat("YYYY/mm/dd"));

    if (listOfBirthdays.size() > 1) {
        std::sort(std::begin(listOfBirthdays), std::end(listOfBirthdays), compareToSortByDay);
    }

    for (const auto &item : listOfBirthdays) {
        std::tm* localItem = localtime(&item);
        cout << asctime(localItem) << endl;
    }
}

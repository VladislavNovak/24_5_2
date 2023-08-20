#include <windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <limits> // numeric_limits
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>

using std::string;
using std::vector;
using std::cout;
using std::endl;

std::string getTrimmedString(std::string str, std::string const &whiteSpaces = " \r\n\t\v\f") {
    auto start = str.find_first_not_of(whiteSpaces);
    str.erase(0, start);
    auto end = str.find_last_not_of(whiteSpaces);
    str.erase(end + 1);

    return str;
}

std::string putLineString(const std::string &msg) {
    while (true) {
        std::string userLineString;
        printf("%s: ", msg.c_str());
        std::getline(std::cin, userLineString);

        userLineString = getTrimmedString(userLineString);
        if (userLineString.empty()) {
            std::cout << "Строка не может быть пустой. Попробуйте снова!" << std::endl;
            continue;
        }

        return userLineString;
    }
}

void reloadStream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Позволяет получить того типа времени, который передан в timeType (любой один из разрешенных для get_time)
// Нет ограничения по датам будущего
time_t putTime(char timeType, time_t basisTime) {
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
time_t putTimeByFormat(const std::string &format, char delim = '/') {
    vector<std::string> parts;
    std::stringstream ss(format);
    std::string temp;

    while(std::getline(ss, temp, delim)) parts.emplace_back(temp);

    time_t date = time(nullptr);

    for (const auto &dateType : parts) {
        cout << "Введите " << dateType << ": ";
        date = putTime(dateType[0], date);
    }

    return date;
}

// Внимание: нужно ещё учесть - является ли високосным текущий год. Здесь этого нет
int extractDayOfYearFromDate(time_t date) {
    std::tm* local = localtime(&date);
    int year = local->tm_year + 1900;
    bool isLeapYear = ((year % 400 == 0 || year % 100 != 0) && year % 4 == 0);

    return (local->tm_yday + (isLeapYear ? -1 : 0));
}

// сравнивает даты исходя из положения compareDate относительно baseDate
int getRelativeToBaseDay(const time_t baseDate, const time_t comparedDate) {
    std::tm base = *localtime(&baseDate);
    std::tm compared = *localtime(&comparedDate);

    if (compared.tm_mon == base.tm_mon) {
        if (compared.tm_mday == base.tm_mday) return 0;
        return (compared.tm_mday < base.tm_mday) ? -1 : 1;
    }
    return (compared.tm_mon < base.tm_mon) ? -1 : 1;
}

// Для варианта: std::sort(std::begin(listOfBirthdays), std::end(listOfBirthdays), compareToSortByDay)
bool compareToSortByDay (const time_t baseDate, const time_t comparedDate) {
    std::tm base = *localtime(&baseDate);
    std::tm compared = *localtime(&comparedDate);
    cout << "День в году: " << base.tm_yday << endl;

    return (compared.tm_mon == base.tm_mon) ? compared.tm_mday >= base.tm_mday : compared.tm_mon >= base.tm_mon;
}

// 1. Получаем фамилию человека и дату его рождения (putTimeByFormat)
// из полученного значения даты, выделяем tm_yday
void addEntry(std::map<int, vector<string>> &calendar) {
    string format = "YYYY/mm/dd";

    auto name = putLineString("Введите имя человека");
    cout << "Получаем день его рождения в формате " << format << endl;
    auto fullBirthDate = putTimeByFormat(format);
    // Преобразовываем к строке, чтобы сохранить в vector<string>. Обрезаем конечные пробелы
    string fullBirthDateAsString = getTrimmedString(std::ctime(&fullBirthDate));
    // Извлекаем дату рождения относительно начала текущего года
    auto dayOfYear = extractDayOfYearFromDate(fullBirthDate);

    // ищем по ключу
    auto it = calendar.find(dayOfYear);
    // если записи не существует, создаём её и добавляем в calendar
    if (it == calendar.end()) {
        // Создаем vector, первым значением которого будет полная дата, а вторым - имя человека
        // дату можно будет в будущем, при необходимости, конвертировать в time_t. А имена - добавлять
        vector<string> newEntryValues = { name, std::ctime(&fullBirthDate) };
        std::pair<int, vector<string>> newEntry(dayOfYear, newEntryValues);
        calendar.insert(newEntry);
    }
    // если же запись существует, просто добавляем новое имя
    else {
        it->second.emplace_back(name);
        it->second.emplace_back( std::ctime(&fullBirthDate));
    }
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // Для записей типа { день_с_начала_текущего_года: { "Sun Aug 20 10:23:44 2023", realDate, name_2 ... }
    std::map<int, vector<std::string>> calendar;

    addEntry(calendar);
    addEntry(calendar);
    addEntry(calendar);
    addEntry(calendar);

    for (const auto &[key, values] : calendar) {
        cout << key << ": " << endl;
        for (const auto &value : values) {
            cout << "   - " << value << endl;
            cout << "   - " << value.length() << endl;
        }
    }

    time_t now = time(nullptr);
    int today = extractDayOfYearFromDate(now);

    // Находим следующий день рождения
    auto newBirthDay = calendar.lower_bound(today);

    cout << "Следующий: " << endl;
    if (newBirthDay != calendar.end()) {
        cout << newBirthDay->first << ": " << endl;
        for (const auto &value : newBirthDay->second) {
            cout << value << endl;
        }
    }
    else {
        cout << calendar.begin()->first << ": " << endl;
        for (const auto &value : calendar.begin()->second) {
            cout << value << endl;
        }
    }
}

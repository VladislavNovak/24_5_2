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

// Выводит следующий ближайший день рождения
void printNextBirthday(const std::map<int, vector<string>> &calendar) {
    time_t now = time(nullptr);
    // В calendar ключами выступают дни текущего года.
    // Поэтому из текущей даты извлекаем день года. Он позволит анализировать calendar.first
    int today = extractDayOfYearFromDate(now);

    // Находим следующий день рождения
    auto nextBirthDay = calendar.lower_bound(today);
    // Если запись найдена, значит до конца года есть ближайший день рождения. Выводим его
    if (nextBirthDay != calendar.end()) {
        cout << "Следующий ближайший день рождения в этом году у: " << endl;
        for (const auto &value : nextBirthDay->second) {
            cout << "  - " << value << endl;
        }
    }
    // Либо уже выводим первый день рождения в начале года (но это не должна быть сегодняшняя дата)
    else if (today != calendar.begin()->first) {
        cout << "Следующий ближайший день рождения будет уже в следующем году у: " << endl;
        for (const auto &value : calendar.begin()->second) {
            cout << "  - " << value << endl;
        }
    }
}

void printCurrentBirthday(const std::map<int, vector<string>> &calendar) {
    time_t now = time(nullptr);
    int today = extractDayOfYearFromDate(now);

    auto it = calendar.find(today);
    if (it != calendar.end()) {
        cout << "Сегодня день рождения у: " << endl;
        for (const auto &value : it->second) {
            cout << "  - " << value << endl;
        }
    }
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

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

    if (!calendar.empty()) {
        printNextBirthday(calendar);
        printCurrentBirthday(calendar);
    }
}

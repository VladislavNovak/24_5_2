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

bool hasLeapYear(time_t targetDate) {
    std::tm* local = localtime(&targetDate);
    int year = local->tm_year + 1900;
    return ((year % 400 == 0 || year % 100 != 0) && year % 4 == 0);
}

// Получает номер дня из даты.
int extractDayOfYearFromDate(time_t date) {
    std::tm local = *localtime(&date);
    time_t current = time(nullptr);
    int corrective{0};

    if (hasLeapYear(date) && !hasLeapYear(current)) corrective -= 1;
    else if (!hasLeapYear(date) && hasLeapYear(current)) corrective += 1;

    return (local.tm_yday + corrective);
}

void addEntry(std::map<int, vector<string>> &calendar, const string &name) {
    string format = "YYYY/mm/dd";

    // auto name = putLineString("Введите имя человека");
    cout << "Получаем день его рождения в формате " << format << endl;
    auto fullBirthDate = putTimeByFormat(format);
    // Преобразовываем к строке, чтобы сохранить в vector<string>. Обрезаем конечные пробелы
    string fullBirthDateAsString = getTrimmedString(std::ctime(&fullBirthDate));

    auto numberDayOfYear = extractDayOfYearFromDate(fullBirthDate);

    // ищем по ключу
    auto it = calendar.find(numberDayOfYear);
    // если записи не существует, создаём её и добавляем в calendar
    if (it == calendar.end()) {
        // Создаем vector, первым значением которого будет полная дата, а вторым - имя человека
        vector<string> newEntryValues = { name, std::ctime(&fullBirthDate) };
        std::pair<int, vector<string>> newEntry(numberDayOfYear, newEntryValues);
        calendar.insert(newEntry);
    }
    // если же запись существует, просто добавляем новое имя
    else {
        it->second.emplace_back(name);
        it->second.emplace_back( std::ctime(&fullBirthDate));
    }
}

std::tm convertStringToTime(const string &date) {
    time_t now = time(nullptr);
    std::tm toParse = *localtime(&now);
    std::istringstream ss(date);
    ss >> std::get_time(&toParse, "%a %b %d %H:%M:%S %Y");
    return toParse;
}

void printList(const vector<string> &list) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((it - list.begin()) % 2) {
            std::tm parse = convertStringToTime(*it);
            printf("%i/%i/%i\n", parse.tm_mday, (parse.tm_mon + 1), (parse.tm_year + 1900));
        }
        else cout << *it << ": ";
    }
}

// Выводит следующий ближайший день рождения
void printNextBirthday(const std::map<int, vector<string>> &calendar) {
    time_t now = time(nullptr);

    // Из текущей даты извлекаем день года. Он позволит анализировать calendar.first
    int today = extractDayOfYearFromDate(now);

    // Находим следующий день рождения
    auto nextBirthDay = calendar.upper_bound(today);

    if (nextBirthDay != calendar.end()) {
        cout << "Следующий ближайший день рождения в этом году: " << endl;
        printList(nextBirthDay->second);
    }
    // Либо уже выводим первый день рождения в начале года (но это не должна быть сегодняшняя дата)
    else if (today != calendar.begin()->first) {
        cout << "Ближайший день рождения будет уже в следующем году: " << endl;
        printList(calendar.begin()->second);
    }
}

void printCurrentBirthday(const std::map<int, vector<string>> &calendar) {
    time_t now = time(nullptr);
    int today = extractDayOfYearFromDate(now);

    auto it = calendar.find(today);

    if (it != calendar.end()) {
        cout << "Сегодня день рождения: " << endl;
        printList(it->second);
    }
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    string msg = "Введите имя человека для создания новой записи. Либо введите end для вывода отчета";
    std::map<int, vector<std::string>> calendar;

    while(true) {
        auto name = putLineString(msg);
        if (name == "end") break;

        addEntry(calendar, name);
    }

    if (!calendar.empty()) {
        printNextBirthday(calendar);
        printCurrentBirthday(calendar);
    }
}

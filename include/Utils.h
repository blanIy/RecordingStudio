#pragma once
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace util {

std::string trim(const std::string& s);

// Формат записей файла: поля через '|', спецсимволы экранируются.
std::string escape(const std::string& s);
std::vector<std::string> splitRecord(const std::string& line);
std::vector<int> parseIntList(const std::string& s);  // "1,2,3"

template <typename Range>
std::string joinInts(const Range& r) {
    std::ostringstream out;
    bool first = true;
    for (int v : r) {
        if (!first) out << ',';
        out << v;
        first = false;
    }
    return out.str();
}

int parseInt(const std::string& s, const std::string& field = "число");
double parseDouble(const std::string& s, const std::string& field = "число");
bool parseBool(const std::string& s);

std::string num(double v);          // число для записи в файл
std::string formatMoney(double v);  // "123.45 BYN"

// Ввод: подсказка выводится только если читаем из std::cin.
std::string readLine(std::istream& is, const std::string& prompt);
int readInt(std::istream& is, const std::string& prompt);
double readDouble(std::istream& is, const std::string& prompt);

}  // namespace util

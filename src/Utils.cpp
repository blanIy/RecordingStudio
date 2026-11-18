#include "Utils.h"

#include <iomanip>
#include <iostream>

#include "Exceptions.h"

namespace util {

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    const auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    const auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

std::string escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '|') out += "\\p";
        else if (c == '\n') out += "\\n";
        else if (c != '\r') out += c;
    }
    return out;
}

std::vector<std::string> splitRecord(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '\\' && i + 1 < line.size()) {
            const char n = line[++i];
            cur += (n == 'p') ? '|' : (n == 'n') ? '\n' : n;
        } else if (c == '|') {
            fields.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    fields.push_back(cur);
    return fields;
}

std::vector<int> parseIntList(const std::string& s) {
    std::vector<int> out;
    std::string t = trim(s);
    if (t.empty()) return out;
    std::stringstream ss(t);
    std::string item;
    while (std::getline(ss, item, ',')) out.push_back(parseInt(item, "идентификатор"));
    return out;
}

int parseInt(const std::string& s, const std::string& field) {
    const std::string t = trim(s);
    try {
        std::size_t pos = 0;
        const int v = std::stoi(t, &pos);
        if (pos == t.size()) return v;
    } catch (const std::exception&) {
    }
    throw ValidationException("поле «" + field + "» должно быть целым числом");
}

double parseDouble(const std::string& s, const std::string& field) {
    std::string t = trim(s);
    for (char& c : t)
        if (c == ',') c = '.';
    try {
        std::size_t pos = 0;
        const double v = std::stod(t, &pos);
        if (pos == t.size()) return v;
    } catch (const std::exception&) {
    }
    throw ValidationException("поле «" + field + "» должно быть числом");
}

bool parseBool(const std::string& s) {
    const std::string t = trim(s);
    if (t == "1" || t == "true") return true;
    if (t == "0" || t == "false") return false;
    throw ValidationException("ожидалось логическое значение 0/1");
}

std::string num(double v) {
    std::ostringstream o;
    o << std::setprecision(10) << v;
    return o.str();
}

std::string formatMoney(double v) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v << " BYN";
    return o.str();
}

std::string readLine(std::istream& is, const std::string& prompt) {
    if (&is == &std::cin) std::cout << prompt << ": " << std::flush;
    std::string s;
    if (!std::getline(is, s)) throw ValidationException("ввод прерван");
    return trim(s);
}

int readInt(std::istream& is, const std::string& prompt) { return parseInt(readLine(is, prompt), prompt); }
double readDouble(std::istream& is, const std::string& prompt) { return parseDouble(readLine(is, prompt), prompt); }

}  // namespace util

#include "DateTime.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "Exceptions.h"

namespace {

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : d[m - 1];
}

// Алгоритмы перевода даты <-> число дней от 1970-01-01 (собственная реализация).
long long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const long long yoe = y - era * 400;
    const long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

void civilFromDays(long long z, int& y, int& m, int& d) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const long long doe = z - era * 146097;
    const long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long long mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    y = static_cast<int>(yoe + era * 400) + (m <= 2 ? 1 : 0);
}

}  // namespace

DateTime::DateTime(int year, int month, int day, int hour, int minute)
    : year_(year), month_(month), day_(day), hour_(hour), minute_(minute) {
    if (year < 2000 || year > 2100) throw ValidationException("год должен быть в диапазоне 2000–2100");
    if (month < 1 || month > 12) throw ValidationException("месяц должен быть от 1 до 12");
    if (day < 1 || day > daysInMonth(year, month)) throw ValidationException("неверный день месяца");
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) throw ValidationException("неверное время");
}

DateTime DateTime::parse(const std::string& text) {
    std::istringstream in(text);
    int y = 0, m = 0, d = 0, h = 0, mi = 0;
    char c1 = 0, c2 = 0, c3 = 0;
    if (!(in >> y >> c1 >> m >> c2 >> d) || c1 != '-' || c2 != '-')
        throw ValidationException("дата должна иметь формат ГГГГ-ММ-ДД [ЧЧ:ММ]");
    in >> std::ws;
    if (in.peek() != std::char_traits<char>::eof()) {
        if (!(in >> h >> c3 >> mi) || c3 != ':') throw ValidationException("время должно иметь формат ЧЧ:ММ");
        in >> std::ws;
        if (in.peek() != std::char_traits<char>::eof()) throw ValidationException("лишние символы после времени");
    }
    return DateTime(y, m, d, h, mi);
}

long long DateTime::toMinutes() const {
    return daysFromCivil(year_, month_, day_) * 1440 + hour_ * 60 + minute_;
}

DateTime DateTime::fromMinutes(long long total) {
    long long days = total / 1440;
    long long rem = total % 1440;
    if (rem < 0) { rem += 1440; --days; }
    int y = 0, m = 0, d = 0;
    civilFromDays(days, y, m, d);
    return DateTime(y, m, d, static_cast<int>(rem / 60), static_cast<int>(rem % 60));
}

std::string DateTime::dateString() const {
    std::ostringstream o;
    o << std::setfill('0') << std::setw(4) << year_ << '-' << std::setw(2) << month_ << '-' << std::setw(2) << day_;
    return o.str();
}

std::string DateTime::toString() const {
    std::ostringstream o;
    o << dateString() << ' ' << std::setfill('0') << std::setw(2) << hour_ << ':' << std::setw(2) << minute_;
    return o.str();
}

std::ostream& operator<<(std::ostream& os, const DateTime& dt) { return os << dt.toString(); }

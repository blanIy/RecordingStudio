#pragma once
#include <iosfwd>
#include <string>

// Простая дата-время (минутная точность) для расписания студии.
class DateTime {
public:
    DateTime() = default;
    DateTime(int year, int month, int day, int hour = 0, int minute = 0);

    static DateTime parse(const std::string& text);  // "ГГГГ-ММ-ДД" или "ГГГГ-ММ-ДД ЧЧ:ММ"
    static DateTime fromMinutes(long long minutesSinceEpoch);

    long long toMinutes() const;
    int minuteOfDay() const { return hour_ * 60 + minute_; }
    DateTime plusMinutes(long long m) const { return fromMinutes(toMinutes() + m); }

    int year() const { return year_; }
    int month() const { return month_; }
    int day() const { return day_; }

    std::string toString() const;      // "2026-12-01 10:00"
    std::string dateString() const;    // "2026-12-01"

    bool operator==(const DateTime&) const = default;
    auto operator<=>(const DateTime&) const = default;

private:
    int year_ = 1970, month_ = 1, day_ = 1, hour_ = 0, minute_ = 0;
};

std::ostream& operator<<(std::ostream& os, const DateTime& dt);

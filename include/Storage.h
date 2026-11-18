#pragma once
#include <string>

#include "Studio.h"

// Файловый ввод-вывод (лаб. 7): состояние, журнал, текстовый отчёт.
class Storage {
public:
    static void save(const Studio& studio, const std::string& path);
    static void load(Studio& studio, const std::string& path);  // при ошибке studio не меняется
    static void appendLog(const std::string& path, const std::string& message);
    static void writeReport(const Studio& studio, const std::string& path);
};

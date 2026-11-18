#pragma once
#include <string>
#include <vector>

#include "Studio.h"

// Консольный интерфейс. Содержит только ввод/вывод — вся логика находится в Studio.
class ConsoleUI {
public:
    explicit ConsoleUI(Studio& studio, std::string dataFile = "studio_data.txt", std::string logFile = "studio.log",
                       std::string reportFile = "report.txt");
    void run();

private:
    int choose(const std::string& title, const std::vector<std::string>& items) const;

    void clientsMenu();
    void staffMenu();
    void roomsMenu();
    void projectsMenu();
    void sessionsMenu();
    void invoicesMenu();
    void reportsMenu();
    void fileMenu();

    void createSession();
    void printStatistics() const;

    Studio& studio_;
    std::string dataFile_, logFile_, reportFile_;
};

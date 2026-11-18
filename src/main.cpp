#include <iostream>

#include "ConsoleUI.h"
#include "Exceptions.h"
#include "Storage.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    Studio studio;
    const std::string dataFile = "studio_data.txt";

    // Автозагрузка предыдущего состояния (если файл есть)
    try {
        Storage::load(studio, dataFile);
        std::cout << "Состояние восстановлено из " << dataFile << ".\n";
    } catch (const FileException&) {
        std::cout << "Файл данных не найден — начинаем с пустой базы.\n";
    } catch (const DataCorruptionException& e) {
        std::cout << e.what() << "\nФайл проигнорирован, начинаем с пустой базы.\n";
    }

    ConsoleUI ui(studio, dataFile);
    ui.run();
    return 0;
}

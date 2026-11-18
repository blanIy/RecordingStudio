#include "ConsoleUI.h"

#include <iostream>

#include "DemoData.h"
#include "Exceptions.h"
#include "Storage.h"
#include "Utils.h"

namespace {

// Единая обёртка: разные типы ошибок обрабатываются по-разному, программа продолжает работу (лаб. 6).
template <typename F>
void guarded(F&& action) {
    try {
        action();
    } catch (const ScheduleConflictException& e) {
        std::cout << "[Конфликт] " << e.what() << "\n  Подсказка: воспользуйтесь поиском свободного окна в меню «Сессии».\n";
    } catch (const EquipmentUnavailableException& e) {
        std::cout << "[Оборудование] " << e.what() << "\n  Подсказка: отметьте прибор исправным или уберите его из зала.\n";
    } catch (const NotFoundException& e) {
        std::cout << "[Не найдено] " << e.what() << "\n  Подсказка: проверьте введённый ID в списке.\n";
    } catch (const ValidationException& e) {
        std::cout << "[Ввод] " << e.what() << "\n  Повторите операцию.\n";
    } catch (const FileException& e) {
        std::cout << "[Файл] " << e.what() << '\n';
    } catch (const StudioException& e) {  // прочие ошибки предметной области
        std::cout << "[Ошибка] " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cout << "[Системная ошибка] " << e.what() << '\n';
    }
}

DateTime askDateTime(const std::string& prompt) { return DateTime::parse(util::readLine(std::cin, prompt + " (ГГГГ-ММ-ДД ЧЧ:ММ)")); }

}  // namespace

ConsoleUI::ConsoleUI(Studio& studio, std::string dataFile, std::string logFile, std::string reportFile)
    : studio_(studio), dataFile_(std::move(dataFile)), logFile_(std::move(logFile)), reportFile_(std::move(reportFile)) {
    studio_.setLogger([this](const std::string& msg) {
        try {
            Storage::appendLog(logFile_, msg);
        } catch (const FileException&) {  // сбой журнала не должен ломать основную операцию
        }
    });
}

int ConsoleUI::choose(const std::string& title, const std::vector<std::string>& items) const {
    std::cout << "\n=== " << title << " ===\n";
    for (std::size_t i = 0; i < items.size(); ++i) std::cout << i + 1 << ". " << items[i] << '\n';
    std::cout << "0. Назад\n";
    try {
        return util::parseInt(util::readLine(std::cin, "Выбор"), "пункт меню");
    } catch (const ValidationException& e) {
        if (std::cin.eof()) return 0;
        std::cout << e.what() << '\n';
        return -1;
    }
}

void ConsoleUI::run() {
    std::cout << "Информационная система студии звукозаписи\n";
    while (true) {
        const int c = choose("ГЛАВНОЕ МЕНЮ", {"Клиенты", "Персонал (звукорежиссёры, продюсеры)", "Залы и оборудование", "Проекты",
                                              "Сессии записи", "Счета и оплата", "Отчёты и статистика", "Файлы (сохранить/загрузить)",
                                              "Загрузить демо-данные"});
        if (c == 0) break;
        switch (c) {
            case 1: clientsMenu(); break;
            case 2: staffMenu(); break;
            case 3: roomsMenu(); break;
            case 4: projectsMenu(); break;
            case 5: sessionsMenu(); break;
            case 6: invoicesMenu(); break;
            case 7: reportsMenu(); break;
            case 8: fileMenu(); break;
            case 9: guarded([&] { seedDemoData(studio_); std::cout << "Демо-данные добавлены.\n"; }); break;
            default: std::cout << "Нет такого пункта.\n";
        }
    }
    guarded([&] { Storage::save(studio_, dataFile_); std::cout << "Данные сохранены в " << dataFile_ << ".\n"; });
}

void ConsoleUI::clientsMenu() {
    while (true) {
        const int c = choose("КЛИЕНТЫ", {"Список", "Добавить", "Изменить скидку", "Удалить", "Поиск (имя/группа, диапазон скидки)"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1: studio_.clients().print(std::cout); break;
                case 2: {
                    Client cl;
                    std::cin >> cl;  // перегруженный оператор ввода
                    std::cout << "Добавлен клиент с ID " << studio_.registerClient(std::move(cl)) << ".\n";
                    break;
                }
                case 3: {
                    const int id = util::readInt(std::cin, "ID клиента");
                    studio_.setClientDiscount(id, util::readDouble(std::cin, "Новая скидка, %"));
                    break;
                }
                case 4: studio_.removeClient(util::readInt(std::cin, "ID клиента")); break;
                case 5: {
                    const std::string sub = util::readLine(std::cin, "Часть имени/группы (пусто - любой)");
                    const double mn = util::readDouble(std::cin, "Скидка от, %");
                    const double mx = util::readDouble(std::cin, "Скидка до, %");
                    const auto found = studio_.findClients(sub, mn, mx);
                    for (const Client* p : found) std::cout << *p << '\n';
                    std::cout << "Найдено: " << found.size() << '\n';
                    break;
                }
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::staffMenu() {
    while (true) {
        const int c = choose("ПЕРСОНАЛ", {"Все участники (полиморфный вывод)", "Добавить звукорежиссёра", "Добавить продюсера",
                                          "Удалить звукорежиссёра", "Удалить продюсера"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1:
                    for (const Person* p : studio_.allPeople())
                        std::cout << *p << (p->hourlyCost() > 0 ? "" : "  (почасовой оплаты нет)") << '\n';
                    break;
                case 2: {
                    SoundEngineer e;
                    std::cin >> e;
                    std::cout << "Добавлен звукорежиссёр с ID " << studio_.registerEngineer(std::move(e)) << ".\n";
                    break;
                }
                case 3: {
                    Producer p;
                    std::cin >> p;
                    std::cout << "Добавлен продюсер с ID " << studio_.registerProducer(std::move(p)) << ".\n";
                    break;
                }
                case 4: studio_.removeEngineer(util::readInt(std::cin, "ID звукорежиссёра")); break;
                case 5: studio_.removeProducer(util::readInt(std::cin, "ID продюсера")); break;
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::roomsMenu() {
    while (true) {
        const int c = choose("ЗАЛЫ И ОБОРУДОВАНИЕ", {"Список залов", "Добавить зал", "Список оборудования", "Добавить оборудование",
                                                     "Установить оборудование в зал", "Снять оборудование из зала",
                                                     "Отметить неисправным", "Отметить исправным", "Удалить оборудование"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1: studio_.rooms().print(std::cout); break;
                case 2: {
                    StudioRoom r;
                    std::cin >> r;
                    std::cout << "Добавлен зал с ID " << studio_.registerRoom(std::move(r)) << ".\n";
                    break;
                }
                case 3: studio_.equipment().print(std::cout); break;
                case 4: {
                    Equipment e;
                    std::cin >> e;
                    std::cout << "Добавлено оборудование с ID " << studio_.registerEquipment(std::move(e)) << ".\n";
                    break;
                }
                case 5: {
                    const int room = util::readInt(std::cin, "ID зала");
                    studio_.installEquipment(room, util::readInt(std::cin, "ID оборудования"));
                    break;
                }
                case 6: {
                    const int room = util::readInt(std::cin, "ID зала");
                    studio_.uninstallEquipment(room, util::readInt(std::cin, "ID оборудования"));
                    break;
                }
                case 7: studio_.reportBroken(util::readInt(std::cin, "ID оборудования")); break;
                case 8: studio_.repairEquipment(util::readInt(std::cin, "ID оборудования")); break;
                case 9: studio_.removeEquipment(util::readInt(std::cin, "ID оборудования")); break;
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::projectsMenu() {
    while (true) {
        const int c = choose("ПРОЕКТЫ", {"Список", "Создать проект", "Перевести на следующий этап", "Удалить"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1: studio_.projects().print(std::cout); break;
                case 2: {
                    Project p;
                    std::cin >> p;
                    std::cout << "Создан проект с ID " << studio_.createProject(std::move(p)) << ".\n";
                    break;
                }
                case 3: {
                    const int id = util::readInt(std::cin, "ID проекта");
                    const auto next = static_cast<int>(studio_.projects().get(id).getStatus()) + 1;
                    if (next > static_cast<int>(ProjectStatus::MASTERED)) throw ConstraintViolationException("проект уже завершён");
                    studio_.advanceProject(id, static_cast<ProjectStatus>(next));
                    std::cout << "Проект переведён на этап «" << statusLabel(static_cast<ProjectStatus>(next)) << "».\n";
                    break;
                }
                case 4: studio_.removeProject(util::readInt(std::cin, "ID проекта")); break;
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::createSession() {
    const int t = util::readInt(std::cin, "Тип (1-запись, 2-сведение, 3-мастеринг, 4-репетиция)");
    if (t < 1 || t > 4) throw ValidationException("тип сессии должен быть от 1 до 4");
    const auto type = static_cast<SessionType>(t - 1);

    SessionData d;
    d.clientId = util::readInt(std::cin, "ID клиента");
    d.engineerId = util::readInt(std::cin, "ID звукорежиссёра");
    d.roomId = util::readInt(std::cin, "ID зала");
    d.projectId = util::readInt(std::cin, "ID проекта (0 - без проекта, только репетиция)");
    d.producerId = util::readInt(std::cin, "ID продюсера (0 - без продюсера)");
    d.start = askDateTime("Начало");
    d.durationHours = util::readDouble(std::cin, "Длительность, ч (0.5-12)");
    d.rentedEquipmentIds = util::parseIntList(util::readLine(std::cin, "ID арендуемого оборудования через запятую (пусто - нет)"));
    static const char* labels[] = {"Число треков", "Число стемов", "Число форматов выдачи", "Число музыкантов"};
    const int param = util::readInt(std::cin, labels[t - 1]);

    const int id = studio_.planSession(type, std::move(d), param);
    std::cout << "Сессия #" << id << " запланирована. Предварительная стоимость: "
              << util::formatMoney(studio_.calculateSessionCost(id)) << ".\nДля резервирования времени подтвердите её.\n";
}

void ConsoleUI::sessionsMenu() {
    while (true) {
        const int c = choose("СЕССИИ", {"Список (по времени)", "Запланировать сессию", "Подтвердить сессию", "Отменить сессию",
                                        "Рассчитать стоимость", "Найти свободное окно"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1:
                    for (const Session* s : studio_.sessionsSortedByStart()) std::cout << *s << '\n';
                    if (studio_.sessions().empty()) std::cout << "(сессий нет)\n";
                    break;
                case 2: createSession(); break;
                case 3: {
                    const int id = util::readInt(std::cin, "ID сессии");
                    studio_.confirmSession(id);
                    std::cout << "Сессия подтверждена.\n";
                    break;
                }
                case 4: studio_.cancelSession(util::readInt(std::cin, "ID сессии")); break;
                case 5: {
                    const int id = util::readInt(std::cin, "ID сессии");
                    std::cout << "Стоимость: " << util::formatMoney(studio_.calculateSessionCost(id)) << '\n';
                    break;
                }
                case 6: {
                    const int room = util::readInt(std::cin, "ID зала");
                    const int eng = util::readInt(std::cin, "ID звукорежиссёра");
                    const DateTime day = DateTime::parse(util::readLine(std::cin, "День (ГГГГ-ММ-ДД)"));
                    const double h = util::readDouble(std::cin, "Длительность, ч");
                    if (auto slot = studio_.findFreeSlot(room, eng, day, h)) std::cout << "Ближайшее свободное время: " << *slot << '\n';
                    else std::cout << "В этот день свободных окон нет.\n";
                    break;
                }
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::invoicesMenu() {
    while (true) {
        const int c = choose("СЧЕТА", {"Список", "Выставить счёт по сессии", "Принять оплату", "Показать квитанцию"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1: studio_.invoices().print(std::cout); break;
                case 2: {
                    const int id = studio_.issueInvoice(util::readInt(std::cin, "ID подтверждённой сессии"));
                    std::cout << "Выставлен счёт #" << id << ".\n";
                    break;
                }
                case 3: {
                    const int id = util::readInt(std::cin, "ID счёта");
                    studio_.payInvoice(id, askDateTime("Дата и время оплаты"));
                    std::cout << studio_.invoices().get(id).getReceiptDetails();
                    break;
                }
                case 4: std::cout << studio_.invoices().get(util::readInt(std::cin, "ID счёта")).getReceiptDetails(); break;
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::printStatistics() const {
    const StudioStats st = studio_.statistics();
    std::cout << "Клиентов: " << st.clients << ", звукорежиссёров: " << st.engineers << ", залов: " << st.rooms
              << ", оборудования: " << st.equipment << ", проектов: " << st.projects << '\n'
              << "Сессий: " << st.sessionsTotal << " (подтверждено " << st.sessionsConfirmed << ", отменено " << st.sessionsCancelled << ")\n"
              << "Оплачено: " << util::formatMoney(st.revenuePaid) << ", ожидает оплаты: " << util::formatMoney(st.revenueUnpaid) << '\n';
    if (st.maxInvoice) {
        std::cout << "Максимальный счёт: #" << st.maxInvoice->getId() << " (" << util::formatMoney(st.maxInvoice->getAmount()) << "), минимальный: #"
                  << st.minInvoice->getId() << " (" << util::formatMoney(st.minInvoice->getAmount()) << "), средний: "
                  << util::formatMoney(st.averageInvoice) << '\n';
    }
    if (const Client* top = studio_.topClient()) std::cout << "Клиент с наибольшей выручкой: " << top->getName() << '\n';
}

void ConsoleUI::reportsMenu() {
    while (true) {
        const int c = choose("ОТЧЁТЫ", {"Общая статистика", "Сессии по типам", "Загрузка звукорежиссёров (часы)", "Оборудование по типам",
                                        "Неисправное оборудование", "Жанры проектов", "Последние события", "Сформировать текстовый отчёт в файл"});
        if (c == 0) return;
        guarded([&] {
            switch (c) {
                case 1: printStatistics(); break;
                case 2:
                    for (const auto& [name, n] : studio_.sessionCountByType()) std::cout << name << ": " << n << '\n';
                    break;
                case 3:
                    for (const auto& [id, h] : studio_.engineerHours()) {
                        const auto* e = studio_.engineers().find(id);
                        std::cout << (e ? e->getName() : "#" + std::to_string(id)) << ": " << h << " ч\n";
                    }
                    break;
                case 4:
                    for (const auto& [type, list] : studio_.equipmentByType()) {
                        std::cout << typeLabel(type) << " (" << list.size() << "):\n";
                        for (const Equipment* e : list) std::cout << "  " << *e << '\n';
                    }
                    break;
                case 5: {
                    const auto broken = studio_.brokenEquipment();
                    for (const Equipment* e : broken) std::cout << *e << '\n';
                    if (broken.empty()) std::cout << "Неисправного оборудования нет.\n";
                    break;
                }
                case 6:
                    for (const auto& g : studio_.projectGenres()) std::cout << g << '\n';
                    break;
                case 7:
                    for (const auto& msg : studio_.recentActivity()) std::cout << "- " << msg << '\n';
                    break;
                case 8:
                    Storage::writeReport(studio_, reportFile_);
                    std::cout << "Отчёт записан в " << reportFile_ << ".\n";
                    break;
                default: std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

void ConsoleUI::fileMenu() {
    while (true) {
        const int c = choose("ФАЙЛЫ", {"Сохранить данные", "Загрузить данные"});
        if (c == 0) return;
        guarded([&] {
            if (c == 1) {
                Storage::save(studio_, dataFile_);
                std::cout << "Сохранено в " << dataFile_ << ".\n";
            } else if (c == 2) {
                Storage::load(studio_, dataFile_);
                std::cout << "Данные загружены из " << dataFile_ << ".\n";
            } else {
                std::cout << "Нет такого пункта.\n";
            }
        });
    }
}

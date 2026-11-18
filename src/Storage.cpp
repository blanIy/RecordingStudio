#include "Storage.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include "Exceptions.h"
#include "Utils.h"

namespace {

constexpr const char* kHeader = "STUDIO_DB";
constexpr const char* kVersion = "1";

std::string now() {
    const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream o;
    o << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return o.str();
}

void parseRecord(Studio& s, const std::vector<std::string>& f) {
    const std::string& tag = f[0];
    const std::vector<std::string> rest(f.begin() + 1, f.end());
    if (tag == "CLIENT") s.restore(Client::fromFields(rest));
    else if (tag == "ENGINEER") s.restore(SoundEngineer::fromFields(rest));
    else if (tag == "PRODUCER") s.restore(Producer::fromFields(rest));
    else if (tag == "EQUIP") s.restore(Equipment::fromFields(rest));
    else if (tag == "ROOM") s.restore(StudioRoom::fromFields(rest));
    else if (tag == "PROJECT") s.restore(Project::fromFields(rest));
    else if (tag == "SESSION") s.restore(sessionFromFields(rest));
    else if (tag == "INVOICE") s.restore(Invoice::fromFields(rest));
    else throw ValidationException("неизвестный тип записи «" + tag + "»");
}

}  // namespace

void Storage::save(const Studio& studio, const std::string& path) {
    const std::string tmpPath = path + ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::trunc);
        if (!out) throw FileException("не удалось открыть файл для записи: " + tmpPath);
        out << kHeader << '|' << kVersion << '\n';
        // Порядок важен: сначала сущности, на которые ссылаются остальные.
        for (const auto& x : studio.clients()) out << x.toRecord() << '\n';
        for (const auto& x : studio.engineers()) out << x.toRecord() << '\n';
        for (const auto& x : studio.producers()) out << x.toRecord() << '\n';
        for (const auto& x : studio.equipment()) out << x.toRecord() << '\n';
        for (const auto& x : studio.rooms()) out << x.toRecord() << '\n';
        for (const auto& x : studio.projects()) out << x.toRecord() << '\n';
        for (const auto& x : studio.sessions()) out << x->toRecord() << '\n';
        for (const auto& x : studio.invoices()) out << x.toRecord() << '\n';
        out.flush();
        if (!out) throw FileException("ошибка записи в файл: " + tmpPath);
    }
    std::error_code ec;
    std::filesystem::rename(tmpPath, path, ec);
    if (ec) {  // например, на Windows rename не заменяет существующий файл
        std::filesystem::remove(path, ec);
        std::filesystem::rename(tmpPath, path, ec);
        if (ec) throw FileException("не удалось заменить файл " + path + ": " + ec.message());
    }
}

void Storage::load(Studio& studio, const std::string& path) {
    std::ifstream in(path);
    if (!in) throw FileException("не удалось открыть файл для чтения: " + path);

    Studio fresh;  // загружаем во временный объект — при ошибке текущее состояние не портится
    std::string line;
    std::size_t lineNo = 0;
    bool headerSeen = false;
    while (std::getline(in, line)) {
        ++lineNo;
        if (util::trim(line).empty()) continue;
        try {
            const auto fields = util::splitRecord(line);
            if (!headerSeen) {
                if (fields.size() != 2 || fields[0] != kHeader || fields[1] != kVersion)
                    throw ValidationException("неверный заголовок файла");
                headerSeen = true;
            } else {
                parseRecord(fresh, fields);
            }
        } catch (const DataCorruptionException&) {
            throw;
        } catch (const StudioException& e) {
            throw DataCorruptionException("строка " + std::to_string(lineNo) + ": " + e.what());
        }
    }
    if (in.bad()) throw FileException("ошибка чтения файла: " + path);
    if (!headerSeen) throw DataCorruptionException("файл пуст или не содержит заголовка");
    studio = std::move(fresh);
}

void Storage::appendLog(const std::string& path, const std::string& message) {
    std::ofstream out(path, std::ios::app);
    if (!out) throw FileException("не удалось открыть журнал: " + path);
    out << '[' << now() << "] " << message << '\n';
    if (!out) throw FileException("ошибка записи в журнал: " + path);
}

void Storage::writeReport(const Studio& studio, const std::string& path) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) throw FileException("не удалось открыть файл отчёта: " + path);
    const StudioStats st = studio.statistics();
    out << "ОТЧЁТ ПО СТУДИИ ЗВУКОЗАПИСИ\nСформирован: " << now() << "\n\n";
    out << "Клиентов: " << st.clients << "\nЗвукорежиссёров: " << st.engineers << "\nЗалов: " << st.rooms
        << "\nЕдиниц оборудования: " << st.equipment << "\nПроектов: " << st.projects << "\n";
    out << "Сессий всего: " << st.sessionsTotal << " (подтверждено: " << st.sessionsConfirmed
        << ", отменено: " << st.sessionsCancelled << ")\n";
    out << "Оплачено: " << util::formatMoney(st.revenuePaid) << "\nОжидает оплаты: " << util::formatMoney(st.revenueUnpaid) << "\n\n";

    out << "РАСПИСАНИЕ (по возрастанию времени начала)\n";
    for (const Session* s : studio.sessionsSortedByStart()) out << "  " << *s << '\n';

    out << "\nСЧЕТА\n";
    for (const auto& i : studio.invoices()) out << "  " << i << '\n';

    out << "\nВЫРУЧКА ПО КЛИЕНТАМ (оплаченные счета)\n";
    for (const auto& [clientId, sum] : studio.revenueByClient()) {
        const Client* c = studio.clients().find(clientId);
        out << "  " << (c ? c->getName() : "клиент #" + std::to_string(clientId)) << ": " << util::formatMoney(sum) << '\n';
    }

    out << "\nОБОРУДОВАНИЕ НА РЕМОНТЕ\n";
    const auto broken = studio.brokenEquipment();
    if (broken.empty()) out << "  нет\n";
    for (const Equipment* e : broken) out << "  " << *e << '\n';
    if (!out) throw FileException("ошибка записи отчёта: " + path);
}

#pragma once
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Equipment.h"
#include "Invoice.h"
#include "Person.h"
#include "Project.h"
#include "Repository.h"
#include "Session.h"
#include "StudioRoom.h"

struct StudioStats {
    std::size_t clients = 0, engineers = 0, rooms = 0, equipment = 0, projects = 0;
    std::size_t sessionsTotal = 0, sessionsConfirmed = 0, sessionsCancelled = 0;
    double revenuePaid = 0.0, revenueUnpaid = 0.0, averageInvoice = 0.0;
    const Invoice* maxInvoice = nullptr;
    const Invoice* minInvoice = nullptr;
};

// Фасад предметной области: вся бизнес-логика находится здесь, а не в обработчиках интерфейса.
class Studio {
public:
    static constexpr int kOpenHour = 8;    // студия работает с 08:00
    static constexpr int kCloseHour = 24;  // до 24:00

    Studio();

    // --- Регистрация (id назначается автоматически) ---
    int registerClient(Client c);
    int registerEngineer(SoundEngineer e);
    int registerProducer(Producer p);
    int registerEquipment(Equipment e);
    int registerRoom(StudioRoom r);
    int createProject(Project p);

    // --- Изменение и удаление (с контролем связей) ---
    void setClientDiscount(int clientId, double percent);
    void removeClient(int id);
    void removeEngineer(int id);
    void removeProducer(int id);
    void removeEquipment(int id);
    void removeProject(int id);
    void installEquipment(int roomId, int equipmentId);
    void uninstallEquipment(int roomId, int equipmentId);
    void reportBroken(int equipmentId);
    void repairEquipment(int equipmentId);
    void advanceProject(int projectId, ProjectStatus next);

    // --- Сессии и финансы ---
    int planSession(SessionType type, SessionData data, int param);
    void confirmSession(int sessionId);   // проверка расписания и оборудования
    void cancelSession(int sessionId);
    double calculateSessionCost(int sessionId) const;
    int issueInvoice(int sessionId);
    void payInvoice(int invoiceId, const DateTime& when);
    // Собственный алгоритм: первое свободное окно в зале и у инженера в указанный день.
    std::optional<DateTime> findFreeSlot(int roomId, int engineerId, const DateTime& day, double hours) const;

    // --- Доступ только для чтения ---
    const Repository<Client>& clients() const { return clients_; }
    const Repository<SoundEngineer>& engineers() const { return engineers_; }
    const Repository<Producer>& producers() const { return producers_; }
    const Repository<Equipment>& equipment() const { return equipment_; }
    const Repository<StudioRoom>& rooms() const { return rooms_; }
    const Repository<Project>& projects() const { return projects_; }
    const Repository<std::unique_ptr<Session>>& sessions() const { return sessions_; }
    const Repository<Invoice>& invoices() const { return invoices_; }

    // --- Выборки и статистика (STL) ---
    std::vector<const Person*> allPeople() const;  // полиморфная коллекция
    std::vector<const Client*> findClients(const std::string& nameSubstr, double minDiscount, double maxDiscount) const;
    std::vector<const Session*> sessionsSortedByStart() const;
    std::vector<const Equipment*> brokenEquipment() const;
    std::map<EquipmentType, std::vector<const Equipment*>> equipmentByType() const;
    std::map<std::string, int> sessionCountByType() const;
    std::map<int, double> engineerHours() const;
    std::unordered_map<int, double> revenueByClient() const;
    std::set<std::string> projectGenres() const;
    const Client* topClient() const;
    StudioStats statistics() const;
    const std::deque<std::string>& recentActivity() const { return activity_; }

    void setLogger(std::function<void(const std::string&)> logger) { logger_ = std::move(logger); }

    // --- Восстановление из файла (с проверкой связей) ---
    void restore(Client c);
    void restore(SoundEngineer e);
    void restore(Producer p);
    void restore(Equipment e);
    void restore(StudioRoom r);
    void restore(Project p);
    void restore(std::unique_ptr<Session> s);
    void restore(Invoice i);

private:
    void validateSessionRefs(const Session& s) const;
    void checkWorkingHours(const Session& s) const;
    void checkAvailability(const Session& s) const;
    void logEvent(const std::string& msg);
    int roomOfEquipment(int equipmentId) const;  // 0 если не установлено

    Repository<Client> clients_;
    Repository<SoundEngineer> engineers_;
    Repository<Producer> producers_;
    Repository<Equipment> equipment_;
    Repository<StudioRoom> rooms_;
    Repository<Project> projects_;
    Repository<std::unique_ptr<Session>> sessions_;
    Repository<Invoice> invoices_;
    std::deque<std::string> activity_;
    std::function<void(const std::string&)> logger_;
};

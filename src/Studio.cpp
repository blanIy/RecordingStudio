#include "Studio.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "Algorithms.h"
#include "Exceptions.h"
#include "Utils.h"

Studio::Studio()
    : clients_("Клиент"), engineers_("Звукорежиссёр"), producers_("Продюсер"), equipment_("Оборудование"),
      rooms_("Зал"), projects_("Проект"), sessions_("Сессия"), invoices_("Счёт") {}

void Studio::logEvent(const std::string& msg) {
    activity_.push_back(msg);
    if (activity_.size() > 20) activity_.pop_front();
    if (logger_) logger_(msg);
}

// ---------------- Регистрация ----------------
int Studio::registerClient(Client c) {
    c.setId(clients_.nextId());
    const int id = c.getId();
    const std::string name = c.getName();
    clients_.add(std::move(c));
    logEvent("Зарегистрирован клиент #" + std::to_string(id) + " " + name);
    return id;
}

int Studio::registerEngineer(SoundEngineer e) {
    e.setId(engineers_.nextId());
    const int id = e.getId();
    const std::string name = e.getName();
    engineers_.add(std::move(e));
    logEvent("Принят звукорежиссёр #" + std::to_string(id) + " " + name);
    return id;
}

int Studio::registerProducer(Producer p) {
    p.setId(producers_.nextId());
    const int id = p.getId();
    const std::string name = p.getName();
    producers_.add(std::move(p));
    logEvent("Добавлен продюсер #" + std::to_string(id) + " " + name);
    return id;
}

int Studio::registerEquipment(Equipment e) {
    e.setId(equipment_.nextId());
    const int id = e.getId();
    const std::string model = e.getModel();
    equipment_.add(std::move(e));
    logEvent("Добавлено оборудование #" + std::to_string(id) + " " + model);
    return id;
}

int Studio::registerRoom(StudioRoom r) {
    r.setId(rooms_.nextId());
    const int id = r.getId();
    const std::string name = r.getName();
    rooms_.add(std::move(r));
    logEvent("Добавлен зал #" + std::to_string(id) + " " + name);
    return id;
}

int Studio::createProject(Project p) {
    clients_.get(p.getClientId());  // клиент должен существовать
    p.setId(projects_.nextId());
    const int id = p.getId();
    const std::string title = p.getTitle();
    projects_.add(std::move(p));
    logEvent("Создан проект #" + std::to_string(id) + " «" + title + "»");
    return id;
}

// ---------------- Изменение / удаление ----------------
void Studio::setClientDiscount(int clientId, double percent) {
    clients_.get(clientId).setDiscountPercent(percent);
    logEvent("Клиенту #" + std::to_string(clientId) + " установлена скидка " + util::num(percent) + "%");
}

void Studio::removeClient(int id) {
    clients_.get(id);
    if (projects_.findIf([id](const Project& p) { return p.getClientId() == id; }) ||
        sessions_.findIf([id](const std::unique_ptr<Session>& s) { return s->clientId() == id; }))
        throw ConstraintViolationException("клиент #" + std::to_string(id) + " связан с проектами или сессиями");
    clients_ -= id;
    logEvent("Удалён клиент #" + std::to_string(id));
}

void Studio::removeEngineer(int id) {
    engineers_.get(id);
    if (sessions_.findIf([id](const std::unique_ptr<Session>& s) { return s->engineerId() == id; }))
        throw ConstraintViolationException("звукорежиссёр #" + std::to_string(id) + " назначен на сессии");
    engineers_ -= id;
    logEvent("Удалён звукорежиссёр #" + std::to_string(id));
}

void Studio::removeProducer(int id) {
    producers_.get(id);
    if (sessions_.findIf([id](const std::unique_ptr<Session>& s) { return s->producerId() == id; }))
        throw ConstraintViolationException("продюсер #" + std::to_string(id) + " назначен на сессии");
    producers_ -= id;
    logEvent("Удалён продюсер #" + std::to_string(id));
}

int Studio::roomOfEquipment(int equipmentId) const {
    const auto* r = rooms_.findIf([equipmentId](const StudioRoom& room) { return room.hasEquipment(equipmentId); });
    return r ? r->getId() : 0;
}

void Studio::removeEquipment(int id) {
    equipment_.get(id);
    if (roomOfEquipment(id) != 0) throw ConstraintViolationException("сначала снимите оборудование #" + std::to_string(id) + " из зала");
    equipment_ -= id;
    logEvent("Удалено оборудование #" + std::to_string(id));
}

void Studio::removeProject(int id) {
    projects_.get(id);
    if (sessions_.findIf([id](const std::unique_ptr<Session>& s) { return s->projectId() == id; }))
        throw ConstraintViolationException("проект #" + std::to_string(id) + " используется в сессиях");
    projects_ -= id;
    logEvent("Удалён проект #" + std::to_string(id));
}

void Studio::installEquipment(int roomId, int equipmentId) {
    StudioRoom& room = rooms_.get(roomId);
    const Equipment& eq = equipment_.get(equipmentId);
    if (const int other = roomOfEquipment(equipmentId); other != 0)
        throw ConstraintViolationException("оборудование #" + std::to_string(equipmentId) + " уже установлено в зале #" + std::to_string(other));
    room += eq;
    logEvent("Оборудование #" + std::to_string(equipmentId) + " установлено в зал #" + std::to_string(roomId));
}

void Studio::uninstallEquipment(int roomId, int equipmentId) {
    StudioRoom& room = rooms_.get(roomId);
    room -= equipment_.get(equipmentId);
    logEvent("Оборудование #" + std::to_string(equipmentId) + " снято из зала #" + std::to_string(roomId));
}

void Studio::reportBroken(int equipmentId) {
    equipment_.get(equipmentId).markForRepair();
    logEvent("Оборудование #" + std::to_string(equipmentId) + " отправлено в ремонт");
}

void Studio::repairEquipment(int equipmentId) {
    equipment_.get(equipmentId).markRepaired();
    logEvent("Оборудование #" + std::to_string(equipmentId) + " исправно");
}

void Studio::advanceProject(int projectId, ProjectStatus next) {
    projects_.get(projectId).updateStatus(next);
    logEvent("Проект #" + std::to_string(projectId) + " переведён на этап «" + statusLabel(next) + "»");
}

// ---------------- Проверки сессий ----------------
void Studio::validateSessionRefs(const Session& s) const {
    const Client& client = clients_.get(s.clientId());
    engineers_.get(s.engineerId());
    const StudioRoom& room = rooms_.get(s.roomId());
    if (s.projectId() != 0) {
        const Project& pr = projects_.get(s.projectId());
        if (pr.getClientId() != client.getId())
            throw ConstraintViolationException("проект #" + std::to_string(pr.getId()) + " принадлежит другому клиенту");
    }
    if (s.producerId() != 0) producers_.get(s.producerId());
    for (int eqId : s.rentedEquipmentIds()) {
        equipment_.get(eqId);
        if (!room.hasEquipment(eqId))
            throw ConstraintViolationException("арендуемое оборудование #" + std::to_string(eqId) + " не установлено в зале");
    }
}

void Studio::checkWorkingHours(const Session& s) const {
    const long long startMin = s.start().minuteOfDay();
    const long long dur = std::llround(s.durationHours() * 60.0);
    if (startMin < kOpenHour * 60 || startMin + dur > kCloseHour * 60)
        throw ConstraintViolationException("студия работает с 08:00 до 24:00, сессия должна завершиться в тот же день");
}

void Studio::checkAvailability(const Session& s) const {
    for (const auto& other : sessions_) {
        if (other->getId() == s.getId() || other->status() != SessionStatus::CONFIRMED || !s.overlaps(*other)) continue;
        const std::string where = " (сессия #" + std::to_string(other->getId()) + ", " + other->start().toString() + ")";
        if (other->roomId() == s.roomId()) throw ScheduleConflictException("зал #" + std::to_string(s.roomId()) + " занят" + where);
        if (other->engineerId() == s.engineerId()) throw ScheduleConflictException("звукорежиссёр #" + std::to_string(s.engineerId()) + " занят" + where);
        if (other->clientId() == s.clientId()) throw ScheduleConflictException("клиент #" + std::to_string(s.clientId()) + " уже участвует в другой сессии" + where);
        if (s.producerId() != 0 && other->producerId() == s.producerId())
            throw ScheduleConflictException("продюсер #" + std::to_string(s.producerId()) + " занят" + where);
    }
}

// ---------------- Сессии и финансы ----------------
int Studio::planSession(SessionType type, SessionData data, int param) {
    data.id = sessions_.nextId();
    data.status = SessionStatus::PLANNED;
    auto session = makeSession(type, std::move(data), param);
    validateSessionRefs(*session);
    checkWorkingHours(*session);

    if (type != SessionType::REHEARSAL && session->projectId() == 0)
        throw ConstraintViolationException("для сессии типа «" + session->typeName() + "» необходимо указать проект");
    if (session->projectId() != 0) {
        const ProjectStatus st = projects_.get(session->projectId()).getStatus();
        if (type == SessionType::MIXING && st < ProjectStatus::RECORDING)
            throw ConstraintViolationException("сведение возможно только после этапа записи");
        if (type == SessionType::MASTERING && st < ProjectStatus::MIXING)
            throw ConstraintViolationException("мастеринг возможен только после этапа сведения");
    }
    const int id = session->getId();
    const std::string label = session->typeName();
    sessions_.add(std::move(session));
    logEvent("Запланирована сессия #" + std::to_string(id) + " (" + label + ")");
    return id;
}

void Studio::confirmSession(int sessionId) {
    Session& s = *sessions_.get(sessionId);
    if (s.status() != SessionStatus::PLANNED)
        throw InvalidStatusTransitionException(statusLabel(s.status()), statusLabel(SessionStatus::CONFIRMED));
    // 1) занятость зала, инженера, клиента, продюсера
    checkAvailability(s);
    // 2) исправность оборудования зала
    const StudioRoom& room = rooms_.get(s.roomId());
    for (int eqId : room.equipmentIds()) {
        const Equipment& eq = equipment_.get(eqId);
        if (!eq.isWorking() && eq.isCritical())
            throw EquipmentUnavailableException("критичное оборудование #" + std::to_string(eqId) + " (" + eq.getModel() + ") неисправно");
    }
    for (int eqId : s.rentedEquipmentIds()) {
        const Equipment& eq = equipment_.get(eqId);
        if (!eq.isWorking()) throw EquipmentUnavailableException("арендуемое оборудование #" + std::to_string(eqId) + " (" + eq.getModel() + ") неисправно");
    }
    s.confirm();
    logEvent("Подтверждена сессия #" + std::to_string(sessionId));
}

void Studio::cancelSession(int sessionId) {
    sessions_.get(sessionId)->cancel();
    logEvent("Отменена сессия #" + std::to_string(sessionId));
}

double Studio::calculateSessionCost(int sessionId) const {
    const Session& s = *sessions_.get(sessionId);
    CostContext ctx;
    ctx.roomRate = rooms_.get(s.roomId()).getBaseRatePerHour();
    ctx.engineerRate = engineers_.get(s.engineerId()).hourlyCost();
    ctx.producerRate = s.producerId() ? producers_.get(s.producerId()).hourlyCost() : 0.0;
    ctx.discountPercent = clients_.get(s.clientId()).getDiscountPercent();
    for (int eqId : s.rentedEquipmentIds()) ctx.rentalSum += equipment_.get(eqId).getRentalFee();
    return s.calculateCost(ctx);  // полиморфный вызов
}

int Studio::issueInvoice(int sessionId) {
    const Session& s = *sessions_.get(sessionId);
    if (s.status() != SessionStatus::CONFIRMED)
        throw ConstraintViolationException("счёт выставляется только для подтверждённой сессии");
    if (invoices_.findIf([sessionId](const Invoice& i) { return i.getSessionId() == sessionId; }))
        throw ConstraintViolationException("для сессии #" + std::to_string(sessionId) + " счёт уже выставлен");
    Invoice inv(invoices_.nextId(), sessionId, s.clientId(), calculateSessionCost(sessionId));
    const int id = inv.getId();
    const double amount = inv.getAmount();
    invoices_.add(std::move(inv));
    logEvent("Выставлен счёт #" + std::to_string(id) + " на " + util::formatMoney(amount));
    return id;
}

void Studio::payInvoice(int invoiceId, const DateTime& when) {
    invoices_.get(invoiceId).processPayment(when);
    logEvent("Оплачен счёт #" + std::to_string(invoiceId));
}

std::optional<DateTime> Studio::findFreeSlot(int roomId, int engineerId, const DateTime& day, double hours) const {
    rooms_.get(roomId);
    engineers_.get(engineerId);
    if (hours < Session::kMinHours || hours > Session::kMaxHours)
        throw ValidationException("длительность должна быть от 0.5 до 12 часов");
    const long long dayStart = DateTime(day.year(), day.month(), day.day()).toMinutes();
    const long long dur = std::llround(hours * 60.0);
    for (long long m = kOpenHour * 60; m + dur <= kCloseHour * 60; m += 30) {  // шаг 30 минут
        const long long s = dayStart + m, e = s + dur;
        const bool busy = std::any_of(sessions_.begin(), sessions_.end(), [&](const std::unique_ptr<Session>& p) {
            return p->status() == SessionStatus::CONFIRMED && (p->roomId() == roomId || p->engineerId() == engineerId) &&
                   p->start().toMinutes() < e && s < p->end().toMinutes();
        });
        if (!busy) return DateTime::fromMinutes(s);
    }
    return std::nullopt;
}

// ---------------- Выборки и статистика ----------------
std::vector<const Person*> Studio::allPeople() const {
    std::vector<const Person*> people;
    for (const auto& c : clients_) people.push_back(&c);
    for (const auto& e : engineers_) people.push_back(&e);
    for (const auto& p : producers_) people.push_back(&p);
    return people;
}

std::vector<const Client*> Studio::findClients(const std::string& sub, double minD, double maxD) const {
    return clients_.findAll([&](const Client& c) {
        return (sub.empty() || c.getName().find(sub) != std::string::npos || c.getBandName().find(sub) != std::string::npos) &&
               c.getDiscountPercent() >= minD && c.getDiscountPercent() <= maxD;
    });
}

std::vector<const Session*> Studio::sessionsSortedByStart() const {
    std::vector<const Session*> v;
    for (const auto& s : sessions_) v.push_back(s.get());
    mergeSort(v, [](const Session* a, const Session* b) { return *a < *b; });  // собственная сортировка
    return v;
}

std::vector<const Equipment*> Studio::brokenEquipment() const {
    std::vector<const Equipment*> v;
    for (const auto& e : equipment_)
        if (!e.isWorking()) v.push_back(&e);
    return v;
}

std::map<EquipmentType, std::vector<const Equipment*>> Studio::equipmentByType() const {
    std::map<EquipmentType, std::vector<const Equipment*>> groups;
    std::for_each(equipment_.begin(), equipment_.end(), [&](const Equipment& e) { groups[e.getType()].push_back(&e); });
    return groups;
}

std::map<std::string, int> Studio::sessionCountByType() const {
    std::map<std::string, int> m;
    for (const auto& s : sessions_)
        if (s->status() != SessionStatus::CANCELLED) ++m[s->typeName()];
    return m;
}

std::map<int, double> Studio::engineerHours() const {
    std::map<int, double> m;
    for (const auto& s : sessions_)
        if (s->status() == SessionStatus::CONFIRMED) m[s->engineerId()] += s->durationHours();
    return m;
}

std::unordered_map<int, double> Studio::revenueByClient() const {
    std::unordered_map<int, double> m;
    std::for_each(invoices_.begin(), invoices_.end(), [&](const Invoice& i) {
        if (i.isPaid()) m[i.getClientId()] += i.getAmount();
    });
    return m;
}

std::set<std::string> Studio::projectGenres() const {
    std::set<std::string> g;
    for (const auto& p : projects_) g.insert(p.getGenre());
    return g;
}

const Client* Studio::topClient() const {
    const auto rev = revenueByClient();
    if (rev.empty()) return nullptr;
    const auto it = std::max_element(rev.begin(), rev.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return clients_.find(it->first);
}

StudioStats Studio::statistics() const {
    StudioStats st;
    st.clients = clients_.size();
    st.engineers = engineers_.size();
    st.rooms = rooms_.size();
    st.equipment = equipment_.size();
    st.projects = projects_.size();
    st.sessionsTotal = sessions_.size();
    st.sessionsConfirmed = static_cast<std::size_t>(std::count_if(sessions_.begin(), sessions_.end(),
        [](const std::unique_ptr<Session>& s) { return s->status() == SessionStatus::CONFIRMED; }));
    st.sessionsCancelled = countIf(sessions_, [](const std::unique_ptr<Session>& s) { return s->status() == SessionStatus::CANCELLED; });
    for (const auto& i : invoices_) (i.isPaid() ? st.revenuePaid : st.revenueUnpaid) += i.getAmount();
    if (!invoices_.empty()) {
        const auto [mn, mx] = std::minmax_element(invoices_.begin(), invoices_.end(),
            [](const Invoice& a, const Invoice& b) { return a.getAmount() < b.getAmount(); });
        st.minInvoice = &*mn;
        st.maxInvoice = &*mx;
        st.averageInvoice = (st.revenuePaid + st.revenueUnpaid) / static_cast<double>(invoices_.size());
    }
    return st;
}

// ---------------- Восстановление ----------------
void Studio::restore(Client c) { clients_.add(std::move(c)); }
void Studio::restore(SoundEngineer e) { engineers_.add(std::move(e)); }
void Studio::restore(Producer p) { producers_.add(std::move(p)); }
void Studio::restore(Equipment e) { equipment_.add(std::move(e)); }

void Studio::restore(StudioRoom r) {
    for (int eqId : r.equipmentIds()) {
        equipment_.get(eqId);
        if (roomOfEquipment(eqId) != 0)
            throw ConstraintViolationException("оборудование #" + std::to_string(eqId) + " числится сразу в двух залах");
    }
    rooms_.add(std::move(r));
}

void Studio::restore(Project p) {
    clients_.get(p.getClientId());
    projects_.add(std::move(p));
}

void Studio::restore(std::unique_ptr<Session> s) {
    validateSessionRefs(*s);
    if (s->status() == SessionStatus::CONFIRMED) checkAvailability(*s);
    sessions_.add(std::move(s));
}

void Studio::restore(Invoice i) {
    const Session& s = *sessions_.get(i.getSessionId());
    clients_.get(i.getClientId());
    if (s.clientId() != i.getClientId()) throw ConstraintViolationException("клиент счёта не совпадает с клиентом сессии");
    if (invoices_.findIf([&](const Invoice& x) { return x.getSessionId() == i.getSessionId(); }))
        throw ConstraintViolationException("для сессии #" + std::to_string(i.getSessionId()) + " уже есть счёт");
    invoices_.add(std::move(i));
}

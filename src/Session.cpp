#include "Session.h"

#include <cmath>
#include <iostream>

#include "Exceptions.h"
#include "Utils.h"

std::string toString(SessionType t) {
    switch (t) {
        case SessionType::RECORDING: return "RECORDING";
        case SessionType::MIXING: return "MIXING";
        case SessionType::MASTERING: return "MASTERING";
        case SessionType::REHEARSAL: return "REHEARSAL";
    }
    return "?";
}

SessionType sessionTypeFromString(const std::string& s) {
    for (auto t : {SessionType::RECORDING, SessionType::MIXING, SessionType::MASTERING, SessionType::REHEARSAL})
        if (toString(t) == s) return t;
    throw ValidationException("неизвестный тип сессии: " + s);
}

std::string toString(SessionStatus s) {
    switch (s) {
        case SessionStatus::PLANNED: return "PLANNED";
        case SessionStatus::CONFIRMED: return "CONFIRMED";
        case SessionStatus::CANCELLED: return "CANCELLED";
    }
    return "?";
}

std::string statusLabel(SessionStatus s) {
    switch (s) {
        case SessionStatus::PLANNED: return "запланирована";
        case SessionStatus::CONFIRMED: return "подтверждена";
        case SessionStatus::CANCELLED: return "отменена";
    }
    return "?";
}

SessionStatus sessionStatusFromString(const std::string& s) {
    for (auto t : {SessionStatus::PLANNED, SessionStatus::CONFIRMED, SessionStatus::CANCELLED})
        if (toString(t) == s) return t;
    throw ValidationException("неизвестный статус сессии: " + s);
}

// ---------- Session ----------
Session::Session(SessionData data) : d_(std::move(data)) {
    if (d_.id < 0) throw ValidationException("id не может быть отрицательным");
    if (d_.clientId <= 0 || d_.engineerId <= 0 || d_.roomId <= 0)
        throw ValidationException("в сессии должны быть указаны клиент, звукорежиссёр и зал");
    if (d_.projectId < 0 || d_.producerId < 0) throw ValidationException("неверный идентификатор проекта/продюсера");
    if (d_.durationHours < kMinHours || d_.durationHours > kMaxHours)
        throw ValidationException("длительность сессии должна быть от 0.5 до 12 часов");
}

DateTime Session::end() const {
    return d_.start.plusMinutes(std::llround(d_.durationHours * 60.0));
}

void Session::confirm() {
    if (d_.status != SessionStatus::PLANNED)
        throw InvalidStatusTransitionException(statusLabel(d_.status), statusLabel(SessionStatus::CONFIRMED));
    d_.status = SessionStatus::CONFIRMED;
}

void Session::cancel() {
    if (d_.status == SessionStatus::CANCELLED)
        throw InvalidStatusTransitionException(statusLabel(d_.status), statusLabel(SessionStatus::CANCELLED));
    d_.status = SessionStatus::CANCELLED;
}

bool Session::overlaps(const Session& o) const {
    return start().toMinutes() < o.end().toMinutes() && o.start().toMinutes() < end().toMinutes();
}

// C = (T_зала + T_инженера + T_продюсера) * t * K_типа * (1 - D/100) + надбавка_типа + аренда
double Session::calculateCost(const CostContext& c) const {
    const double base = (c.roomRate + c.engineerRate + c.producerRate) * d_.durationHours *
                        complexityCoefficient() * (1.0 - c.discountPercent / 100.0);
    return std::round((base + surcharge() + c.rentalSum) * 100.0) / 100.0;
}

void Session::printInfo(std::ostream& os) const {
    os << "[Сессия #" << getId() << " | " << typeName() << "] " << start() << " – " << end() << " (" << d_.durationHours
       << " ч) | клиент #" << d_.clientId << " | инженер #" << d_.engineerId << " | зал #" << d_.roomId;
    if (d_.projectId) os << " | проект #" << d_.projectId;
    if (d_.producerId) os << " | продюсер #" << d_.producerId;
    os << " | " << paramLabel() << ": " << param() << " | " << statusLabel(d_.status);
}

std::string Session::toRecord() const {
    return "SESSION|" + toString(type()) + "|" + std::to_string(d_.id) + "|" + std::to_string(d_.clientId) + "|" +
           std::to_string(d_.engineerId) + "|" + std::to_string(d_.roomId) + "|" + std::to_string(d_.projectId) +
           "|" + std::to_string(d_.producerId) + "|" + d_.start.toString() + "|" + util::num(d_.durationHours) + "|" +
           toString(d_.status) + "|" + util::joinInts(d_.rentedEquipmentIds) + "|" + std::to_string(param());
}

std::ostream& operator<<(std::ostream& os, const Session& s) {
    s.printInfo(os);
    return os;
}

namespace {
int requirePositive(int v, const char* what) {
    if (v < 1) throw ValidationException(std::string(what) + " должно быть не меньше 1");
    return v;
}
}  // namespace

RecordingSession::RecordingSession(SessionData d, int n) : Session(std::move(d)), tracks_(requirePositive(n, "число треков")) {}
MixingSession::MixingSession(SessionData d, int n) : Session(std::move(d)), stems_(requirePositive(n, "число стемов")) {}
MasteringSession::MasteringSession(SessionData d, int n) : Session(std::move(d)), formats_(requirePositive(n, "число форматов")) {}
RehearsalSession::RehearsalSession(SessionData d, int n) : Session(std::move(d)), musicians_(requirePositive(n, "число музыкантов")) {}

std::unique_ptr<Session> makeSession(SessionType type, SessionData data, int param) {
    switch (type) {
        case SessionType::RECORDING: return std::make_unique<RecordingSession>(std::move(data), param);
        case SessionType::MIXING: return std::make_unique<MixingSession>(std::move(data), param);
        case SessionType::MASTERING: return std::make_unique<MasteringSession>(std::move(data), param);
        case SessionType::REHEARSAL: return std::make_unique<RehearsalSession>(std::move(data), param);
    }
    throw ValidationException("неизвестный тип сессии");
}

std::unique_ptr<Session> sessionFromFields(const std::vector<std::string>& f) {
    if (f.size() != 12) throw ValidationException("запись «SESSION» должна содержать 12 полей");
    SessionData d;
    const SessionType type = sessionTypeFromString(f[0]);
    d.id = util::parseInt(f[1], "id");
    d.clientId = util::parseInt(f[2], "клиент");
    d.engineerId = util::parseInt(f[3], "инженер");
    d.roomId = util::parseInt(f[4], "зал");
    d.projectId = util::parseInt(f[5], "проект");
    d.producerId = util::parseInt(f[6], "продюсер");
    d.start = DateTime::parse(f[7]);
    d.durationHours = util::parseDouble(f[8], "длительность");
    d.status = sessionStatusFromString(f[9]);
    d.rentedEquipmentIds = util::parseIntList(f[10]);
    return makeSession(type, std::move(d), util::parseInt(f[11], "параметр"));
}

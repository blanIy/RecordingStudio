#pragma once
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "DateTime.h"

enum class SessionType { RECORDING, MIXING, MASTERING, REHEARSAL };
enum class SessionStatus { PLANNED, CONFIRMED, CANCELLED };

std::string toString(SessionType t);
SessionType sessionTypeFromString(const std::string& s);
std::string toString(SessionStatus s);
std::string statusLabel(SessionStatus s);
SessionStatus sessionStatusFromString(const std::string& s);

// Данные для расчёта стоимости собирает Studio, а сам расчёт — полиморфный.
struct CostContext {
    double roomRate = 0.0;
    double engineerRate = 0.0;
    double producerRate = 0.0;
    double discountPercent = 0.0;
    double rentalSum = 0.0;
};

struct SessionData {
    int id = 0, clientId = 0, engineerId = 0, roomId = 0, projectId = 0, producerId = 0;
    DateTime start;
    double durationHours = 1.0;
    std::vector<int> rentedEquipmentIds;
    SessionStatus status = SessionStatus::PLANNED;
};

// Абстрактная сессия (лаб. 3, 4). Наследники: Recording, Mixing, Mastering, Rehearsal.
class Session {
public:
    static constexpr double kMinHours = 0.5;
    static constexpr double kMaxHours = 12.0;

    virtual ~Session() = default;
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    int getId() const { return d_.id; }
    int clientId() const { return d_.clientId; }
    int engineerId() const { return d_.engineerId; }
    int roomId() const { return d_.roomId; }
    int projectId() const { return d_.projectId; }
    int producerId() const { return d_.producerId; }
    const DateTime& start() const { return d_.start; }
    double durationHours() const { return d_.durationHours; }
    const std::vector<int>& rentedEquipmentIds() const { return d_.rentedEquipmentIds; }
    SessionStatus status() const { return d_.status; }
    DateTime end() const;

    void setId(int id) { d_.id = id; }
    void confirm();
    void cancel();
    bool overlaps(const Session& other) const;

    // Полиморфный интерфейс
    virtual SessionType type() const = 0;                  // чисто виртуальный
    virtual std::string typeName() const = 0;              // чисто виртуальный
    virtual double complexityCoefficient() const = 0;      // чисто виртуальный (K типа)
    virtual int param() const = 0;                         // специфический параметр типа
    virtual std::string paramLabel() const = 0;
    virtual double surcharge() const { return 0.0; }       // фиксированная надбавка типа
    virtual double calculateCost(const CostContext& ctx) const;
    virtual void printInfo(std::ostream& os) const;

    std::string toRecord() const;

    bool operator<(const Session& o) const { return d_.start < o.d_.start; }

protected:
    explicit Session(SessionData data);

private:
    SessionData d_;
};

std::ostream& operator<<(std::ostream& os, const Session& s);

class RecordingSession : public Session {
public:
    RecordingSession(SessionData d, int tracksCount);
    SessionType type() const override { return SessionType::RECORDING; }
    std::string typeName() const override { return "Запись"; }
    double complexityCoefficient() const override { return 1.0; }
    int param() const override { return tracks_; }
    std::string paramLabel() const override { return "треков"; }
private:
    int tracks_;
};

class MixingSession : public Session {
public:
    MixingSession(SessionData d, int stemsCount);
    SessionType type() const override { return SessionType::MIXING; }
    std::string typeName() const override { return "Сведение"; }
    double complexityCoefficient() const override { return 1.15; }
    int param() const override { return stems_; }
    std::string paramLabel() const override { return "стемов"; }
    double surcharge() const override { return 2.0 * stems_; }
private:
    int stems_;
};

class MasteringSession : public Session {
public:
    MasteringSession(SessionData d, int formatsCount);
    SessionType type() const override { return SessionType::MASTERING; }
    std::string typeName() const override { return "Мастеринг"; }
    double complexityCoefficient() const override { return 1.30; }
    int param() const override { return formats_; }
    std::string paramLabel() const override { return "форматов выдачи"; }
    double surcharge() const override { return 30.0 * formats_; }
private:
    int formats_;
};

class RehearsalSession : public Session {
public:
    RehearsalSession(SessionData d, int musiciansCount);
    SessionType type() const override { return SessionType::REHEARSAL; }
    std::string typeName() const override { return "Репетиция"; }
    double complexityCoefficient() const override { return 0.85; }
    int param() const override { return musicians_; }
    std::string paramLabel() const override { return "музыкантов"; }
    double surcharge() const override { return musicians_ > 4 ? 5.0 * (musicians_ - 4) : 0.0; }
private:
    int musicians_;
};

std::unique_ptr<Session> makeSession(SessionType type, SessionData data, int param);
std::unique_ptr<Session> sessionFromFields(const std::vector<std::string>& f);

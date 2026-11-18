#include "StudioRoom.h"

#include <iostream>

#include "Exceptions.h"
#include "Utils.h"

StudioRoom::StudioRoom(int id, std::string name, double baseRatePerHour) : name_(util::trim(name)) {
    setId(id);
    setBaseRate(baseRatePerHour);
    if (name_.empty()) throw ValidationException("название зала не может быть пустым");
}

void StudioRoom::setId(int id) {
    if (id < 0) throw ValidationException("id не может быть отрицательным");
    id_ = id;
}

void StudioRoom::setBaseRate(double rate) {
    if (rate <= 0.0) throw ValidationException("тариф зала должен быть положительным");
    baseRatePerHour_ = rate;
}

StudioRoom& StudioRoom::operator+=(const Equipment& eq) {
    if (!equipmentIds_.insert(eq.getId()).second)
        throw ConstraintViolationException("оборудование #" + std::to_string(eq.getId()) + " уже установлено в зале");
    return *this;
}

StudioRoom& StudioRoom::operator-=(const Equipment& eq) {
    if (equipmentIds_.erase(eq.getId()) == 0)
        throw ConstraintViolationException("оборудование #" + std::to_string(eq.getId()) + " не установлено в зале");
    return *this;
}

std::string StudioRoom::toRecord() const {
    return "ROOM|" + std::to_string(id_) + "|" + util::escape(name_) + "|" + util::num(baseRatePerHour_) + "|" +
           util::joinInts(equipmentIds_);
}

StudioRoom StudioRoom::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 4) throw ValidationException("запись «ROOM» должна содержать 4 поля");
    StudioRoom r(util::parseInt(f[0], "id"), f[1], util::parseDouble(f[2], "тариф"));
    for (int id : util::parseIntList(f[3]))
        if (!r.equipmentIds_.insert(id).second) throw ValidationException("повтор оборудования в зале");
    return r;
}

std::ostream& operator<<(std::ostream& os, const StudioRoom& r) {
    os << "[Зал #" << r.getId() << "] " << r.getName() << " | тариф: " << util::formatMoney(r.getBaseRatePerHour())
       << "/ч | оборудование: " << (r.equipmentIds().empty() ? "нет" : util::joinInts(r.equipmentIds()));
    return os;
}

std::istream& operator>>(std::istream& is, StudioRoom& r) {
    const std::string name = util::readLine(is, "Название зала");
    const double rate = util::readDouble(is, "Базовый тариф, BYN/ч");
    StudioRoom tmp(r.getId(), name, rate);
    for (int id : r.equipmentIds_) tmp.equipmentIds_.insert(id);
    r = std::move(tmp);
    return is;
}

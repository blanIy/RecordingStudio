#include "Equipment.h"

#include <iostream>

#include "Exceptions.h"
#include "Utils.h"

std::string toString(EquipmentType t) {
    switch (t) {
        case EquipmentType::MIC: return "MIC";
        case EquipmentType::CONSOLE: return "CONSOLE";
        case EquipmentType::INSTRUMENT: return "INSTRUMENT";
        case EquipmentType::MONITOR: return "MONITOR";
    }
    return "?";
}

std::string typeLabel(EquipmentType t) {
    switch (t) {
        case EquipmentType::MIC: return "микрофон";
        case EquipmentType::CONSOLE: return "микшерный пульт";
        case EquipmentType::INSTRUMENT: return "инструмент";
        case EquipmentType::MONITOR: return "мониторы";
    }
    return "?";
}

EquipmentType equipmentTypeFromString(const std::string& s) {
    for (auto t : {EquipmentType::MIC, EquipmentType::CONSOLE, EquipmentType::INSTRUMENT, EquipmentType::MONITOR})
        if (toString(t) == s) return t;
    throw ValidationException("неизвестный тип оборудования: " + s);
}

Equipment::Equipment(int id, std::string model, EquipmentType type, double rentalFee, bool critical, bool working)
    : model_(util::trim(model)), type_(type), rentalFee_(rentalFee), critical_(critical), working_(working) {
    setId(id);
    if (model_.empty()) throw ValidationException("модель оборудования не может быть пустой");
    if (rentalFee_ < 0.0) throw ValidationException("надбавка за аренду не может быть отрицательной");
}

void Equipment::setId(int id) {
    if (id < 0) throw ValidationException("id не может быть отрицательным");
    id_ = id;
}

std::string Equipment::toRecord() const {
    return "EQUIP|" + std::to_string(id_) + "|" + util::escape(model_) + "|" + toString(type_) + "|" +
           util::num(rentalFee_) + "|" + (critical_ ? "1" : "0") + "|" + (working_ ? "1" : "0");
}

Equipment Equipment::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 6) throw ValidationException("запись «EQUIP» должна содержать 6 полей");
    return Equipment(util::parseInt(f[0], "id"), f[1], equipmentTypeFromString(f[2]),
                     util::parseDouble(f[3], "надбавка"), util::parseBool(f[4]), util::parseBool(f[5]));
}

std::ostream& operator<<(std::ostream& os, const Equipment& e) {
    os << "[Оборудование #" << e.getId() << "] " << e.getModel() << " | " << typeLabel(e.getType());
    if (e.getRentalFee() > 0) os << " | аренда: +" << util::formatMoney(e.getRentalFee());
    if (e.isCritical()) os << " | критичное";
    os << " | " << (e.isWorking() ? "исправно" : "НА РЕМОНТЕ");
    return os;
}

std::istream& operator>>(std::istream& is, Equipment& e) {
    const std::string model = util::readLine(is, "Модель");
    const int t = util::readInt(is, "Тип (1-микрофон, 2-пульт, 3-инструмент, 4-мониторы)");
    if (t < 1 || t > 4) throw ValidationException("тип оборудования должен быть от 1 до 4");
    const double fee = util::readDouble(is, "Надбавка за аренду, BYN (0 если нет)");
    const int critical = util::readInt(is, "Критичное для работы зала? (1-да, 0-нет)");
    e = Equipment(e.getId(), model, static_cast<EquipmentType>(t - 1), fee, critical != 0, true);
    return is;
}

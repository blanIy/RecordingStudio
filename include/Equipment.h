#pragma once
#include <iosfwd>
#include <string>
#include <vector>

enum class EquipmentType { MIC, CONSOLE, INSTRUMENT, MONITOR };

std::string toString(EquipmentType t);                 // "MIC" ... (для файла)
std::string typeLabel(EquipmentType t);                // русское название
EquipmentType equipmentTypeFromString(const std::string& s);

class Equipment {
public:
    Equipment() = default;
    Equipment(int id, std::string model, EquipmentType type, double rentalFee = 0.0, bool critical = false,
              bool working = true);

    int getId() const { return id_; }
    const std::string& getModel() const { return model_; }
    EquipmentType getType() const { return type_; }
    double getRentalFee() const { return rentalFee_; }  // надбавка за аренду (редкие приборы), BYN
    bool isCritical() const { return critical_; }       // без неё сессия невозможна
    bool isWorking() const { return working_; }

    void setId(int id);
    void markForRepair() { working_ = false; }
    void markRepaired() { working_ = true; }

    std::string toRecord() const;
    static Equipment fromFields(const std::vector<std::string>& f);

    bool operator==(const Equipment& o) const { return id_ == o.id_; }

private:
    int id_ = 0;
    std::string model_;
    EquipmentType type_ = EquipmentType::MIC;
    double rentalFee_ = 0.0;
    bool critical_ = false;
    bool working_ = true;
};

std::ostream& operator<<(std::ostream& os, const Equipment& e);
std::istream& operator>>(std::istream& is, Equipment& e);

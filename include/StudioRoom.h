#pragma once
#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "Equipment.h"

// Тон-зал / аппаратная. Хранит идентификаторы установленного оборудования.
class StudioRoom {
public:
    StudioRoom() = default;
    StudioRoom(int id, std::string name, double baseRatePerHour);

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    double getBaseRatePerHour() const { return baseRatePerHour_; }
    const std::set<int>& equipmentIds() const { return equipmentIds_; }
    bool hasEquipment(int equipmentId) const { return equipmentIds_.count(equipmentId) != 0; }

    void setId(int id);
    void setBaseRate(double rate);

    // Лаб. 2: += добавляет оборудование в зал, -= убирает
    StudioRoom& operator+=(const Equipment& eq);
    StudioRoom& operator-=(const Equipment& eq);

    std::string toRecord() const;
    static StudioRoom fromFields(const std::vector<std::string>& f);

    bool operator==(const StudioRoom& o) const { return id_ == o.id_; }

    friend std::istream& operator>>(std::istream& is, StudioRoom& r);

private:
    int id_ = 0;
    std::string name_;
    double baseRatePerHour_ = 1.0;
    std::set<int> equipmentIds_;
};

std::ostream& operator<<(std::ostream& os, const StudioRoom& r);
std::istream& operator>>(std::istream& is, StudioRoom& r);

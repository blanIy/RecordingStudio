#include "Person.h"

#include <iostream>

#include "Exceptions.h"
#include "Utils.h"

// ---------- Person ----------
Person::Person(int id, std::string name, std::string phone, std::string email) {
    setId(id);
    setName(std::move(name));
    setPhone(std::move(phone));
    setEmail(std::move(email));
}

void Person::setId(int id) {
    if (id < 0) throw ValidationException("id не может быть отрицательным");
    id_ = id;
}

void Person::setName(std::string name) {
    name = util::trim(name);
    if (name.empty() || name.size() > 80) throw ValidationException("ФИО должно содержать от 1 до 80 символов");
    name_ = std::move(name);
}

void Person::setPhone(std::string phone) {
    phone = util::trim(phone);
    if (phone.size() < 5 || phone.size() > 20 || phone.find_first_not_of("+-() 0123456789") != std::string::npos)
        throw ValidationException("телефон должен содержать 5–20 символов (цифры, +, -, скобки)");
    phone_ = std::move(phone);
}

void Person::setEmail(std::string email) {
    email = util::trim(email);
    const auto at = email.find('@');
    if (at == std::string::npos || at == 0 || email.find('.', at) == std::string::npos ||
        email.find(' ') != std::string::npos)
        throw ValidationException("email имеет неверный формат");
    email_ = std::move(email);
}

std::string Person::baseRecord() const {
    return std::to_string(id_) + "|" + util::escape(name_) + "|" + util::escape(phone_) + "|" +
           util::escape(email_);
}

std::ostream& operator<<(std::ostream& os, const Person& p) {
    p.printInfo(os);
    return os;
}

namespace {
void requireSize(const std::vector<std::string>& f, std::size_t n, const char* what) {
    if (f.size() != n) throw ValidationException(std::string("запись «") + what + "» должна содержать " +
                                                 std::to_string(n) + " полей");
}
}  // namespace

// ---------- Client ----------
Client::Client(int id, std::string name, std::string phone, std::string email, std::string bandName,
               double discountPercent)
    : Person(id, std::move(name), std::move(phone), std::move(email)) {
    setBandName(std::move(bandName));
    setDiscountPercent(discountPercent);
}

void Client::setBandName(std::string band) {
    band = util::trim(band);
    if (band.empty()) throw ValidationException("название группы/артиста не может быть пустым");
    bandName_ = std::move(band);
}

void Client::setDiscountPercent(double percent) {
    if (percent < 0.0 || percent > 50.0) throw ValidationException("скидка должна быть в диапазоне 0–50%");
    discountPercent_ = percent;
}

void Client::printInfo(std::ostream& os) const {
    os << "[Клиент #" << getId() << "] " << getName() << " | " << getPhone() << " | " << getEmail()
       << " | группа: " << bandName_ << " | скидка: " << discountPercent_ << "%";
}

std::string Client::toRecord() const {
    return "CLIENT|" + baseRecord() + "|" + util::escape(bandName_) + "|" + util::num(discountPercent_);
}

Client Client::fromFields(const std::vector<std::string>& f) {
    requireSize(f, 6, "CLIENT");
    return Client(util::parseInt(f[0], "id"), f[1], f[2], f[3], f[4], util::parseDouble(f[5], "скидка"));
}

std::istream& operator>>(std::istream& is, Client& c) {
    Client tmp = c;  // изменяем копию, чтобы при ошибке исходный объект не пострадал
    tmp.setName(util::readLine(is, "ФИО"));
    tmp.setPhone(util::readLine(is, "Телефон"));
    tmp.setEmail(util::readLine(is, "Email"));
    tmp.bandName_ = util::readLine(is, "Название группы/артиста");  // доступ к закрытому полю (friend)
    if (tmp.bandName_.empty()) throw ValidationException("название группы/артиста не может быть пустым");
    tmp.setDiscountPercent(util::readDouble(is, "Персональная скидка, % (0-50)"));
    c = std::move(tmp);
    return is;
}

// ---------- SoundEngineer ----------
SoundEngineer::SoundEngineer(int id, std::string name, std::string phone, std::string email,
                             std::string specialization, double hourlyRate, int experienceYears)
    : Person(id, std::move(name), std::move(phone), std::move(email)) {
    setSpecialization(std::move(specialization));
    setHourlyRate(hourlyRate);
    setExperienceYears(experienceYears);
}

void SoundEngineer::setSpecialization(std::string s) {
    s = util::trim(s);
    if (s.empty()) throw ValidationException("специализация не может быть пустой");
    specialization_ = std::move(s);
}

void SoundEngineer::setHourlyRate(double rate) {
    if (rate <= 0.0) throw ValidationException("ставка должна быть положительной");
    hourlyRate_ = rate;
}

void SoundEngineer::setExperienceYears(int years) {
    if (years < 0 || years > 60) throw ValidationException("стаж должен быть в диапазоне 0–60 лет");
    experienceYears_ = years;
}

void SoundEngineer::printInfo(std::ostream& os) const {
    os << "[Звукорежиссёр #" << getId() << "] " << getName() << " | " << getPhone() << " | " << getEmail()
       << " | специализация: " << specialization_ << " | ставка: " << util::formatMoney(hourlyRate_)
       << "/ч | стаж: " << experienceYears_ << " л.";
}

std::string SoundEngineer::toRecord() const {
    return "ENGINEER|" + baseRecord() + "|" + util::escape(specialization_) + "|" + util::num(hourlyRate_) + "|" +
           std::to_string(experienceYears_);
}

SoundEngineer SoundEngineer::fromFields(const std::vector<std::string>& f) {
    requireSize(f, 7, "ENGINEER");
    return SoundEngineer(util::parseInt(f[0], "id"), f[1], f[2], f[3], f[4], util::parseDouble(f[5], "ставка"),
                         util::parseInt(f[6], "стаж"));
}

std::istream& operator>>(std::istream& is, SoundEngineer& e) {
    SoundEngineer tmp = e;
    tmp.setName(util::readLine(is, "ФИО"));
    tmp.setPhone(util::readLine(is, "Телефон"));
    tmp.setEmail(util::readLine(is, "Email"));
    tmp.setSpecialization(util::readLine(is, "Специализация"));
    tmp.setHourlyRate(util::readDouble(is, "Почасовая ставка, BYN"));
    tmp.setExperienceYears(util::readInt(is, "Стаж, лет"));
    e = std::move(tmp);
    return is;
}

// ---------- Producer ----------
Producer::Producer(int id, std::string name, std::string phone, std::string email, std::string genreFocus,
                   double hourlyRate)
    : Person(id, std::move(name), std::move(phone), std::move(email)) {
    setGenreFocus(std::move(genreFocus));
    setHourlyRate(hourlyRate);
}

void Producer::setGenreFocus(std::string g) {
    g = util::trim(g);
    if (g.empty()) throw ValidationException("жанровая специализация не может быть пустой");
    genreFocus_ = std::move(g);
}

void Producer::setHourlyRate(double rate) {
    if (rate < 0.0) throw ValidationException("ставка не может быть отрицательной");
    hourlyRate_ = rate;
}

void Producer::printInfo(std::ostream& os) const {
    os << "[Продюсер #" << getId() << "] " << getName() << " | " << getPhone() << " | " << getEmail()
       << " | жанр: " << genreFocus_ << " | ставка: " << util::formatMoney(hourlyRate_) << "/ч";
}

std::string Producer::toRecord() const {
    return "PRODUCER|" + baseRecord() + "|" + util::escape(genreFocus_) + "|" + util::num(hourlyRate_);
}

Producer Producer::fromFields(const std::vector<std::string>& f) {
    requireSize(f, 6, "PRODUCER");
    return Producer(util::parseInt(f[0], "id"), f[1], f[2], f[3], f[4], util::parseDouble(f[5], "ставка"));
}

std::istream& operator>>(std::istream& is, Producer& p) {
    Producer tmp = p;
    tmp.setName(util::readLine(is, "ФИО"));
    tmp.setPhone(util::readLine(is, "Телефон"));
    tmp.setEmail(util::readLine(is, "Email"));
    tmp.setGenreFocus(util::readLine(is, "Жанровая специализация"));
    tmp.setHourlyRate(util::readDouble(is, "Почасовая ставка, BYN (0 если нет)"));
    p = std::move(tmp);
    return is;
}

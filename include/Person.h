#pragma once
#include <compare>
#include <iosfwd>
#include <string>
#include <vector>

enum class PersonType { CLIENT, ENGINEER, PRODUCER };

// Абстрактная база иерархии «участники студии» (лаб. 3, 4).
class Person {
public:
    virtual ~Person() = default;

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getPhone() const { return phone_; }
    const std::string& getEmail() const { return email_; }

    void setId(int id);
    void setName(std::string name);
    void setPhone(std::string phone);
    void setEmail(std::string email);

    // Полиморфный интерфейс
    virtual PersonType type() const = 0;                 // чисто виртуальный
    virtual std::string typeName() const = 0;            // чисто виртуальный
    virtual void printInfo(std::ostream& os) const = 0;  // чисто виртуальный
    virtual double hourlyCost() const = 0;               // стоимость часа работы (0 для клиента)
    virtual std::string toRecord() const = 0;            // строка для файла

    friend std::ostream& operator<<(std::ostream& os, const Person& p);

protected:
    Person() = default;
    Person(int id, std::string name, std::string phone, std::string email);
    // Копирование разрешено только наследникам — защита от срезки объектов.
    Person(const Person&) = default;
    Person(Person&&) = default;
    Person& operator=(const Person&) = default;
    Person& operator=(Person&&) = default;

    std::string baseRecord() const;

private:
    int id_ = 0;
    std::string name_, phone_, email_;
};

class Client : public Person {
public:
    Client() = default;
    Client(int id, std::string name, std::string phone, std::string email, std::string bandName,
           double discountPercent);

    const std::string& getBandName() const { return bandName_; }
    double getDiscountPercent() const { return discountPercent_; }
    void setBandName(std::string band);
    void setDiscountPercent(double percent);  // 0..50

    PersonType type() const override { return PersonType::CLIENT; }
    std::string typeName() const override { return "Клиент"; }
    void printInfo(std::ostream& os) const override;
    double hourlyCost() const override { return 0.0; }
    std::string toRecord() const override;
    static Client fromFields(const std::vector<std::string>& f);

    // Лаб. 2: равенство по id, порядок по размеру скидки
    bool operator==(const Client& other) const { return getId() == other.getId(); }
    std::partial_ordering operator<=>(const Client& other) const {
        return discountPercent_ <=> other.discountPercent_;
    }

    friend std::istream& operator>>(std::istream& is, Client& c);

private:
    std::string bandName_;
    double discountPercent_ = 0.0;
};

class SoundEngineer : public Person {
public:
    SoundEngineer() = default;
    SoundEngineer(int id, std::string name, std::string phone, std::string email, std::string specialization,
                  double hourlyRate, int experienceYears);

    const std::string& getSpecialization() const { return specialization_; }
    double getHourlyRate() const { return hourlyRate_; }
    int getExperienceYears() const { return experienceYears_; }
    void setSpecialization(std::string s);
    void setHourlyRate(double rate);  // > 0
    void setExperienceYears(int years);  // >= 0

    PersonType type() const override { return PersonType::ENGINEER; }
    std::string typeName() const override { return "Звукорежиссёр"; }
    void printInfo(std::ostream& os) const override;
    double hourlyCost() const override { return hourlyRate_; }
    std::string toRecord() const override;
    static SoundEngineer fromFields(const std::vector<std::string>& f);

private:
    std::string specialization_;
    double hourlyRate_ = 1.0;
    int experienceYears_ = 0;
};

class Producer : public Person {
public:
    Producer() = default;
    Producer(int id, std::string name, std::string phone, std::string email, std::string genreFocus,
             double hourlyRate);

    const std::string& getGenreFocus() const { return genreFocus_; }
    double getHourlyRate() const { return hourlyRate_; }
    void setGenreFocus(std::string g);
    void setHourlyRate(double rate);  // >= 0

    PersonType type() const override { return PersonType::PRODUCER; }
    std::string typeName() const override { return "Продюсер"; }
    void printInfo(std::ostream& os) const override;
    double hourlyCost() const override { return hourlyRate_; }
    std::string toRecord() const override;
    static Producer fromFields(const std::vector<std::string>& f);

private:
    std::string genreFocus_;
    double hourlyRate_ = 0.0;
};

std::istream& operator>>(std::istream& is, SoundEngineer& e);
std::istream& operator>>(std::istream& is, Producer& p);

#pragma once
#include <stdexcept>
#include <string>

// Иерархия исключений предметной области (лаб. 6).
class StudioException : public std::runtime_error {
public:
    explicit StudioException(const std::string& msg) : std::runtime_error(msg) {}
};

class ValidationException : public StudioException {
public:
    explicit ValidationException(const std::string& m) : StudioException("Некорректные данные: " + m) {}
};

class NotFoundException : public StudioException {
public:
    NotFoundException(const std::string& entity, int id)
        : StudioException(entity + " с id=" + std::to_string(id) + " не найден(а)") {}
};

class DuplicateIdException : public StudioException {
public:
    DuplicateIdException(const std::string& entity, int id)
        : StudioException(entity + " с id=" + std::to_string(id) + " уже существует") {}
};

class ScheduleConflictException : public StudioException {
public:
    explicit ScheduleConflictException(const std::string& m) : StudioException("Конфликт расписания: " + m) {}
};

class EquipmentUnavailableException : public StudioException {
public:
    explicit EquipmentUnavailableException(const std::string& m) : StudioException("Оборудование недоступно: " + m) {}
};

class InvalidStatusTransitionException : public StudioException {
public:
    InvalidStatusTransitionException(const std::string& from, const std::string& to)
        : StudioException("Недопустимый переход статуса: " + from + " -> " + to) {}
};

class ConstraintViolationException : public StudioException {
public:
    explicit ConstraintViolationException(const std::string& m) : StudioException("Нарушено ограничение: " + m) {}
};

class FileException : public StudioException {
public:
    explicit FileException(const std::string& m) : StudioException("Ошибка файла: " + m) {}
};

class DataCorruptionException : public StudioException {
public:
    explicit DataCorruptionException(const std::string& m) : StudioException("Повреждённые данные: " + m) {}
};

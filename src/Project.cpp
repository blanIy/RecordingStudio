#include "Project.h"

#include <iostream>

#include "Exceptions.h"
#include "Utils.h"

std::string toString(ProjectStatus s) {
    switch (s) {
        case ProjectStatus::DEMO: return "DEMO";
        case ProjectStatus::RECORDING: return "RECORDING";
        case ProjectStatus::MIXING: return "MIXING";
        case ProjectStatus::MASTERED: return "MASTERED";
    }
    return "?";
}

std::string statusLabel(ProjectStatus s) {
    switch (s) {
        case ProjectStatus::DEMO: return "демо";
        case ProjectStatus::RECORDING: return "запись";
        case ProjectStatus::MIXING: return "сведение";
        case ProjectStatus::MASTERED: return "мастеринг завершён";
    }
    return "?";
}

ProjectStatus projectStatusFromString(const std::string& s) {
    for (auto v : {ProjectStatus::DEMO, ProjectStatus::RECORDING, ProjectStatus::MIXING, ProjectStatus::MASTERED})
        if (toString(v) == s) return v;
    throw ValidationException("неизвестный статус проекта: " + s);
}

Project::Project(int id, int clientId, std::string title, std::string genre, ProjectStatus status)
    : clientId_(clientId), title_(util::trim(title)), genre_(util::trim(genre)), status_(status) {
    setId(id);
    if (clientId_ <= 0) throw ValidationException("у проекта должен быть клиент");
    if (title_.empty()) throw ValidationException("название проекта не может быть пустым");
    if (genre_.empty()) throw ValidationException("жанр проекта не может быть пустым");
}

void Project::setId(int id) {
    if (id < 0) throw ValidationException("id не может быть отрицательным");
    id_ = id;
}

void Project::updateStatus(ProjectStatus next) {
    if (static_cast<int>(next) != static_cast<int>(status_) + 1)
        throw InvalidStatusTransitionException(statusLabel(status_), statusLabel(next));
    status_ = next;
}

std::string Project::toRecord() const {
    return "PROJECT|" + std::to_string(id_) + "|" + std::to_string(clientId_) + "|" + util::escape(title_) + "|" +
           util::escape(genre_) + "|" + toString(status_);
}

Project Project::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 5) throw ValidationException("запись «PROJECT» должна содержать 5 полей");
    return Project(util::parseInt(f[0], "id"), util::parseInt(f[1], "клиент"), f[2], f[3],
                   projectStatusFromString(f[4]));
}

std::ostream& operator<<(std::ostream& os, const Project& p) {
    return os << "[Проект #" << p.getId() << "] «" << p.getTitle() << "» | жанр: " << p.getGenre()
              << " | клиент #" << p.getClientId() << " | этап: " << statusLabel(p.getStatus());
}

std::istream& operator>>(std::istream& is, Project& p) {
    const int clientId = util::readInt(is, "ID клиента");
    const std::string title = util::readLine(is, "Название проекта");
    const std::string genre = util::readLine(is, "Жанр");
    p = Project(p.getId(), clientId, title, genre, ProjectStatus::DEMO);
    return is;
}

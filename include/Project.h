#pragma once
#include <iosfwd>
#include <string>
#include <vector>

enum class ProjectStatus { DEMO, RECORDING, MIXING, MASTERED };

std::string toString(ProjectStatus s);
std::string statusLabel(ProjectStatus s);
ProjectStatus projectStatusFromString(const std::string& s);

class Project {
public:
    Project() = default;
    Project(int id, int clientId, std::string title, std::string genre, ProjectStatus status = ProjectStatus::DEMO);

    int getId() const { return id_; }
    int getClientId() const { return clientId_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getGenre() const { return genre_; }
    ProjectStatus getStatus() const { return status_; }

    void setId(int id);
    // Статус можно менять только на следующий этап жизненного цикла.
    void updateStatus(ProjectStatus next);

    std::string toRecord() const;
    static Project fromFields(const std::vector<std::string>& f);

    bool operator==(const Project& o) const { return id_ == o.id_; }

private:
    int id_ = 0;
    int clientId_ = 0;
    std::string title_, genre_;
    ProjectStatus status_ = ProjectStatus::DEMO;
};

std::ostream& operator<<(std::ostream& os, const Project& p);
std::istream& operator>>(std::istream& is, Project& p);

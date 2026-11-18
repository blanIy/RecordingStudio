#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Exceptions.h"

// Универсальный шаблонный репозиторий (лаб. 5). Работает и со значениями
// (Client, Project, ...), и с умными указателями (std::unique_ptr<Session>).
template <typename T>
class Repository {
public:
    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    explicit Repository(std::string entityName = "Объект") : entityName_(std::move(entityName)) {}

    void add(T item) {
        const int id = idOf(item);
        if (contains(id)) throw DuplicateIdException(entityName_, id);
        items_.push_back(std::move(item));
    }

    void remove(int id) {
        auto it = std::find_if(items_.begin(), items_.end(), [id](const T& x) { return idOf(x) == id; });
        if (it == items_.end()) throw NotFoundException(entityName_, id);
        items_.erase(it);
    }

    T& get(int id) {
        T* p = find(id);
        if (!p) throw NotFoundException(entityName_, id);
        return *p;
    }
    const T& get(int id) const {
        const T* p = find(id);
        if (!p) throw NotFoundException(entityName_, id);
        return *p;
    }

    T* find(int id) {
        auto it = std::find_if(items_.begin(), items_.end(), [id](const T& x) { return idOf(x) == id; });
        return it == items_.end() ? nullptr : &*it;
    }
    const T* find(int id) const {
        auto it = std::find_if(items_.begin(), items_.end(), [id](const T& x) { return idOf(x) == id; });
        return it == items_.end() ? nullptr : &*it;
    }

    bool contains(int id) const { return find(id) != nullptr; }

    template <typename Pred>
    const T* findIf(Pred pred) const {
        auto it = std::find_if(items_.begin(), items_.end(), pred);
        return it == items_.end() ? nullptr : &*it;
    }

    template <typename Pred>
    std::vector<const T*> findAll(Pred pred) const {
        std::vector<const T*> result;
        for (const auto& x : items_)
            if (pred(x)) result.push_back(&x);
        return result;
    }

    std::size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }
    void clear() { items_.clear(); }

    int nextId() const {
        int mx = 0;
        for (const auto& x : items_) mx = std::max(mx, idOf(x));
        return mx + 1;
    }

    void print(std::ostream& os) const {
        if (items_.empty()) os << "(" << entityName_ << ": список пуст)\n";
        for (const auto& x : items_) {
            if constexpr (requires { *x; }) os << *x << '\n';
            else os << x << '\n';
        }
    }

    // Лаб. 2/5: += добавляет объект, -= удаляет по id
    Repository& operator+=(T item) { add(std::move(item)); return *this; }
    Repository& operator-=(int id) { remove(id); return *this; }

    iterator begin() { return items_.begin(); }
    iterator end() { return items_.end(); }
    const_iterator begin() const { return items_.begin(); }
    const_iterator end() const { return items_.end(); }

private:
    static int idOf(const T& item) {
        if constexpr (requires { item.getId(); }) return item.getId();
        else return item->getId();
    }

    std::string entityName_;
    std::vector<T> items_;
};

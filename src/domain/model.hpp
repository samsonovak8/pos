#pragma once

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "domain/value_objs.hpp"

namespace pos::domain {

inline std::string num(double v) {
    std::ostringstream os;
    os << v;
    return os.str();
}

class Student {
public:
    Student(StudentId id, std::string name) : id_(id), name_(std::move(name)) {
        if (name_.find_first_not_of(" \t") == std::string::npos) {
            throw InvalidValue("Имя студента не может быть пустым");
        }
    }
    StudentId id() const { return id_; }
    const std::string& name() const { return name_; }

private:
    StudentId id_;
    std::string name_;
};

class Criterion {
public:
    Criterion(CriterionId id, std::string name, Score max, Weight weight)
        : id_(id), name_(std::move(name)), max_(max), weight_(weight) {
        if (max.value() <= 0) {
            throw InvalidValue("Максимальный балл критерия должен быть больше нуля");
        }

        if (name_.find_first_not_of(" \t") == std::string::npos) {
            throw InvalidValue("Название критерия не может быть пустым");
        }
    }
    CriterionId id() const { return id_; }
    const std::string& name() const { return name_; }
    Score max() const { return max_; }
    Weight weight() const { return weight_; }

private:
    CriterionId id_;
    std::string name_;
    Score max_;
    Weight weight_;
};

class Rubric {
public:
    Criterion add(std::string name, Score max, Weight weight) {
        for (const auto& c : criteria_) {
            if (c.name() == name) {
                throw DuplicateCriterion("Критерий \"" + name + "\" уже есть в рубрикаторе");
            }
        }

        Criterion criterion(CriterionId{next_}, std::move(name), max, weight);
        criteria_.push_back(std::move(criterion));
        ++next_;
        return criteria_.back();
    }

    const Criterion& get(CriterionId id) const {
        for (const auto& c : criteria_) {
            if (c.id() == id) {
                return c;
            }
        }
        throw UnknownCriterion("Критерия №" + std::to_string(id.value()) + " нет в рубрикаторе");
    }

    const std::vector<Criterion>& criteria() const { return criteria_; }
    double totalWeight() const {
        double sum = 0;
        for (const auto& c : criteria_) {
            sum += c.weight().value();
        }
        return sum;
    }

private:
    std::vector<Criterion> criteria_;
    int next_ = 1;
};

}  // namespace pos::domain

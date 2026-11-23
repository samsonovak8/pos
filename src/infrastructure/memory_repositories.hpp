#pragma once

#include <algorithm>
#include <optional>
#include <vector>

#include "domain/ports.hpp"

namespace pos::infrastructure {

class InMemoryStudents final : public domain::IStudentRepository {
public:
    domain::StudentId nextId() override { return domain::StudentId{next_++}; }
    void add(const domain::Student& s) override {
        if (std::ranges::any_of(students_, [&](const auto& x) { return x.id() == s.id(); })) {
            throw domain::DuplicateStudent("Студент с таким id уже есть");
        }
        students_.push_back(s);
        if (s.id().value() >= next_) {
            next_ = s.id().value() + 1;
        }
    }
    std::optional<domain::Student> find(domain::StudentId id) const override {
        const auto it = std::ranges::find_if(students_, [&](const auto& s) { return s.id() == id; });
        return it == students_.end() ? std::nullopt : std::optional(*it);
    }
    std::vector<domain::Student> all() const override { return students_; }

private:
    std::vector<domain::Student> students_;
    int next_ = 1;
};

class InMemoryRubric final : public domain::IRubricRepository {
public:
    domain::Rubric load() const override { return rubric_; }
    void save(const domain::Rubric& r) override { rubric_ = r; }

private:
    domain::Rubric rubric_;
};

class InMemoryGradebook final : public domain::IGradebookRepository {
public:
    domain::Gradebook load() const override { return book_; }
    void save(const domain::Gradebook& g) override { book_ = g; }

private:
    domain::Gradebook book_;
};

}  // namespace pos::infrastructure

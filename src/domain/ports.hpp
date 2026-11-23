#pragma once

#include <optional>
#include <vector>

#include "domain/evaluation_factory.hpp"
#include "domain/model.hpp"

namespace pos::domain {

class IStudentRepository {
public:
    virtual ~IStudentRepository() = default;
    virtual StudentId nextId() = 0;
    virtual void add(const Student& student) = 0;
    virtual std::optional<Student> find(StudentId id) const = 0;
    virtual std::vector<Student> all() const = 0;
};

class IRubricRepository {
public:
    virtual ~IRubricRepository() = default;
    virtual Rubric load() const = 0;
    virtual void save(const Rubric& rubric) = 0;
};

class IGradebookRepository {
public:
    virtual ~IGradebookRepository() = default;
    virtual Gradebook load() const = 0;
    virtual void save(const Gradebook& gradebook) = 0;
};

}  // namespace pos::domain

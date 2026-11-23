#pragma once

#include <limits>

#include "domain/error.hpp"

namespace pos::domain {

template <class Tag>
class Id {
public:
    explicit Id(int value) : value_(value) {
        if (value <= 0) {
            throw InvalidValue("Номер должен быть положительным числом");
        }
    }
    int value() const { return value_; }
    friend bool operator==(const Id&, const Id&) = default;
    friend auto operator<=>(const Id&, const Id&) = default;

private:
    int value_;
};

using StudentId = Id<struct StudentTag>;
using CriterionId = Id<struct CriterionTag>;

class Score {
public:
    explicit Score(double value) : value_(value) {
        if (!(value < std::numeric_limits<double>::max()) || value < 0) {
            throw InvalidValue("Балл должен быть неотрицательным числом");
        }
    }
    double value() const { return value_; }
    bool operator==(const Score&) const = default;

private:
    double value_;
};

class Weight {
public:
    explicit Weight(double value) : value_(value) {
        if (!(value < std::numeric_limits<double>::max()) || value <= 0) {
            throw InvalidValue("Вес критерия должен быть положительным числом");
        }
    }
    double value() const { return value_; }
    bool operator==(const Weight&) const = default;

private:
    double value_;
};

}  // namespace pos::domain
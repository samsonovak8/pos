#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "domain/evaluation_factory.hpp"

namespace pos::domain {

class FinalGrade {
public:
    FinalGrade(double percent, std::size_t graded, std::size_t total)
        : percent_(percent), graded_(graded), total_(total) {
        if (percent < 0 || percent > 100) {
            throw InvalidValue("Итог должен лежать в диапазоне [0, 100]");
        }

        if (graded > total) {
            throw InvalidValue("Оценённых критериев не может быть больше, чем критериев в рубрикаторе");
        }
    }
    double percent() const { return percent_; }
    std::size_t graded() const { return graded_; }
    std::size_t total() const { return total_; }
    bool operator==(const FinalGrade&) const = default;

private:
    double percent_;
    std::size_t graded_, total_;
};

class IGradingStrategy {
public:
    virtual ~IGradingStrategy() = default;
    virtual double percent(const Rubric& rubric, const std::vector<Evaluation>& evaluations) const = 0;
    virtual std::string name() const = 0;
};

namespace detail {

inline std::pair<double, double> weighted(const Rubric& rubric, const std::vector<Evaluation>& evaluations) {
    double sum = 0, weights = 0;
    for (const auto& e : evaluations) {
        const Criterion& c = rubric.get(e.criterion());
        sum += c.weight().value() * e.score().value() / c.max().value();
        weights += c.weight().value();
    }
    return {sum, weights};
}
}  // namespace detail

class WeightedSum final : public IGradingStrategy {
public:
    double percent(const Rubric& r, const std::vector<Evaluation>& e) const override {
        const double total = r.totalWeight();
        return total > 0 ? 100 * detail::weighted(r, e).first / total : 0;
    }

    std::string name() const override {
        return "Взвешенная сумма (невыставленная оценка = 0)";
    }
};

class WeightedAverageOfGraded final : public IGradingStrategy {
public:
    double percent(const Rubric& r, const std::vector<Evaluation>& e) const override {
        const auto [sum, weights] = detail::weighted(r, e);
        return weights > 0 ? 100 * sum / weights : 0;
    }
    std::string name() const override {
        return "Взвешенное среднее по выставленным оценкам";
    }
};

class IFinalGradeCalculator {
public:
    virtual ~IFinalGradeCalculator() = default;
    virtual void use(std::shared_ptr<const IGradingStrategy> strategy) = 0;
    virtual const IGradingStrategy& strategy() const = 0;
    virtual FinalGrade calculate(const Rubric& rubric, const Gradebook& book, StudentId student) const = 0;
};

class FinalGradeCalculator final : public IFinalGradeCalculator {
public:
    explicit FinalGradeCalculator(std::shared_ptr<const IGradingStrategy> strategy) {
        use(std::move(strategy));
    }
    const IGradingStrategy& strategy() const override { return *strategy_; }

    void use(std::shared_ptr<const IGradingStrategy> strategy) override {
        if (!strategy) {
            throw InvalidValue("Стратегия расчёта итога не задана");
        }
        strategy_ = std::move(strategy);
    }

    FinalGrade calculate(const Rubric& rubric, const Gradebook& book, StudentId student) const override {
        const auto evaluations = book.of(student);
        const double raw = strategy_->percent(rubric, evaluations);
        constexpr double eps = 1e-9;
        if (raw < -eps || raw > 100 + eps) {
            throw InvalidValue("Стратегия \"" + strategy_->name() + "\" вернула процент вне [0, 100]: " + num(raw));
        }
        return FinalGrade(std::clamp(raw, 0.0, 100.0), evaluations.size(), rubric.criteria().size());
    }

private:
    std::shared_ptr<const IGradingStrategy> strategy_;
};

}  // namespace pos::domain

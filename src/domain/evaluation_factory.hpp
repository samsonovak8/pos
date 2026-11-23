#pragma once

#include <map>
#include <vector>
#include <utility>

#include "domain/model.hpp"

namespace pos::domain {

class EvaluationFactory;

class Evaluation {
public:
    StudentId student() const { return student_; }
    CriterionId criterion() const { return criterion_; }
    Score score() const { return score_; }
    Score max() const { return max_; }

private:
    friend class EvaluationFactory;
    Evaluation(StudentId s, CriterionId c, Score score, Score max) : student_(s), criterion_(c), score_(score), max_(max) {}
    StudentId student_;
    CriterionId criterion_;
    Score score_, max_;
};

class EvaluationFactory {
public:
    static Evaluation create(const Student& student, const Rubric& rubric, CriterionId criterion, Score score) {
        const Criterion& c = rubric.get(criterion);
        if (score.value() > c.max().value()) {
            throw InvalidValue("Балл " + num(score.value()) + " выходит за пределы критерия [0, " + num(c.max().value()) + "]");
        }
        return Evaluation(student.id(), c.id(), score, c.max());
    }
};

class Gradebook {
public:
    void add(const Evaluation& e) {
        if (!items_.emplace(key(e), e).second) {
            throw DuplicateEvaluation("Оценка по этому студенту и критерию уже выставлена - используйте исправление");
        }
    }

    void replace(const Evaluation& e) {
        const auto it = items_.find(key(e));
        if (it == items_.end()) {
            throw EvaluationNotFound("Оценка не найдена");
        }
        it->second = e;
    }

    void revoke(StudentId s, CriterionId c) {
        if (items_.erase({s, c}) == 0) {
            throw EvaluationNotFound("Оценка не найдена");
        }
    }

    std::vector<Evaluation> of(StudentId s) const {
        std::vector<Evaluation> result;
        for (const auto& [k, e] : items_) {
            if (k.first == s) {
                result.push_back(e);
            }
        }
        return result;
    }

    std::vector<Evaluation> all() const {
        std::vector<Evaluation> result;
        for (const auto& [k, e] : items_) {
            result.push_back(e);
        }
        return result;
    }

private:
    using Key = std::pair<StudentId, CriterionId>;
    static Key key(const Evaluation& e) { return {e.student(), e.criterion()}; }
    std::map<Key, Evaluation> items_;
};

}  // namespace pos::domain
#pragma once

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "domain/grading.hpp"
#include "domain/ports.hpp"
#include "domain/error.hpp"

namespace pos::application {

struct StudentResult {
    domain::Student student;
    domain::FinalGrade grade;
};

struct EvaluationLine {
    std::string student, criterion;
    double score, max;
};

class IResultExporter {
public:
    virtual ~IResultExporter() = default;
    virtual void write(const std::vector<StudentResult>& results, std::ostream& out) const = 0;
};

class PosFacade {
public:
    PosFacade(std::shared_ptr<domain::IStudentRepository> students, std::shared_ptr<domain::IRubricRepository> rubric,
              std::shared_ptr<domain::IGradebookRepository> gradebook,
              std::shared_ptr<domain::IFinalGradeCalculator> calculator,
              std::vector<std::shared_ptr<const domain::IGradingStrategy>> strategies,
              std::shared_ptr<IResultExporter> exporter);

    domain::Student addStudent(std::string name);
    std::vector<domain::Student> students() const { return students_->all(); }

    domain::Criterion addCriterion(std::string name, double max, double weight);
    std::vector<domain::Criterion> criteria() const { return rubric_->load().criteria(); }

    void grade(domain::StudentId student, domain::CriterionId criterion, double score);
    void correct(domain::StudentId student, domain::CriterionId criterion, double score);
    void revoke(domain::StudentId student, domain::CriterionId criterion);

    std::vector<EvaluationLine> evaluations() const;

    StudentResult finalGrade(domain::StudentId id) const {
        return {requireStudent(id), calculator_->calculate(rubric_->load(), gradebook_->load(), id)};
    }

    std::vector<StudentResult> finalGrades() const;

    void selectRule(std::size_t index);
    void selectRule(const std::string& name);

    std::vector<std::string> ruleNames() const;

    std::string currentRule() const { return calculator_->strategy().name(); }

    void exportFinals(std::ostream& out) const { exporter_->write(finalGrades(), out); }

private:
    domain::Student requireStudent(domain::StudentId id) const {
        if (auto s = students_->find(id)) {
            return *s;
        }
        throw domain::UnknownStudent("Студента №" + std::to_string(id.value()) + " не существует");
    }

    std::shared_ptr<domain::IStudentRepository> students_;
    std::shared_ptr<domain::IRubricRepository> rubric_;
    std::shared_ptr<domain::IGradebookRepository> gradebook_;
    std::shared_ptr<domain::IFinalGradeCalculator> calculator_;
    std::vector<std::shared_ptr<const domain::IGradingStrategy>> strategies_;
    std::shared_ptr<IResultExporter> exporter_;
};

}  // namespace pos::application

#include "facade.hpp"

namespace pos::application {
PosFacade::PosFacade(std::shared_ptr<domain::IStudentRepository> students,
            std::shared_ptr<domain::IRubricRepository> rubric,
            std::shared_ptr<domain::IGradebookRepository> gradebook,
            std::shared_ptr<domain::IFinalGradeCalculator> calculator,
            std::vector<std::shared_ptr<const domain::IGradingStrategy>> strategies,
            std::shared_ptr<IResultExporter> exporter)
    : students_(std::move(students)), rubric_(std::move(rubric)), gradebook_(std::move(gradebook)),
        calculator_(std::move(calculator)), strategies_(std::move(strategies)), exporter_(std::move(exporter)) {

    if (!students_ || !rubric_ || !gradebook_ || !calculator_ || !exporter_) {
        throw domain::InvalidValue("Не все зависимости PosFacade заданы");
    }

    if (strategies_.empty()) {
        throw domain::InvalidValue("Нет ни одной стратегии итога");
    }

    for (const auto& strategy : strategies_) {
        if (!strategy) {
            throw domain::InvalidValue("Стратегия итога не задана");
        }
    }

    calculator_->use(strategies_.front());
}

domain::Student PosFacade::addStudent(std::string name) {
    domain::Student student(students_->nextId(), std::move(name));
    students_->add(student);
    return student;
}

domain::Criterion PosFacade::addCriterion(std::string name, double max, double weight) {
    auto rubric = rubric_->load();
    const domain::Criterion c = rubric.add(std::move(name), domain::Score(max), domain::Weight(weight));
    rubric_->save(rubric);
    return c;
}

void PosFacade::grade(domain::StudentId student, domain::CriterionId criterion, double score) {
    auto book = gradebook_->load();
    book.add(domain::EvaluationFactory::create(requireStudent(student), rubric_->load(), criterion, domain::Score(score)));
    gradebook_->save(book);
}

void PosFacade::correct(domain::StudentId student, domain::CriterionId criterion, double score) {
    auto book = gradebook_->load();
    book.replace(domain::EvaluationFactory::create(requireStudent(student), rubric_->load(), criterion, domain::Score(score)));
    gradebook_->save(book);
}

void PosFacade::revoke(domain::StudentId student, domain::CriterionId criterion) {
    auto book = gradebook_->load();
    book.revoke(student, criterion);
    gradebook_->save(book);
}

std::vector<EvaluationLine> PosFacade::evaluations() const {
    const auto rubric = rubric_->load();
    std::vector<EvaluationLine> lines;
    for (const auto& e : gradebook_->load().all()) {
        lines.push_back({requireStudent(e.student()).name(), rubric.get(e.criterion()).name(), e.score().value(), e.max().value()});
    }
    return lines;
}

std::vector<StudentResult> PosFacade::finalGrades() const {
    const auto rubric = rubric_->load();
    const auto book = gradebook_->load();
    std::vector<StudentResult> results;
    for (const auto& s : students_->all()) {
        results.push_back({s, calculator_->calculate(rubric, book, s.id())});
    }
    return results;
}

void PosFacade::selectRule(std::size_t index) {
    if (index >= strategies_.size()) {
        throw domain::InvalidValue("Нет такого правила итога");
    }
    calculator_->use(strategies_[index]);
}

void PosFacade::selectRule(const std::string& name) {
    for (const auto& strategy : strategies_) {
        if (strategy->name() == name) {
            calculator_->use(strategy);
            return;
        }
    }
    throw domain::InvalidValue("Нет правила итога \"" + name + "\"");
}

std::vector<std::string> PosFacade::ruleNames() const {
    std::vector<std::string> names;
    names.reserve(strategies_.size());
    for (const auto& strategy : strategies_) {
        names.push_back(strategy->name());
    }
    return names;
}

}  // namespace pos::application
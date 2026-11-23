#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "composition_root.hpp"
#include "ui/menu.hpp"

using namespace pos;
using namespace pos::domain;

class FacadeTest : public ::testing::Test {
protected:
    std::shared_ptr<application::PosFacade> f = buildFacade();
};

class DomainTest : public ::testing::Test {
protected:
    void SetUp() override {
        test = rubric.add("Тест", Score(10), Weight(2)).id();
        project = rubric.add("Проект", Score(20), Weight(3)).id();
    }

    Evaluation make(CriterionId criterion, double score) const {
        return EvaluationFactory::create(ivan, rubric, criterion, Score(score));
    }

    Rubric rubric;
    Student ivan{StudentId{1}, "Иван"};
    CriterionId test{1}, project{2};
};

// Value objs basic checks
TEST(Values, RejectsInvalidScore) {
    EXPECT_THROW(Score(-1), InvalidValue);
    EXPECT_THROW(Score(1.7976931348623157e+308), InvalidValue);
    EXPECT_NO_THROW(Score(0));
}

TEST(Values, RejectsInvalidWeight) {
    EXPECT_THROW(Weight(0), InvalidValue);
    EXPECT_THROW(Weight(-2), InvalidValue);
}

TEST(Values, StudentNameMustNotBeBlank) {
    EXPECT_THROW(Student(StudentId{2}, "  "), InvalidValue);
}

TEST(Values, RejectInvalidCriterion) {
    Rubric r;
    EXPECT_THROW(r.add("Тест", Score(0), Weight(1)), InvalidValue);
}

TEST(Values, IdMustBePositive) {
    EXPECT_THROW(StudentId{0}, InvalidValue);
    EXPECT_THROW(CriterionId{-5}, InvalidValue);
}

TEST(Values, RejectsInvalidFinalGrade) {
    EXPECT_THROW(FinalGrade(150, 1, 2), InvalidValue);
    EXPECT_THROW(FinalGrade(-1, 1, 2), InvalidValue);
    EXPECT_THROW(FinalGrade(50, 3, 2), InvalidValue);
    EXPECT_NO_THROW(FinalGrade(100, 2, 2));
}

namespace {
struct FixedStrategy final : IGradingStrategy {
    explicit FixedStrategy(double value) : value_(value) {}
    double percent(const Rubric&, const std::vector<Evaluation>&) const override { return value_; }
    std::string name() const override { return "фиксированная"; }
    double value_;
};
}  // namespace

// Domain objs basic checks
TEST_F(DomainTest, RejectsOutOfRangePercent) {
    Gradebook book;
    for (double bad : {-1.0, 101.0, HUGE_VAL}) {
        FinalGradeCalculator calc(std::make_shared<FixedStrategy>(bad));
        EXPECT_THROW(calc.calculate(rubric, book, ivan.id()), InvalidValue);
    }
}

TEST_F(DomainTest, ToleratesBoundaryNoise) {
    Gradebook book;
    FinalGradeCalculator high(std::make_shared<FixedStrategy>(100.0 + 1e-12));
    FinalGradeCalculator low(std::make_shared<FixedStrategy>(-1e-12));
    EXPECT_EQ(high.calculate(rubric, book, ivan.id()).percent(), 100.0);
    EXPECT_EQ(low.calculate(rubric, book, ivan.id()).percent(), 0.0);
}

TEST(Values, RejectsNullStrategy) {
    EXPECT_THROW(FinalGradeCalculator(nullptr), InvalidValue);
}

TEST_F(DomainTest, RejectsDuplicateCriterion) {
    EXPECT_THROW(rubric.add("Тест", Score(5), Weight(1)), DuplicateCriterion);
    EXPECT_EQ(rubric.criteria().size(), 2u);
}

TEST_F(DomainTest, RejectsUnknownCriterion) {
    EXPECT_THROW(rubric.get(CriterionId{99}), UnknownCriterion);
}

TEST_F(DomainTest, RejectsScoreAboveMax) {
    EXPECT_THROW(make(test, 10.5), InvalidValue);
}

TEST_F(DomainTest, AcceptsBoundaryScore) {
    EXPECT_EQ(make(test, 0).score().value(), 0);
    EXPECT_EQ(make(test, 10).score().value(), 10);
}

TEST_F(DomainTest, RejectsForeignCriterion) {
    EXPECT_THROW(make(CriterionId{99}, 1), UnknownCriterion);
}

// Simple scenarios
TEST_F(DomainTest, RejectsDuplicateEvaluation) {
    Gradebook book;
    book.add(make(test, 5));
    EXPECT_THROW(book.add(make(test, 6)), DuplicateEvaluation);
}

TEST_F(DomainTest, ReplacesAndRevokes) {
    Gradebook book;
    book.add(make(test, 5));
    book.replace(make(test, 10));
    ASSERT_EQ(book.of(ivan.id()).size(), 1u);
    EXPECT_EQ(book.of(ivan.id())[0].score().value(), 10);
    book.revoke(ivan.id(), test);
    EXPECT_TRUE(book.of(ivan.id()).empty());
}

TEST_F(DomainTest, RejectsMissingEvaluation) {
    Gradebook book;
    EXPECT_THROW(book.replace(make(test, 5)), EvaluationNotFound);
    EXPECT_THROW(book.revoke(ivan.id(), test), EvaluationNotFound);
}

// Not very simple scenarios
TEST_F(DomainTest, CountsUngradedAsZero) {
    Gradebook book;
    book.add(make(test, 10));
    FinalGradeCalculator calc(std::make_shared<WeightedSum>());
    const auto grade = calc.calculate(rubric, book, ivan.id());
    EXPECT_DOUBLE_EQ(grade.percent(), 40.0);
    EXPECT_EQ(grade.graded(), 1u);
    EXPECT_EQ(grade.total(), 2u);
}

TEST_F(DomainTest, IgnoresUngraded) {
    Gradebook book;
    book.add(make(test, 10));
    FinalGradeCalculator calc(std::make_shared<WeightedAverageOfGraded>());
    const auto grade = calc.calculate(rubric, book, ivan.id());
    EXPECT_DOUBLE_EQ(grade.percent(), 100.0);
    EXPECT_EQ(grade.graded(), 1u);
    EXPECT_EQ(grade.total(), 2u);
}

TEST_F(DomainTest, CorrectionUpdates) {
    Gradebook book;
    book.add(make(test, 5));
    FinalGradeCalculator calc(std::make_shared<WeightedAverageOfGraded>());
    const auto before = calc.calculate(rubric, book, ivan.id());
    book.replace(make(test, 10));
    const auto after = calc.calculate(rubric, book, ivan.id());
    EXPECT_DOUBLE_EQ(before.percent(), 50.0);
    EXPECT_DOUBLE_EQ(after.percent(), 100.0);
}

TEST_F(DomainTest, RevokeUpdates) {
    Gradebook book;
    book.add(make(test, 5));
    FinalGradeCalculator calc(std::make_shared<WeightedAverageOfGraded>());
    book.revoke(ivan.id(), test);
    const auto grade = calc.calculate(rubric, book, ivan.id());
    EXPECT_EQ(grade.graded(), 0u);
    EXPECT_DOUBLE_EQ(grade.percent(), 0.0);
}

// Facade tests
TEST_F(FacadeTest, EndToEndExport) {
    const auto s = f->addStudent("Иван");
    const auto c = f->addCriterion("Тест", 10, 1);
    f->grade(s.id(), c.id(), 10);

    const auto grade = f->finalGrade(s.id());
    EXPECT_DOUBLE_EQ(grade.grade.percent(), 100.0);
    EXPECT_EQ(grade.grade.graded(), 1u);
    EXPECT_EQ(grade.grade.total(), 1u);

    std::ostringstream csv;
    f->exportFinals(csv);
    EXPECT_NE(csv.str().find("Иван"), std::string::npos);
    EXPECT_NE(csv.str().find("100.00"), std::string::npos);
}

TEST_F(FacadeTest, FailedGradeKeepsState) {
    const auto s = f->addStudent("Иван");
    const auto c = f->addCriterion("Тест", 10, 1);
    EXPECT_THROW(f->grade(s.id(), c.id(), 11), InvalidValue);
    EXPECT_TRUE(f->evaluations().empty());
    EXPECT_EQ(f->finalGrade(s.id()).grade.graded(), 0u);
}

TEST_F(FacadeTest, RuleSelectionChangesFinal) {
    const auto s = f->addStudent("Иван");
    const auto a = f->addCriterion("A", 10, 1);
    f->addCriterion("B", 10, 3);
    f->grade(s.id(), a.id(), 10);

    EXPECT_DOUBLE_EQ(f->finalGrade(s.id()).grade.percent(), 25.0);
    ASSERT_EQ(f->ruleNames().size(), 2u);
    f->selectRule(1);
    EXPECT_DOUBLE_EQ(f->finalGrade(s.id()).grade.percent(), 100.0);
}

// CSV checks
TEST_F(FacadeTest, CsvEscapesCommaAndQuote) {
    f->addStudent("Иван, \"И\"");
    std::ostringstream csv;
    f->exportFinals(csv);
    EXPECT_NE(csv.str().find("\"Иван, \"\"И\"\"\""), std::string::npos);
}

TEST_F(FacadeTest, CsvEscapesCrAndFormats) {
    const auto s = f->addStudent("Анна\rИванова");
    const auto c = f->addCriterion("Тест", 10, 1);
    f->grade(s.id(), c.id(), 0.0001);
    std::ostringstream csv;
    f->exportFinals(csv);
    EXPECT_EQ(csv.str(), "student,final_percent,graded,total\n\"Анна\rИванова\",0.00,1,1\n");
}

namespace {
int occurrences(const std::string& text, const std::string& part) {
    int n = 0;
    for (auto pos = text.find(part); pos != std::string::npos; pos = text.find(part, pos + part.size())) ++n;
    return n;
}
}  // namespace

// Menu checks
TEST_F(FacadeTest, RejectsNonIntegerInput) {
    std::istringstream in("nan\n1e999\n2.5\n0\n");
    std::ostringstream out;
    ui::runMenu(*f, in, out);
    EXPECT_EQ(occurrences(out.str(), "Нужно целое число"), 3);
}

TEST_F(FacadeTest, AcceptsCommaDecimal) {
    std::istringstream in("1\nИван\n3\nТест\n10\n1\n5\n1\n1\n8,5\n0\n");
    std::ostringstream out;
    ui::runMenu(*f, in, out);
    ASSERT_EQ(f->evaluations().size(), 1u);
    EXPECT_EQ(f->evaluations()[0].score, 8.5);
}

TEST_F(FacadeTest, ReportInvalidId) {
    std::istringstream in("5\n0\n0\n");
    std::ostringstream out;
    ui::runMenu(*f, in, out);
    EXPECT_NE(out.str().find("Ошибка: Номер должен быть положительным"), std::string::npos);
}

TEST_F(FacadeTest, SelectsRuleFromMenu) {
    std::istringstream in(
        "1\nИван\n"
        "3\nA\n10\n1\n"
        "3\nB\n10\n3\n"
        "5\n1\n1\n10\n"
        "10\n2\n"
        "9\n"
        "0\n");
    std::ostringstream out;
    ui::runMenu(*f, in, out);
    EXPECT_NE(out.str().find("Взвешенное среднее по выставленным оценкам"), std::string::npos);
    EXPECT_NE(out.str().find("Иван - 100%"), std::string::npos);
}
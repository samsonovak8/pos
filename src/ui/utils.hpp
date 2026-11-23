#pragma once

#include <fstream>
#include <ostream>
#include <string>

#include "application/facade.hpp"

namespace pos::ui {

using application::PosFacade;

void printStudents(const PosFacade& f, std::ostream& out) {
    for (const auto& s : f.students()) {
        out << "  №" << s.id().value() << "  " << s.name() << "\n";
    }
}

void printCriteria(const PosFacade& f, std::ostream& out) {
    for (const auto& c : f.criteria()) {
        out << "  №" << c.id().value() << "  " << c.name() << " - максимум " << c.max().value() << ", вес "
            << c.weight().value() << "\n";
    }
}

void printEvaluations(const PosFacade& f, std::ostream& out) {
    for (const auto& e : f.evaluations()) {
        out << "  " << e.student << " | " << e.criterion << ": " << e.score << " / " << e.max << "\n";
    }
}

void printFinals(const PosFacade& f, std::ostream& out) {
    out << "Правило: " << f.currentRule() << "\n";
    for (const auto& r : f.finalGrades()) {
        out << "  " << r.student.name() << " - " << r.grade.percent() << "% (оценено " << r.grade.graded() << "/"
            << r.grade.total() << ")\n";
    }
}

bool saveFinals(const PosFacade& f, const std::string& path) {
    std::ofstream file(path);
    if (!file) {
        return false;
    }
    f.exportFinals(file);
    return static_cast<bool>(file.flush());
}
}  // namespace pos::ui
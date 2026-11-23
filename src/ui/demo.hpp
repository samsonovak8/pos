#pragma once

#include <iomanip>
#include <ostream>

#include "ui/utils.hpp"

namespace pos::ui {

using application::PosFacade;
using domain::CriterionId;

void runDemo(PosFacade& f, std::ostream& out) {
    out << std::setprecision(4);
    const auto step = [&](const char* title) { out << "\n=== " << title << " ===\n"; };

    step("1. Студенты и критерии");
    const auto ivanov = f.addStudent("Иванов Иван");
    const auto petrova = f.addStudent("Петрова Анна");
    const auto test = f.addCriterion("Контрольная", 20, 3);
    const auto labs = f.addCriterion("Лабораторные", 10, 2);
    const auto project = f.addCriterion("Проект", 30, 4);
    printStudents(f, out);
    printCriteria(f, out);

    step("2. Оценки");
    f.grade(ivanov.id(), test.id(), 18);
    f.grade(ivanov.id(), labs.id(), 9);
    f.grade(ivanov.id(), project.id(), 27);
    f.grade(petrova.id(), test.id(), 20);
    f.grade(petrova.id(), labs.id(), 7);
    printEvaluations(f, out);

    step("3. Итог");
    printFinals(f, out);
    f.selectRule("Взвешенное среднее по выставленным оценкам");
    printFinals(f, out);

    step("4. Экспорт итога");
    f.exportFinals(out);
}

}  // namespace pos::ui
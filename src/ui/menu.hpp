#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "application/facade.hpp"
#include "ui/utils.hpp"

namespace pos::ui {

using application::PosFacade;
using domain::CriterionId;
using domain::StudentId;

void runMenu(PosFacade& f, std::istream& in, std::ostream& out) {
    out << std::setprecision(4);
    const auto ask = [&](const char* prompt) {
        out << prompt << std::flush;
        std::string line;
        if (!std::getline(in, line)) {
            throw std::ios_base::failure("конец ввода");
        }
        return line;
    };
    const auto number = [&](const char* prompt) {
        for (;;) {
            try {
                std::string text = ask(prompt);
                std::replace(text.begin(), text.end(), ',', '.');
                std::size_t used = 0;
                const double value = std::stod(text, &used);
                if (used == text.find_last_not_of(" \t\r") + 1 && std::isfinite(value)) {
                    return value;
                }
            } catch (const std::logic_error&) {}
            out << "Нужно число, попробуйте ещё раз.\n";
        }
    };

    const auto integer = [&](const char* prompt) {
        for (;;) {
            try {
                const std::string text = ask(prompt);
                std::size_t used = 0;
                const int value = std::stoi(text, &used);
                if (used == text.find_last_not_of(" \t\r") + 1) return value;
            } catch (const std::logic_error&) {}
            out << "Нужно целое число, попробуйте ещё раз.\n";
        }
    };
    const auto studentId = [&] { return StudentId{integer("Номер студента: ")}; };
    const auto criterionId = [&] { return CriterionId{integer("Номер критерия: ")}; };
    const auto rule = [&] {
        const auto names = f.ruleNames();
        if (names.empty()) {
            out << "Нет доступных правил итога\n";
            return;
        }
        out << "Правила итога:\n";
        for (std::size_t i = 0; i < names.size(); ++i)
            out << "  " << (i + 1) << " - " << names[i] << "\n";
        const int choice = integer("Правило: ");
        if (choice < 1 || static_cast<std::size_t>(choice) > names.size()) {
            out << "Нет такого правила\n";
            return;
        }
        f.selectRule(static_cast<std::size_t>(choice - 1));
        out << "Правило: " << f.currentRule() << "\n";
    };

    try {
        for (;;) {
            out << "\n"
                " 1. Добавить студента\n"
                " 2. Список студентов\n"
                " 3. Добавить критерий\n"
                " 4. Рубрикатор\n"
                " 5. Выставить оценку\n"
                " 6. Исправить оценку\n"
                " 7. Отменить оценку\n"
                " 8. Показать оценки\n"
                " 9. Итоги\n"
                "10. Правило итога\n"
                "11. Экспорт итогов (CSV)\n"
                " 0. Выход\n";
            const int choice = integer("Выбор: ");
            if (choice == 0) return;
            try {
                switch (choice) {
                    case 1: {
                        const auto name = ask("Имя: ");
                        out << "Добавлен студент №" << f.addStudent(name).id().value() << "\n";
                        break;
                    }
                    case 2: printStudents(f, out); break;
                    case 3: {
                        const auto name = ask("Название: ");
                        const double max = number("Максимальный балл: ");
                        f.addCriterion(name, max, number("Вес: "));
                        out << "Критерий добавлен\n";
                        break;
                    }
                    case 4: printCriteria(f, out); break;
                    case 5: {
                        const auto s = studentId();
                        const auto c = criterionId();
                        f.grade(s, c, number("Балл: "));
                        out << "Оценка выставлена\n";
                        break;
                    }
                    case 6: {
                        const auto s = studentId();
                        const auto c = criterionId();
                        f.correct(s, c, number("Новый балл: "));
                        out << "Оценка исправлена\n";
                        break;
                    }
                    case 7: {
                        const auto s = studentId();
                        f.revoke(s, criterionId());
                        out << "Оценка отменена\n";
                        break;
                    }
                    case 8: printEvaluations(f, out); break;
                    case 9: printFinals(f, out); break;
                    case 10: rule(); break;
                    case 11: {
                        std::string path = ask("Файл (Enter - results.csv): ");
                        if (path.empty()) {
                            path = "results.csv";
                        }
                        out << (saveFinals(f, path) ? "Итоги сохранены в " + path : "Не удалось записать файл " + path) << "\n";
                        break;
                    }
                    default: out << "Нет такого пункта\n";
                }
            } catch (const domain::DomainError& e) {
                out << "Ошибка: " << e.what() << "\n";
            }
        }
    } catch (const std::ios_base::failure&) {
        out << "\nВвод завершён.\n";
    }
}

}  // namespace pos::ui

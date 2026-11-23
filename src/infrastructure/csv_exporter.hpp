#pragma once

#include <iomanip>

#include "application/facade.hpp"

namespace pos::infrastructure {

class CsvExporter final : public application::IResultExporter {
public:
    void write(const std::vector<application::StudentResult>& results, std::ostream& out) const override {
        out << "student,final_percent,graded,total\n" << std::fixed << std::setprecision(2);
        for (const auto& r : results) {
            std::string name = r.student.name();
            if (name.find_first_of(",\"\r\n") != std::string::npos) {
                std::string quoted = "\"";
                for (char ch : name) {
                    quoted += (ch == '"') ? std::string("\"\"") : std::string(1, ch);
                }
                name = quoted + "\"";
            }
            out << name << ',' << r.grade.percent() << ',' << r.grade.graded() << ','
                << r.grade.total() << '\n';
        }
    }
};

}  // namespace pos::infrastructure

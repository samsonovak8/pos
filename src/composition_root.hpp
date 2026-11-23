#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "application/facade.hpp"
#include "infrastructure/csv_exporter.hpp"
#include "infrastructure/di_container.hpp"
#include "infrastructure/memory_repositories.hpp"

namespace pos {

inline std::shared_ptr<application::PosFacade> buildFacade() {

    using namespace application;
    using namespace domain;
    using namespace infrastructure;

    Container c;
    c.singleton<IStudentRepository>([](Container&) { return new InMemoryStudents; });
    c.singleton<IRubricRepository>([](Container&) { return new InMemoryRubric; });
    c.singleton<IGradebookRepository>([](Container&) { return new InMemoryGradebook; });
    c.singleton<IResultExporter>([](Container&) { return new CsvExporter; });
    c.singleton<IFinalGradeCalculator>([](Container&) {
        return new FinalGradeCalculator(std::make_shared<WeightedSum>());
    });
    c.singleton<PosFacade>([](Container& k) {
        std::vector<std::shared_ptr<const IGradingStrategy>> strategies{
            std::make_shared<WeightedSum>(),
            std::make_shared<WeightedAverageOfGraded>()};
        return new PosFacade(k.resolve<IStudentRepository>(), k.resolve<IRubricRepository>(),
                             k.resolve<IGradebookRepository>(), k.resolve<IFinalGradeCalculator>(),
                             std::move(strategies), k.resolve<IResultExporter>());
    });

    return c.resolve<PosFacade>();
}

}  // namespace pos

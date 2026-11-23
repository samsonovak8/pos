# Модуль "ПОС" - Преподавательская Оценка Студентов

Консольное приложение согласно [ТЗ](МФТИ-ПАПО-ДЗ-1-ПОС.pdf)

## Запуск

```bash
cmake -S . -B build && cmake --build build

./build/pos --demo     # сквозной сценарий: завести студентов и критерии -> выставить оценки -> итог -> экспорт итога
./build/pos            # интерактивное меню
./build/pos_tests      # юнит-тесты
```

## Карта DDD

**Ubiquitous Language** 

Термины из ТЗ соответствуют коду: [`Student`](src/domain/model.hpp#18), [`Criterion`](src/domain/model.hpp#33),
[`Rubric`](src/domain/model.hpp#57), [`Evaluation`](src/domain/evaluation_factory.hpp#13) (оценка), [`Gradebook`](src/domain/evaluation_factory.hpp#39) (журнал), [`FinalGrade`](src/domain/grading.hpp#14) (итог)

**Entity и Value Object**

| Тип | Вид | Почему |
|---|---|---|
| `Student` | Entity | Идентичность задаёт `StudentId`, а имя тут атрибут: два студента с одинаковым именем остаются разными людьми, и оценки ссылаются на студента по id |
| `Criterion` | Entity | Сущность внутри агрегата `Rubric`, на которую ссылаются оценки по `CriterionId`. Название, максимум и вес - состояние критерия, которое может меняться; при этом оценка должна остаться оценкой по тому же критерию. |
| `Evaluation` | Entity | Сущность внутри агрегата `Gradebook`. Идентичность оценки задаётся парой (`StudentId`, `CriterionId`): этому студенту по этому критерию выставлен балл. Балл - состояние, которое можно исправить, но запись остаётся той же; второй оценки по той же паре быть не может. |
| `Score`, `Weight`, `StudentId`, `CriterionId`, `FinalGrade` | Value Object | Неизменяемы, равны по значению, невалидный экземпляр не создаётся |

**Инварианты**

| Правило | Кто охраняет | Тест |
|---|---|---|
| балл не выходит за максимум критерия | `EvaluationFactory` | `RejectsScoreAboveMax`, `AcceptsBoundaryScore` |
| балл неотрицателен | `Score` | `RejectsInvalidScore` |
| оценка только по критерию из рубрикатора | `EvaluationFactory` через `Rubric::get` | `RejectsForeignCriterion`, `RejectsUnknownCriterion` |
| итог всегда пересчитывается из текущих оценок | `FinalGradeCalculator` | `CorrectionUpdates`, `RevokeUpdates`, `CountsUngradedAsZero`, `IgnoresUngraded` |
| стратегия расчёта задана | `FinalGradeCalculator` | `RejectsNullStrategy` |
| процент стратегии лежит в [0, 100] | `FinalGradeCalculator`, `FinalGrade` | `RejectsOutOfRangePercent`, `ToleratesBoundaryNoise`, `RejectsInvalidFinalGrade` |
| одна оценка на пару "студент + критерий" | `Gradebook` | `RejectsDuplicateEvaluation` |
| названия критериев уникальны | `Rubric` | `RejectsDuplicateCriterion` |
| нельзя заменить или отменить несуществующую оценку | `Gradebook` | `RejectsMissingEvaluation` |
| вес критерия > 0 | `Weight` | `RejectsInvalidWeight` |
| максимум критерия > 0 | `Criterion` | `RejectInvalidCriterion` |
| id положительный | `Id` | `IdMustBePositive` |
| итог в [0, 100], оценённых критериев не больше всех | `FinalGrade` | `RejectsInvalidFinalGrade` |
| имя студента не пустое | `Student` | `StudentNameMustNotBeBlank` |

**Repository** - [`domain/ports.hpp`](src/domain/ports.hpp): по одному маленькому интерфейсу на агрегат, тут домен не знает ни про память, ни про файлы и ни про консоль

**Domain Service** - `FinalGradeCalculator` ([`IFinalGradeCalculator`](src/domain/grading.hpp#79)): расчёт итога не принадлежит ни студенту, ни оценке, ни рубрикатору. Сервис проверяет контракт стратегии
и собирает `FinalGrade`

## Принципы SOLID

- **S - single responsibility**: домен хранит правила и инварианты, `PosFacade` описывает сценарии, репозитории отвечают за хранение, `CsvExporter` отвечает за формат, меню - за ввод/вывод
- **O - open/closed principle**: новая формула итога - это новый наследник `IGradingStrategy` и одна строка в  одна строка в списке стратегий внутри `buildFacade()`, `PosFacade` и меню при этом не меняются. Новый формат экспорта подключается аналогично, через `IResultExporter`
- **L - Liskov substitution principle**: `FinalGradeCalculator` работает только с `IGradingStrategy` и не знает, какая формула выбрана. `WeightedSum` и `WeightedAverageOfGraded` взаимозаменяемы: один и тот же вход, одинаковый контракт результата и нет дополнительных требований к окружению
- **I - interface segregation principle**: порты разделены по агрегатам (`IStudentRepository`, `IRubricRepository`, `IGradebookRepository`), экспорт вынесен в отдельный `IResultExporter`
- **D - dependency inversion principle**: `PosFacade` работает через интерфейсы репозиториев, `IFinalGradeCalculator`, `IResultExporter` и `IGradingStrategy`, а конкретные реализации собираются в [`composition_root.hpp`](src/composition_root.hpp). В тестах используется сборка с in-memory репозиториями, а отдельные зависимости подставляются вручную (например, `FixedStrategy` для проверки контракта). Замена хранения не затрагивает сценарии.

## DI-контейнер

Зависимости собираются в одном месте - [`buildFacade()`](src/composition_root.hpp#14). Контейнер [`Container`](src/infrastructure/di_container.hpp#11): `singleton<T>(factory)` регистрирует фабрику, `resolve<T>()` возвращает готовый экземпляр (или бросает `std::logic_error`, если тип не зарегистрирован)

`buildFacade()` регистрирует:

- `IStudentRepository`, `IRubricRepository`, `IGradebookRepository` -> `InMemoryStudents`, `InMemoryRubric`, `InMemoryGradebook`

- `IResultExporter` -> `CsvExporter`;

- `IFinalGradeCalculator` -> `FinalGradeCalculator` с `WeightedSum` по умолчанию

- `PosFacade` -> собирается из `resolve(...)` остальных зависимостей; список стратегий (`WeightedSum`, `WeightedAverageOfGraded`) создаётся здесь же и передаётся в фасад

Контейнер поддерживает только `singleton`: один экземпляр на тип в рамках одной сборки. Контейнер "живёт" внутри `buildFacade()`, а наружу возвращается только `PosFacade`, который удерживает зависимости через `shared_ptr`

## Паттерны GoF

| Группа | Паттерн | Где | Какую проблему решает |
|---|---|---|---|
| Порождающий | **Factory** | `EvaluationFactory` | Единая точка создания оценки: проверяет критерий по рубрикатору и диапазон балла, конструктор `Evaluation` приватный |
| Структурный | **Facade** | `PosFacade` | Консоль знает одну точку входа и не знает про репозитории, стратегии и экспортёры, те же сценарии используют демо и тесты |
| Поведенческий | **Strategy** | `IGradingStrategy` (`WeightedSum`, `WeightedAverageOfGraded`) | Правило итога сменяется во время работы, список правил строится из стратегий, переданных в `PosFacade` (меню, пункт 10). Домен не знает, какое правило выбрано |


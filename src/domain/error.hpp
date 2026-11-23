#pragma once

#include <stdexcept>

namespace pos::domain {

class DomainError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InvalidValue : public DomainError {
public:
    using DomainError::DomainError;
};

class UnknownCriterion : public DomainError {
public:
    using DomainError::DomainError;
};

class UnknownStudent : public DomainError {
public:
    using DomainError::DomainError;
};

class DuplicateStudent : public DomainError {
public:
    using DomainError::DomainError;
};

class DuplicateCriterion : public DomainError {
public:
    using DomainError::DomainError;
};

class DuplicateEvaluation : public DomainError {
public:
    using DomainError::DomainError;
};

class EvaluationNotFound : public DomainError {
public:
    using DomainError::DomainError;
};

}  // namespace pos::domain

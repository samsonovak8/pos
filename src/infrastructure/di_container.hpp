#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace pos::infrastructure {

class Container {
public:
    template <class T, class Factory>
    void singleton(Factory factory) {
        entries_[typeid(T)] = [this, factory, instance = std::shared_ptr<void>()]() mutable -> std::shared_ptr<void> {
            if (!instance) {
                instance = std::shared_ptr<T>(factory(*this));
            }
            return instance;
        };
    }

    template <class T>
    std::shared_ptr<T> resolve() {
        const auto it = entries_.find(typeid(T));
        if (it == entries_.end()) {
            throw std::logic_error("DI: тип не зарегистрирован");
        }
        return std::static_pointer_cast<T>(it->second());
    }

private:
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> entries_;
};

}  // namespace pos::infrastructure

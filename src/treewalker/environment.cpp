#include <print>

#include "treewalker/environment.hpp"
#include "treewalker/value.hpp"

// Environment
void Environment::setVar(const std::string& name, Value* value) {
    if (this->m_vars.contains(name)) {
        this->m_vars[name] = value;
    } else if (this->m_parent) {
        this->m_parent->setVar(name, value);
    } else {
        std::print("Error: Cannot set variable '{}'; not defined\n", name);
    }
}

void Environment::defineVar(const std::string& name, Value* value) {
    this->m_vars[name] = value;
}

Value* Environment::getVar(const std::string& name) const {
    auto it = this->m_vars.find(name);
    if (it != this->m_vars.end()) {
        return it->second;
    } else if (this->m_parent) {
        return this->m_parent->getVar(name);
    } else {
        return nullptr;
    }
}

Environment* Environment::getParent() const {
    return this->m_parent;
}

void Environment::addChildEnvironment(std::unique_ptr<Environment> childEnvironment) {
    this->m_childEnvironments.push_back(std::move(childEnvironment));
}

std::vector<Environment*> Environment::getChildEnvironments() const {
    std::vector<Environment*> result;
    for (const auto& child : this->m_childEnvironments) {
        result.push_back(child.get());
    }
    return result;
}

std::vector<std::unique_ptr<Environment>> Environment::takeChildEnvironments() {
    return std::move(this->m_childEnvironments);
}

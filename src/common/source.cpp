#include "common/source.hpp"
#include <string>

std::string Source::getName() const {
    return this->name;
}

SourceKind Source::getSourceKind() const {
    return this->sourceKind;
}

std::string Source::getSourceString() const {
    return this->sourceString;
}
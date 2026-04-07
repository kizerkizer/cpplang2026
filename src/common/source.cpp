#include "common/source.hpp"
#include <string>

std::string Source::getName() const {
    return this->m_name;
}

SourceKind Source::getSourceKind() const {
    return this->m_sourceKind;
}

std::string Source::getSourceString() const {
    return this->m_sourceString;
}
#pragma once

#include <string>

enum class SourceKind {
    File,
    Stdin,
    String,
};

class Source {
private:
    SourceKind m_sourceKind;
    std::string m_name;
    std::string m_sourceString;
public:
    Source(SourceKind sourceKind, std::string name, std::string sourceString) : m_sourceKind(sourceKind), m_name(name), m_sourceString(sourceString) {}
    std::string getName() const;
    SourceKind getSourceKind() const;
    std::string getSourceString() const;
};
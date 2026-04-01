#pragma once

#include <string>

enum class SourceKind {
    File,
    Stdin,
    String,
};

class Source {
private:
    SourceKind sourceKind;
    std::string name;
    std::string sourceString;
public:
    Source(SourceKind sourceKind, std::string name, std::string sourceString) : sourceKind(sourceKind), name(name), sourceString(sourceString) {}
    std::string getName() const;
    SourceKind getSourceKind() const;
    std::string getSourceString() const;
};
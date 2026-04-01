#pragma once

#include <optional>

#include "checker/typestore.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"

class TypeChecker {
private:
    Source* source;
    Diagnostics& diagnostics;
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    Type* examine(Node* node);
    TypeStore* typeStore = new TypeStore();
public:
    TypeChecker (Source* source, Diagnostics& diagnostics) : source(source), diagnostics(diagnostics) {};
    void typeCheck(Node* rootNode);
    TypeStore* getTypeStore();
};
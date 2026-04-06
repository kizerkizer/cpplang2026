#pragma once

#include <string>

#include "common/source.hpp"
#include "common/sourcecodelocation.hpp"

enum class DiagnosticMessageKind {
    Error,
    Warning,
    Info,
    Debug,
};

constexpr const char* diagnosticMessageKindToString(DiagnosticMessageKind diagnosticMessageKind) {
    return (diagnosticMessageKind == DiagnosticMessageKind::Error ? "Error" :
    (diagnosticMessageKind == DiagnosticMessageKind::Warning ? "Warning" :
    (diagnosticMessageKind == DiagnosticMessageKind::Info ? "Info" :
    (diagnosticMessageKind == DiagnosticMessageKind::Debug ? "Debug" :
        "Unknown"))));
}

constexpr const char* diagnosticMessageKindToColoredString(DiagnosticMessageKind diagnosticMessageKind) {
    return (diagnosticMessageKind == DiagnosticMessageKind::Error ? "\033[31mError\033[0m" :
    (diagnosticMessageKind == DiagnosticMessageKind::Warning ? "\033[33mWarning\033[0m" :
    (diagnosticMessageKind == DiagnosticMessageKind::Info ? "\033[34mInfo\033[0m" :
    (diagnosticMessageKind == DiagnosticMessageKind::Debug ? "\033[32mDebug\033[0m" :
        "Unknown"))));
}

enum class DiagnosticMessageStage {
    Scanner,
    Lexer,
    Parser,
    Binder,
    Desugarer,
    TypeChecker,
};


constexpr const char* diagnosticMessageStageToString(DiagnosticMessageStage diagnosticMessageStage) {
    return (diagnosticMessageStage == DiagnosticMessageStage::Scanner ? "Scanner" :
    (diagnosticMessageStage == DiagnosticMessageStage::Lexer ? "Lexer" :
    (diagnosticMessageStage == DiagnosticMessageStage::Parser ? "Parser" :
    (diagnosticMessageStage == DiagnosticMessageStage::Binder ? "Binder" :
    (diagnosticMessageStage == DiagnosticMessageStage::Desugarer ? "Desugarer" :
    (diagnosticMessageStage == DiagnosticMessageStage::TypeChecker ? "TypeChecker" :
        "Unknown"))))));
}   

class DiagnosticMessage {
private:
    int code;
    DiagnosticMessageKind kind;
    DiagnosticMessageStage stage;
    SourceCodeLocationSpan sourceCodeLocationSpan;
    Source* source;
    std::string message;
public:
    DiagnosticMessage(int code, DiagnosticMessageKind kind, DiagnosticMessageStage stage, SourceCodeLocationSpan sourceCodeLocationSpan, Source* source, std::string message) : code(code), kind(kind), stage(stage), sourceCodeLocationSpan(sourceCodeLocationSpan), source(source), message(message) {}
    int getCode() const;
    DiagnosticMessageKind getKind() const;
    DiagnosticMessageStage getStage() const;
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
    Source* getSource() const;
    std::string getMessage() const;
    std::string getFullMessage() const;
    std::string getCurtMessage() const;
};

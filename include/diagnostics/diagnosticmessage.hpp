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

#define DIAGNOSTIC_MESSAGE_KIND_TO_STRING(diagnosticMessageKind) \
    (diagnosticMessageKind == DiagnosticMessageKind::Error ? "Error" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Warning ? "Warning" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Info ? "Info" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Debug ? "Debug" : \
        "Unknown"))))

#define DIAGNOSTIC_MESSAGE_KIND_TO_COLORED_STRING(diagnosticMessageKind) \
    (diagnosticMessageKind == DiagnosticMessageKind::Error ? "\033[31mError\033[0m" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Warning ? "\033[33mWarning\033[0m" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Info ? "\033[34mInfo\033[0m" : \
    (diagnosticMessageKind == DiagnosticMessageKind::Debug ? "\033[32mDebug\033[0m" : \
        "Unknown"))))

enum class DiagnosticMessageStage {
    Lexer,
    Parser,
    Binder,
    Desugarer,
    TypeChecker,
};

#define DIAGNOSTIC_MESSAGE_STAGE_TO_STRING(diagnosticMessageStage) \
    (diagnosticMessageStage == DiagnosticMessageStage::Lexer ? "Lexer" : \
    (diagnosticMessageStage == DiagnosticMessageStage::Parser ? "Parser" : \
    (diagnosticMessageStage == DiagnosticMessageStage::Binder ? "Binder" : \
    (diagnosticMessageStage == DiagnosticMessageStage::Desugarer ? "Desugarer" : \
    (diagnosticMessageStage == DiagnosticMessageStage::TypeChecker ? "TypeChecker" : \
        "Unknown")))))

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

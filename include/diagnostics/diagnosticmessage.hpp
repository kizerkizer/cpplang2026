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
    switch (diagnosticMessageKind) {
        case DiagnosticMessageKind::Error:
            return "Error";
        case DiagnosticMessageKind::Warning:
            return "Warning";
        case DiagnosticMessageKind::Info:
            return "Info";
        case DiagnosticMessageKind::Debug:
            return "Debug";
    }
}

constexpr const char* diagnosticMessageKindToColoredString(DiagnosticMessageKind diagnosticMessageKind) {
    switch (diagnosticMessageKind) {
        case DiagnosticMessageKind::Error:
            return "\033[31mError\033[0m";
        case DiagnosticMessageKind::Warning:
            return "\033[33mWarning\033[0m";
        case DiagnosticMessageKind::Info:
            return "\033[34mInfo\033[0m";
        case DiagnosticMessageKind::Debug:
            return "\033[32mDebug\033[0m";
    }
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
    switch (diagnosticMessageStage) {
        case DiagnosticMessageStage::Scanner:
            return "Scanner";
        case DiagnosticMessageStage::Lexer:
            return "Lexer";
        case DiagnosticMessageStage::Parser:
            return "Parser";
        case DiagnosticMessageStage::Binder:
            return "Binder";
        case DiagnosticMessageStage::Desugarer:
            return "Desugarer";
        case DiagnosticMessageStage::TypeChecker:
            return "TypeChecker";
    }
}   

class DiagnosticMessage {
private:
    int m_code;
    DiagnosticMessageKind m_kind;
    DiagnosticMessageStage m_stage;
    SourceCodeLocationSpan m_sourceCodeLocationSpan;
    Source* m_source;
    std::string m_message;
public:
    DiagnosticMessage(int code, DiagnosticMessageKind kind, DiagnosticMessageStage stage, SourceCodeLocationSpan sourceCodeLocationSpan, Source* source, std::string message) : m_code(code), m_kind(kind), m_stage(stage), m_sourceCodeLocationSpan(sourceCodeLocationSpan), m_source(source), m_message(message) {}
    int getCode() const;
    DiagnosticMessageKind getKind() const;
    DiagnosticMessageStage getStage() const;
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
    Source* getSource() const;
    std::string getMessage() const;
    std::string getFullMessage() const;
    std::string getCurtMessage() const;
};

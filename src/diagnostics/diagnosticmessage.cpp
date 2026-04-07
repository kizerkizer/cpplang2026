#include <format>

#include "diagnostics/diagnosticmessage.hpp"
#include "common/source.hpp"
#include "common/sourcecodelocation.hpp"

// DiagnosticMessage
int DiagnosticMessage::getCode() const {
    return this->m_code;
}

DiagnosticMessageKind DiagnosticMessage::getKind() const {
    return this->m_kind;
}

DiagnosticMessageStage DiagnosticMessage::getStage() const {
    return this->m_stage;
}

SourceCodeLocationSpan DiagnosticMessage::getSourceCodeLocationSpan() const {
    return this->m_sourceCodeLocationSpan;
}

Source* DiagnosticMessage::getSource() const {
    return this->m_source;
}

std::string DiagnosticMessage::getMessage() const {
    return this->m_message;
}

std::string DiagnosticMessage::getFullMessage() const {
    std::string fullMessage = std::string("[") 
        + diagnosticMessageKindToColoredString(this->m_kind) + std::string("] (")
        + diagnosticMessageStageToString(this->m_stage) 
        + std::string(") E") + std::format("{:06d}", this->m_code) + std::string("\n");
    auto [startByteOffset, startCodepointOffset, startLine, startColumn] = this->m_sourceCodeLocationSpan.start;
    auto [endByteOffset, endCodepointOffset, endLine, endColumn] = this->m_sourceCodeLocationSpan.end;
    fullMessage += this->m_source->getName() + ":" + std::to_string(startLine) + ":" + std::to_string(startColumn) + " - " + std::to_string(endLine) + ":" + std::to_string(endColumn) + "\n";
    fullMessage += this->m_message;
    return fullMessage;
}

std::string DiagnosticMessage::getCurtMessage() const {
    return std::string(diagnosticMessageKindToString(this->m_kind)) + std::string(",") + std::string(diagnosticMessageStageToString(this->m_stage)) + std::string(",") + std::to_string(this->m_code) + std::string(",") + this->m_message;
}
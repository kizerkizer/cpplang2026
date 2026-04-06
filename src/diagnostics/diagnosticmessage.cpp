#include <format>

#include "diagnostics/diagnosticmessage.hpp"
#include "common/source.hpp"
#include "common/sourcecodelocation.hpp"


int DiagnosticMessage::getCode() const {
    return this->code;
}

DiagnosticMessageKind DiagnosticMessage::getKind() const {
    return this->kind;
}

DiagnosticMessageStage DiagnosticMessage::getStage() const {
    return this->stage;
}

SourceCodeLocationSpan DiagnosticMessage::getSourceCodeLocationSpan() const {
    return this->sourceCodeLocationSpan;
}

Source* DiagnosticMessage::getSource() const {
    return this->source;
}

std::string DiagnosticMessage::getMessage() const {
    return this->message;
}

std::string DiagnosticMessage::getFullMessage() const {
    std::string fullMessage = std::string("[") 
        + DIAGNOSTIC_MESSAGE_KIND_TO_COLORED_STRING(this->kind) + std::string("] (")
        + DIAGNOSTIC_MESSAGE_STAGE_TO_STRING(this->stage) 
        + std::string(") E") + std::format("{:06d}", this->code) + std::string("\n");
    auto [startByteOffset, startCodepointOffset, startLine, startColumn] = this->sourceCodeLocationSpan.start;
    auto [endByteOffset, endCodepointOffset, endLine, endColumn] = this->sourceCodeLocationSpan.end;
    fullMessage += this->source->getName() + ":" + std::to_string(startLine) + ":" + std::to_string(startColumn) + " - " + std::to_string(endLine) + ":" + std::to_string(endColumn) + "\n";
    fullMessage += this->message;
    return fullMessage;
}

std::string DiagnosticMessage::getCurtMessage() const {
    return std::string(DIAGNOSTIC_MESSAGE_KIND_TO_STRING(this->kind)) + std::string(",") + std::string(DIAGNOSTIC_MESSAGE_STAGE_TO_STRING(this->stage)) + std::string(",") + std::to_string(this->code) + std::string(",") + this->message;
}
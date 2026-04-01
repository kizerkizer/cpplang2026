#include "diagnostics/diagnostics.hpp"

void Diagnostics::addDiagnosticMessage(DiagnosticMessage diagnosticMessage) {
    this->diagnosticMessages.push_back(diagnosticMessage);
}

std::vector<DiagnosticMessage> Diagnostics::getDiagnosticMessages() const {
    return this->diagnosticMessages;
}
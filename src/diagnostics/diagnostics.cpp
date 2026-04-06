#include "diagnostics/diagnostics.hpp"

// Diagnostics
void Diagnostics::addDiagnosticMessage(DiagnosticMessage diagnosticMessage) {
    this->diagnosticMessages.push_back(diagnosticMessage);
}

std::vector<DiagnosticMessage> Diagnostics::getDiagnosticMessages() const {
    return this->diagnosticMessages;
}
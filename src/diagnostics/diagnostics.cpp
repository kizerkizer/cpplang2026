#include "diagnostics/diagnostics.hpp"

// Diagnostics
void Diagnostics::addDiagnosticMessage(DiagnosticMessage diagnosticMessage) {
    this->m_diagnosticMessages.push_back(diagnosticMessage);
}

std::vector<DiagnosticMessage> Diagnostics::getDiagnosticMessages() const {
    return this->m_diagnosticMessages;
}
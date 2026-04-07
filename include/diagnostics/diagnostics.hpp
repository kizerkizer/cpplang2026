#pragma once

#include <vector>

#include "diagnostics/diagnosticmessage.hpp"

class Diagnostics {
private:
    std::vector<DiagnosticMessage> m_diagnosticMessages;
public:
    Diagnostics() {}
    void addDiagnosticMessage(DiagnosticMessage diagnosticMessage);
    std::vector<DiagnosticMessage> getDiagnosticMessages() const;
};
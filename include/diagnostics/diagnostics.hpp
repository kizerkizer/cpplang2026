#pragma once

#include <vector>

#include "diagnostics/diagnosticmessage.hpp"

class Diagnostics {
public:
    Diagnostics() {}
    void addDiagnosticMessage(DiagnosticMessage diagnosticMessage);
    std::vector<DiagnosticMessage> getDiagnosticMessages() const;
private:
    std::vector<DiagnosticMessage> diagnosticMessages;
};
#pragma once

#include "flow/flownode.hpp"
#include "diagnostics/diagnostics.hpp"
#include "flow/flowbuilder.hpp"
#include "parser/node.hpp"

class FlowAnalyzer {
private:
    Source* m_source;
    Diagnostics& m_diagnostics;
    void warnUnreachable(FlowNode* node);
public:
    FlowAnalyzer(Source* source, Diagnostics& diagnostics) : m_source(source), m_diagnostics(diagnostics) {}
    std::unique_ptr<FlowAnalyzerResult> analyze(Node* rootNode);
};
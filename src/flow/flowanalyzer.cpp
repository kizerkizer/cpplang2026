#include "flow/flowanalyzer.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "flow/flowbuilder.hpp"

void FlowAnalyzer::warnUnreachable(FlowNode* node) {
    m_diagnostics.addDiagnosticMessage(DiagnosticMessage(80, DiagnosticMessageKind::Warning, DiagnosticMessageStage::ControlFlowAnalyzer, node->getAstNode()->getSourceCodeLocationSpan(), m_source, "Unreachable code"));
}

std::unique_ptr<FlowAnalyzerResult> FlowAnalyzer::analyze(Node* rootNode) {
    FlowBuilder builder(m_diagnostics);
    auto result = builder.buildGraph(rootNode);
    for (auto& graph : result->getGraphs()) {
        graph->computeReachability();
        for (auto& unreachableNode : graph->getUnreachable()) {
            this->warnUnreachable(unreachableNode);
        }
    }
    return result;
}
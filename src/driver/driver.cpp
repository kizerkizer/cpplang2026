#include <print>

#include "driver/driver.hpp"
#include "binder/binder.hpp"
#include "checker/checker.hpp"
#include "desugarer/desugarer.hpp"
#include "flowbuilder/flowbuilder.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "treewalker/interpreter.hpp"
#include "utf8scanner/utf8scanner.hpp"

void Driver::compile(Source* source) {
    auto sourceString = source->getSourceString();

    Utf8Scanner utf8Scanner = Utf8Scanner(source, m_diagnostics);
    Lexer lexer = Lexer(utf8Scanner, source, m_diagnostics);
    // TODO print lexer tokens then reset if argument set
    /*std::unique_ptr<Token> token = nullptr;
    while (!lexer.isDone()) {
        token = lexer.getNextToken();
        std::print("<{}> '{}'\n", tokenKindToString(token->getTokenKind()), token->getSourceString());
    }*/
    Parser parser = Parser(source, &lexer, m_diagnostics);
    auto parsed = parser.parse();
    for (const auto& diagnosticMessage : m_diagnostics.getDiagnosticMessages()) {
        std::print("{}\n", diagnosticMessage.getFullMessage());
    }
    if (!m_diagnostics.getDiagnosticMessages().empty()) {
        // TODO
    } else {
        std::print("Parsed successfully!\n");
    }
    Desugarer desugarer = Desugarer(std::move(parsed), m_diagnostics);
    auto desugared = desugarer.desugar();
    for (const auto& diagnosticMessage : m_diagnostics.getDiagnosticMessages()) {
        std::print("{}\n", diagnosticMessage.getFullMessage());
    }
    if (!m_diagnostics.getDiagnosticMessages().empty()) {
        // TODO
    } else {
        std::print("Desugared successfully!\n");
    }
    Binder binder(source, m_diagnostics);
    auto binderResult = binder.bind(desugared.get());
    for (const auto& diagnosticMessage : m_diagnostics.getDiagnosticMessages()) {
        std::print("{}\n", diagnosticMessage.getFullMessage());
    }
    if (!m_diagnostics.getDiagnosticMessages().empty()) {
        // TODO
    } else {
        std::print("Bound successfully!\n");
    }
    FlowBuilder flowBuilder = FlowBuilder();
    std::unique_ptr<FlowBuilderResult> result = flowBuilder.buildGraph(binderResult->getNode());
    for (auto graph : result->getGraphs()) {
        graph->assignReachabilityToNodes();
    }
    // TODO optionally print results of reachability analysis on flow graph
    TypeChecker typeChecker = TypeChecker(source, m_diagnostics);
    typeChecker.typeCheck(binderResult->getNode());
    for (const auto& diagnosticMessage : m_diagnostics.getDiagnosticMessages()) {
        std::print("{}\n", diagnosticMessage.getFullMessage());
    }
    if (!m_diagnostics.getDiagnosticMessages().empty()) {
        // TODO
    } else {
        std::print("Type checked successfully!\n");
    }
    Interpreter interpreter = Interpreter(m_outputStream);
    // TODO --v
    auto resultingValue = interpreter.interpret(binderResult->getNode());
    if (resultingValue->getKind() == ValueKind::Integer) {
        auto integerValue = static_cast<IntegerValue*>(resultingValue);
        std::print("Program interpretation result: {}\n", integerValue->getValue());
    }
}
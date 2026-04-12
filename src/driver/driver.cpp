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

void displayResults(const std::vector<DiagnosticMessage>& diagnosticMessages, std::string stage) {
    for (const auto& diagnosticMessage : diagnosticMessages) {
        std::print("{}", diagnosticMessage.getFullMessage());
    }
    if (diagnosticMessages.empty()) {
        std::print("{} successful\n", stage);
    }
}

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
    displayResults(m_diagnostics.getDiagnosticMessages(), "parse");

    Desugarer desugarer = Desugarer(std::move(parsed), m_diagnostics);
    auto desugared = desugarer.desugar();
    displayResults(m_diagnostics.getDiagnosticMessages(), "desugar");

    Binder binder(source, m_diagnostics);
    auto binderResult = binder.bind(desugared.get());
    displayResults(m_diagnostics.getDiagnosticMessages(), "bind");

    FlowBuilder flowBuilder = FlowBuilder(m_diagnostics);
    std::unique_ptr<FlowBuilderResult> flowBuilderResult = flowBuilder.buildGraph(binderResult->getNode());
    for (auto graph : flowBuilderResult->getGraphs()) {
        graph->assignReachabilityToNodes();
    }
    // TODO flow builder diagnostics

    // TODO optionally print results of reachability analysis on flow graph
    TypeChecker typeChecker = TypeChecker(source, *binderResult, *flowBuilderResult, m_diagnostics);
    auto typeCheckerResult = typeChecker.typeCheck(binderResult->getNode());
    displayResults(m_diagnostics.getDiagnosticMessages(), "type check");

    Interpreter interpreter = Interpreter(typeCheckerResult.get(), binderResult.get(), flowBuilderResult.get(), m_outputStream);
    auto resultingValue = interpreter.interpret(binderResult->getNode());
    if (resultingValue->getKind() == ValueKind::Integer) {
        auto integerValue = static_cast<IntegerValue*>(resultingValue);
        std::print("Program interpretation result: {}\n", integerValue->getValue());
    }
}
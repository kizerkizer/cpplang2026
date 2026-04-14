#include <print>

#include "driver/driver.hpp"
#include "binder/binder.hpp"
#include "checker/checker.hpp"
#include "desugarer/desugarer.hpp"
#include "flow/flowanalyzer.hpp"
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
    Parser parser = Parser(source, &lexer, m_diagnostics);
    auto parsed = parser.parse();
    displayResults(m_diagnostics.getDiagnosticMessages(), "parse");

    Desugarer desugarer = Desugarer(std::move(parsed), m_diagnostics);
    auto desugared = desugarer.desugar();
    displayResults(m_diagnostics.getDiagnosticMessages(), "desugar");

    Binder binder(source, m_diagnostics);
    auto binderResult = binder.bind(desugared.get());
    displayResults(m_diagnostics.getDiagnosticMessages(), "bind");

    FlowAnalyzer flowAnalyzer = FlowAnalyzer(source, m_diagnostics);
    auto flowAnalyzerResult = flowAnalyzer.analyze(binderResult->getNode());
    displayResults(m_diagnostics.getDiagnosticMessages(), "control flow analysis");

    TypeChecker typeChecker = TypeChecker(source, *binderResult, *flowAnalyzerResult, m_diagnostics);
    auto typeCheckerResult = typeChecker.typeCheck(binderResult->getNode());
    displayResults(m_diagnostics.getDiagnosticMessages(), "type check");

    Interpreter interpreter = Interpreter(typeCheckerResult.get(), binderResult.get(), flowAnalyzerResult.get(), m_outputStream);
    auto resultingValue = interpreter.interpret(binderResult->getNode());
    if (resultingValue->getKind() == ValueKind::Integer) {
        auto integerValue = static_cast<IntegerValue*>(resultingValue);
        std::print("Program interpretation result: {}\n", integerValue->getValue());
    }
}
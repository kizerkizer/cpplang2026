#pragma once

#include <optional>

#include "checker/typestore.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"

class TypeChecker {
private:
    Source* m_source;
    Diagnostics& m_diagnostics;
    TypeStore* m_typeStore = new TypeStore();
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    std::vector<Symbol*> checkFunctionDeclarationParameters(FunctionDeclarationNode* functionDeclarationNode);
    void checkFunctionDeclarationReturns(Type* returnType, FunctionDeclarationNode* functionDeclarationNode);
    Type* getFunctionDeclarationReturnType(FunctionDeclarationNode* functionDeclarationNode);
    FunctionType* getFunctionDeclarationFunctionType(FunctionDeclarationNode* functionDeclarationNode);
    Type* examineIdentifier(Node* node);
    Type* examineTypeExpression(Node* node);
    Type* examineBinaryOperatorExpression(Node* node);
    Type* examineUnaryOperatorExpression(Node* node);
    Type* examineIfExpression(Node* node);
    Type* examineFunctionCallExpression(Node* node);
    Type* examineVariableDeclaration(Node* node);
    Type* examineFunctionDeclaration(Node* node);
    Type* examineIfStatement(Node* node);
    Type* examine(Node* node);
public:
    TypeChecker (Source* source, Diagnostics& diagnostics) : m_source(source), m_diagnostics(diagnostics) {};
    void typeCheck(Node* rootNode);
    TypeStore* getTypeStore();
};
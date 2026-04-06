#pragma once

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"
#include "binder/scope.hpp"

class BinderResult {
private:
    Node* node;
    std::unique_ptr<Scope> rootScope;
public:
    BinderResult(Node* node, std::unique_ptr<Scope> rootScope) : node(node), rootScope(std::move(rootScope)) {};
    Node* getNode() const;
    Scope* getRootScope() const;
    std::unique_ptr<Scope> takeRootScope();
};

class Binder {
private:
    Source* source;
    Diagnostics& diagnostics;
    std::unique_ptr<Scope> rootScope;
    Scope* currentScope = nullptr;
    Scope* createAndEnterScope(ScopeKind kind);
    void exitScope();
    void bindVariableDeclaration(Node* node);
    void bindFunctionDeclaration(Node* node);
    void bindAssignmentExpression(Node* node);
    void bindIfStatement(Node* node);
    void bindRecursive(Node* node, bool doNotCreateScope = false);
    void addErrorMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addWarningMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addInfoMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
public:
    Binder(Source* source, Diagnostics& diagnostics);
    std::unique_ptr<BinderResult> bind(Node* node);
    std::unique_ptr<Scope> takeRootScope();
};
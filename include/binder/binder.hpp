#pragma once

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"
#include "binder/scope.hpp"

class BinderResult {
public:
    BinderResult(Node* node, std::unique_ptr<Scope> rootScope) : node(node), rootScope(std::move(rootScope)) {};
    Node* getNode() const;
    Scope* getRootScope() const;
    std::unique_ptr<Scope> takeRootScope();
private:
    Node* node;
    std::unique_ptr<Scope> rootScope;
};

class Binder {
public:
    Binder(Diagnostics& diagnostics);
    std::unique_ptr<BinderResult> bind(Node* node);
    std::unique_ptr<Scope> takeRootScope();
private:
    Diagnostics& diagnostics;
    std::unique_ptr<Scope> rootScope;
    Scope* currentScope = nullptr;
    Scope* createAndEnterScope(ScopeKind kind);
    void exitScope();
    void bindRecursive(Node* node, bool doNotCreateScope = false);
    void addErrorMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addWarningMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addInfoMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
};
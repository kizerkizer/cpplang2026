#pragma once

#include "parser/node.hpp"
#include "binder/scope.hpp"
#include <vector>

class BinderResult {
public:
    BinderResult(std::unique_ptr<Node> node, Scope* rootScope) : node(std::move(node)), rootScope(rootScope) {};
    Node* getNode() const;
    Scope* getRootScope() const;
private:
    std::unique_ptr<Node> node;
    Scope* rootScope;
};

class Binder {
public:
    Binder(std::vector<std::string>& errorMessages_out);
    std::unique_ptr<BinderResult> bind(std::unique_ptr<Node> node);
private:
    std::vector<std::string>& errorMessages;
    Scope* rootScope = nullptr;
    Scope* currentScope = nullptr;
    Scope* createAndEnterScope(ScopeKind kind);
    void exitScope();
    void addErrorMessage(const std::string& message);
    void bindRecursive(Node* node, bool doNotCreateScope = false);
};
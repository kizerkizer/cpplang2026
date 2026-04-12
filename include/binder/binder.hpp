#pragma once

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"
#include "binder/scope.hpp"
#include "parser/nodemap.hpp"

using ASTNodeInformation = struct ASTNodeInformation {
    Symbol* symbol;
    Symbol* functionSymbol;
    Symbol* loopSymbol;
};

class BinderResult {
private:
    std::unique_ptr<NodeMap<ASTNodeInformation>> m_nodeMap;
    Node* m_node;
    std::unique_ptr<Scope> m_rootScope;
public:
    BinderResult(Node* node, std::unique_ptr<Scope> rootScope, std::unique_ptr<NodeMap<ASTNodeInformation>> nodeMap) : m_nodeMap(std::move(nodeMap)), m_node(node), m_rootScope(std::move(rootScope)) {};
    Node* getNode() const;
    Scope* getRootScope() const;
    std::unique_ptr<Scope> takeRootScope();
    NodeMap<ASTNodeInformation>* getNodeMap() const;
    std::unique_ptr<NodeMap<ASTNodeInformation>> takeNodeMap();
    void setNodeMap(std::unique_ptr<NodeMap<ASTNodeInformation>> nodeMap);
    Symbol* getSymbol(Node* node);
    Symbol* getFunctionSymbol(Node* node);
    Symbol* getLoopSymbol(Node* node);
};

class Binder {
private:
    std::unique_ptr<NodeMap<ASTNodeInformation>> m_nodeMap;
    Source* m_source;
    Diagnostics& m_diagnostics;
    std::unique_ptr<Scope> m_rootScope;
    Scope* m_currentScope = nullptr;
    Scope* createAndEnterScope(ScopeKind kind);
    ASTNodeInformation* info(Node* node);
    void setInfo(Node* node, std::unique_ptr<ASTNodeInformation> info);
    Symbol* symbol(Node *node);
    void setNodeSymbol(Node* node, Symbol* symbol);
    void setFunctionSymbol(Node* node, Symbol* symbol);
    void setLoopSymbol(Node* node, Symbol* symbol);
    void exitScope();
    bool atRootScope() const;
    void bindVariableDeclaration(Node* node);
    void bindTypeDeclaration(Node* node);
    void bindFunctionDeclaration(Node* node);
    void bindAssignmentExpression(Node* node);
    void bindIfStatement(Node* node);
    void bindIdentifier(Node* node);
    void bindRecursive(Node* node, bool doNotCreateScope = false);
    void addErrorMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addWarningMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
    void addInfoMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan = std::nullopt);
public:
    Binder(Source* source, Diagnostics& diagnostics);
    std::unique_ptr<BinderResult> bind(Node* node);
    std::unique_ptr<Scope> takeRootScope();
    std::unique_ptr<NodeMap<ASTNodeInformation>> takeNodeMap();
    NodeMap<ASTNodeInformation>* getNodeMap() const;
    void setNodeMap(std::unique_ptr<NodeMap<ASTNodeInformation>> nodeMap);
};
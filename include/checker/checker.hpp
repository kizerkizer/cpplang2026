#pragma once

#include <optional>

#include "checker/type.hpp"
#include "checker/typestore.hpp"
#include "diagnostics/diagnostics.hpp"
#include "parser/node.hpp"
#include "parser/nodemap.hpp"
#include "binder/binder.hpp"
#include "flowbuilder/flowbuilder.hpp"

using TypeInfo = struct TypeInfo {
    Type* type;
};

class TypeCheckerResult {
private:
    std::unique_ptr<NodeMap<TypeInfo>> m_nodeMap;
    std::unique_ptr<TypeStore> m_typeStore;
public:
    TypeCheckerResult(std::unique_ptr<NodeMap<TypeInfo>> nodeMap, std::unique_ptr<TypeStore> typeStore) : m_nodeMap(std::move(nodeMap)), m_typeStore(std::move(typeStore)) {};
    NodeMap<TypeInfo>* getNodeMap() const;
    std::unique_ptr<NodeMap<TypeInfo>> takeNodeMap();
    void setNodeMap(std::unique_ptr<NodeMap<TypeInfo>> nodeMap);
    TypeStore* getTypeStore() const;
    std::unique_ptr<TypeStore> takeTypeStore();
    void setTypeStore(std::unique_ptr<TypeStore> typeStore);
    Type* getASTNodeType(Node* node);
};

class TypeChecker {
private:
    std::unique_ptr<NodeMap<TypeInfo>> m_nodeMap = std::make_unique<NodeMap<TypeInfo>>();
    Source* m_source;
    BinderResult& m_binderResult;
    FlowBuilderResult& m_flowBuilderResult;
    Diagnostics& m_diagnostics;
    std::unique_ptr<TypeStore> m_typeStore;
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan = std::nullopt);
    std::vector<Symbol*> checkFunctionDeclarationParameters(FunctionDeclarationNode* functionDeclarationNode);
    void checkFunctionDeclarationReturns(Type* returnType, FunctionDeclarationNode* functionDeclarationNode);
    Type* computeFunctionDeclarationReturnType(FunctionDeclarationNode* functionDeclarationNode);
    FunctionType* getFunctionDeclarationFunctionType(FunctionDeclarationNode* functionDeclarationNode);
    Type* examineIdentifier(Node* node);
    Type* examineTypeDeclaration(Node* node);
    TypeType* evaluateTypeExpression(Node* node);
    Type* examineBinaryOperatorExpression(Node* node);
    Type* examineUnaryOperatorExpression(Node* node);
    Type* examineIfExpression(Node* node);
    Type* examineFunctionCallExpression(Node* node);
    Type* examineVariableDeclaration(Node* node);
    Type* examineFunctionDeclaration(Node* node);
    Type* examineIfStatement(Node* node);
    Type* examineFunctionCallStatement(Node* node);
    Type* examineExecutionList(Node* node);
    Type* examineAssignmentExpression(Node* node);
    Type* examineAssignmentStatement(Node* node);
    Type* examineLoopStatement(Node* node);
    Type* examineProgram(Node* node);
    Type* examineBlockStatement(Node* node);

    Type* examine(Node* node);

    FlowGraph* getFlowGraph(Node* node);
    FlowNode* getFlowNode(Node* node);
    Symbol* getSymbol(Node* node);
    Symbol* getFunctionSymbol(Node* node);
    Symbol* getLoopSymbol(Node* node);
    Type* setASTNodeType(Node* node, Type* type);
    Type* getASTNodeType(Node* node);
public:
    TypeChecker (Source* source, BinderResult& binderResult, FlowBuilderResult& flowBuilderResult, Diagnostics& diagnostics) : m_source(source), m_binderResult(binderResult), m_flowBuilderResult(flowBuilderResult), m_diagnostics(diagnostics), m_typeStore(std::make_unique<TypeStore>()) {};
    std::unique_ptr<TypeCheckerResult> typeCheck(Node* rootNode);
    TypeStore* getTypeStore();
    std::unique_ptr<TypeStore> takeTypeStore();
    NodeMap<TypeInfo>* getNodeMap() const;
    std::unique_ptr<NodeMap<TypeInfo>> takeNodeMap();
};
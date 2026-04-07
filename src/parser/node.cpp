#include <memory>
#include <vector>
#include <string_view>

#include "parser/node.hpp"
#include "binder/symbol.hpp"
#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "flowbuilder/flownode.hpp"

// Node
NodeKind Node::getNodeKind() const {
    return this->m_nodeKind;
}

void Node::setReachable(bool reachable) {
    this->m_reachable = reachable;
}

bool Node::isReachable() const {
    return this->m_reachable;
}

bool Node::operator==(const NodeKind& rhs) const {
    return this->m_nodeKind == rhs;
}

bool Node::operator!=(const NodeKind& rhs) const {
    return !(*this == rhs);
}

bool Node::isCompilerCreated() const {
    return this->m_compilerCreated;
}

FlowNode* Node::getFlowNode() {
    return this->m_flowNode;
}

void Node::setFlowNode(FlowNode* flowNode) {
    this->m_flowNode = flowNode;
}

Type* Node::getType() const {
    return this->m_type;
}

void Node::setType(Type* type) {
    this->m_type = type;
}

void Node::addToken(std::unique_ptr<Token> token) {
    this->m_tokens.push_back(std::move(token));
}

std::vector<Token*> Node::getTokens() {
    auto returnTokens = std::vector<Token*>();
    for (auto& token : this->m_tokens) {
        returnTokens.push_back(token.get());
    }
    return returnTokens;
}

SourceCodeLocationSpan Node::getSourceCodeLocationSpan() const {
    return this->m_sourceCodeLocationSpan;
}

// ProgramNode
/*void ProgramNode::addNode(std::unique_ptr<Node> child) {
    this->nodes.push_back(std::move(child));
}*/

FlowGraph* ProgramNode::getFlowGraph() {
    return this->m_flowGraph;
}

void ProgramNode::setFlowGraph(FlowGraph* flowGraph) {
    this->m_flowGraph = flowGraph;
}

const std::vector<Node*> ProgramNode::getChildren() const {
    return { this->m_executionListNode.get() };
}

ExecutionListNode* ProgramNode::getExecutionListNode() const {
    return this->m_executionListNode.get();
}

std::unique_ptr<ExecutionListNode> ProgramNode::takeExecutionListNode() {
    return std::move(this->m_executionListNode);
}

void ProgramNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    this->m_executionListNode = std::move(executionListNode);
}

//VariableDeclarationNode
IdentifierNode* VariableDeclarationNode::getIdentifier() const {
    return this->m_identifierNode.get();
}

std::unique_ptr<IdentifierNode> VariableDeclarationNode::takeIdentifier() {
    return std::move(this->m_identifierNode);
}

void VariableDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifierNode) {
    this->m_identifierNode = std::move(identifierNode);
}

ExpressionNode* VariableDeclarationNode::getExpression() const {
    return this->m_expressionNode.get();
}

std::unique_ptr<ExpressionNode> VariableDeclarationNode::takeExpression() {
    return std::move(this->m_expressionNode);
}

void VariableDeclarationNode::setExpression(std::unique_ptr<ExpressionNode> expressionNode) {
    this->m_expressionNode = std::move(expressionNode);
}

TypeExpressionNode* VariableDeclarationNode::getTypeExpression() const {
    return this->m_typeExpression.get();
}

const std::vector<Node*> VariableDeclarationNode::getChildren() const {
    return {this->m_typeExpression.get(), this->m_expressionNode.get() };
}

//FunctionDeclarationNode
void FunctionDeclarationNode::setFlowGraph(FlowGraph* flowGraph) {
    this->m_flowGraph = flowGraph;
}

FlowGraph* FunctionDeclarationNode::getFlowGraph() {
    return this->m_flowGraph;
}

IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return this->m_identifier.get();
};

TypeExpressionNode* FunctionDeclarationNode::getReturnTypeExpression() const {
    return this->m_returnTypeExpression.get();
}

std::string FunctionDeclarationNode::getIdentifierName() const {
    return std::string(this->getIdentifier()->getName());
}

const std::vector<IdentifierWithPossibleAnnotationNode*> FunctionDeclarationNode::getParameters() const {
    std::vector<IdentifierWithPossibleAnnotationNode*> parameterPointers;
    for (const auto& parameter : this->m_parameters) {
        parameterPointers.push_back(parameter.get());
    }
    return parameterPointers;
}

BlockStatementNode* FunctionDeclarationNode::getBody() const {
    return this->m_bodyNode.get();
};

const std::vector<Node*> FunctionDeclarationNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->m_identifier.get());
    for (const auto& parameter : this->m_parameters) {
        children.push_back(parameter.get());
    }
    children.push_back(this->m_bodyNode.get());
    return children;
}

std::unique_ptr<IdentifierNode> FunctionDeclarationNode::takeIdentifier() {
    return std::move(this->m_identifier);
}

std::unique_ptr<TypeExpressionNode> FunctionDeclarationNode::takeReturnTypeExpression() {
    return std::move(this->m_returnTypeExpression);
}

std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> FunctionDeclarationNode::takeParameters() {
    return std::move(this->m_parameters);
}

std::unique_ptr<BlockStatementNode> FunctionDeclarationNode::takeBodyNode() {
    return std::move(this->m_bodyNode);
}

void FunctionDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->m_identifier = std::move(identifier);
}

void FunctionDeclarationNode::setReturnTypeExpression(std::unique_ptr<TypeExpressionNode> returnTypeExpression) {
    this->m_returnTypeExpression = std::move(returnTypeExpression);
}

void FunctionDeclarationNode::setParameters(std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters) {
    this->m_parameters = std::move(parameters);
}

void FunctionDeclarationNode::setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode) {
    this->m_bodyNode = std::move(bodyNode);
}

//IdentifierWithPossibleAnnotationNode
TypeExpressionNode* IdentifierWithPossibleAnnotationNode::getAnnotation() const {
    return this->m_annotation.get();
}

std::unique_ptr<TypeExpressionNode> IdentifierWithPossibleAnnotationNode::takeAnnotation() {
    return std::move(this->m_annotation);
}

void IdentifierWithPossibleAnnotationNode::setAnnotation(std::unique_ptr<TypeExpressionNode> annotation) {
    this->m_annotation = std::move(annotation);
}

//BlockStatementNode
ExecutionListNode* BlockStatementNode::getExecutionListNode() const {
    return this->m_executionListNode.get();
}

const std::vector<Node*> BlockStatementNode::getChildren() const {
    return { this->m_executionListNode.get() };
}

std::unique_ptr<ExecutionListNode> BlockStatementNode::takeExecutionListNode() {
    return std::move(this->m_executionListNode);
}

void BlockStatementNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    this->m_executionListNode = std::move(executionListNode);
}

//ExecutionListNode
const std::vector<Node*> ExecutionListNode::getChildren() const {
    std::vector<Node*> children;
    for (const auto& node : this->m_nodes) {
        children.push_back(node.get());
    }
    return children;
}

std::vector<std::unique_ptr<Node>> ExecutionListNode::takeChildren() {
    return std::move(this->m_nodes);
}

void ExecutionListNode::setChildren(std::vector<std::unique_ptr<Node>> children) {
    this->m_nodes = std::move(children);
}

void ExecutionListNode::addNode(std::unique_ptr<Node> node) {
    this->m_nodes.push_back(std::move(node));
}

//IfStatementNode
ExpressionNode* IfStatementNode::getCondition() const {
    return this->m_condition.get();
}

Node* IfStatementNode::getThenBranch() const {
    return this->m_thenBranch.get();
}

Node* IfStatementNode::getElseBranch() const {
    return this->m_elseBranch.get();
}

const std::vector<Node*> IfStatementNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->m_condition.get());
    if (this->m_thenBranch) {
        children.push_back(this->m_thenBranch.get());
    }
    if (this->m_elseBranch) {
        children.push_back(this->m_elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfStatementNode::takeCondition() {
    return std::move(this->m_condition);
}

std::unique_ptr<Node> IfStatementNode::takeThenBranch() {
    return std::move(this->m_thenBranch);
}

std::unique_ptr<Node> IfStatementNode::takeElseBranch() {
    return std::move(this->m_elseBranch);
}

void IfStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->m_condition = std::move(condition);
}

void IfStatementNode::setThenBranch(std::unique_ptr<Node> thenBranch) {
    this->m_thenBranch = std::move(thenBranch);
}

void IfStatementNode::setElseBranch(std::unique_ptr<Node> elseBranch) {
    this->m_elseBranch = std::move(elseBranch);
}

//WhileStatementNode
ExpressionNode* WhileStatementNode::getCondition() const {
    return this->m_condition.get();
}

BlockStatementNode* WhileStatementNode::getBody() const {
    return this->m_body.get();
}

const std::vector<Node*> WhileStatementNode::getChildren() const {
    return { this->m_condition.get(), this->m_body.get() };
}

std::unique_ptr<ExpressionNode> WhileStatementNode::takeCondition() {
    return std::move(this->m_condition);
}

std::unique_ptr<BlockStatementNode> WhileStatementNode::takeBody() {
    return std::move(this->m_body);
}

void WhileStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->m_condition = std::move(condition);
}

void WhileStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    this->m_body = std::move(body);
}

//LoopStatementNode
BlockStatementNode* LoopStatementNode::getBody() const {
    return this->m_body.get();
}

const std::vector<Node*> LoopStatementNode::getChildren() const {
    return { this->m_body.get() };
}

std::unique_ptr<BlockStatementNode> LoopStatementNode::takeBody() {
    return std::move(this->m_body);
}

void LoopStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    this->m_body = std::move(body);
}

//BreakStatementNode
Symbol* BreakStatementNode::getLoopSymbolReference() const {
    return this->m_loopSymbol;
}

void BreakStatementNode::setLoopSymbolReference(Symbol* name) {
    this->m_loopSymbol = name;
}

//ContinueStatementNode
Symbol* ContinueStatementNode::getLoopSymbolReference() const {
    return this->m_loopSymbol;
}

void ContinueStatementNode::setLoopSymbolReference(Symbol* name) {
    this->m_loopSymbol = name;
}

//ReturnStatementNode
ExpressionNode* ReturnStatementNode::getExpression() const {
    return this->m_expression.get();
}

const std::vector<Node*> ReturnStatementNode::getChildren() const {
    if (this->m_expression) {
        return { this->m_expression.get() };
    }
    return {};
}

std::unique_ptr<ExpressionNode> ReturnStatementNode::takeExpression() {
    return std::move(this->m_expression);
}

void ReturnStatementNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    this->m_expression = std::move(expression);
}

Symbol* ReturnStatementNode::getFunctionNameReference() {
    return this->m_functionName;
}

void ReturnStatementNode::setFunctionNameReference(Symbol* name) {
    this->m_functionName = name;
}

//AssignmentExpressionNode
IdentifierNode* AssignmentExpressionNode::getIdentifier() const {
    return this->m_identifier.get();
}

ExpressionNode* AssignmentExpressionNode::getExpression() const {
    return this->m_expression.get();
}

const std::vector<Node*> AssignmentExpressionNode::getChildren() const {
    return { this->m_identifier.get(), this->m_expression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentExpressionNode::takeIdentifier() {
    return std::move(this->m_identifier);
}

std::unique_ptr<ExpressionNode> AssignmentExpressionNode::takeExpression() {
    return std::move(this->m_expression);
}

void AssignmentExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->m_identifier = std::move(identifier);
}

void AssignmentExpressionNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    this->m_expression = std::move(expression);
}

//AssignmentStatementNode
IdentifierNode* AssignmentStatementNode::getIdentifier() const {
    return this->m_assignmentExpression->getIdentifier();
}

AssignmentExpressionNode* AssignmentStatementNode::getAssignmentExpression() const {
    return this->m_assignmentExpression.get();
}

const std::vector<Node*> AssignmentStatementNode::getChildren() const {
    return { this->m_assignmentExpression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentStatementNode::takeIdentifier() {
    return this->m_assignmentExpression->takeIdentifier();
}

std::unique_ptr<AssignmentExpressionNode> AssignmentStatementNode::takeAssignmentExpression() {
    return std::move(this->m_assignmentExpression);
}

void AssignmentStatementNode::setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) {
    this->m_assignmentExpression = std::move(assignmentExpression);
}

void AssignmentStatementNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->m_assignmentExpression->setIdentifier(std::move(identifier));
}

//IdentifierNode
std::string_view IdentifierNode::getName() const {
    return this->m_identifierToken->getSourceString();
}

const std::vector<Node*> IdentifierNode::getChildren() const {
    return {};
}

Token* IdentifierNode::getIdentifierToken() const {
    return this->m_identifierToken.get();
}

Symbol* IdentifierNode::getSymbolReference() const {
    return this->m_symbol;
}

void IdentifierNode::setSymbolReference(Symbol* symbol) {
    this->m_symbol = symbol;
}

//FunctionCallExpressionNode
IdentifierNode* FunctionCallExpressionNode::getIdentifier() const {
    return this->m_identifier.get();
}

const std::vector<ExpressionNode*> FunctionCallExpressionNode::getArgumentNodes() const {
    std::vector<ExpressionNode*> argumentPointers;
    for (const auto& argument : this->m_argumentNodes) {
        argumentPointers.push_back(argument.get());
    }
    return argumentPointers;
}

const std::vector<Node*> FunctionCallExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->m_identifier.get());
    for (const auto& argument : this->m_argumentNodes) {
        children.push_back(argument.get());
    }
    return children;
}

std::unique_ptr<IdentifierNode> FunctionCallExpressionNode::takeIdentifier() {
    return std::move(this->m_identifier);
}

std::vector<std::unique_ptr<ExpressionNode>> FunctionCallExpressionNode::takeArgumentNodes() {
    return std::move(this->m_argumentNodes);
}

void FunctionCallExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->m_identifier = std::move(identifier);
}

void FunctionCallExpressionNode::setArgumentNodes(std::vector<std::unique_ptr<ExpressionNode>> argumentNodes) {
    this->m_argumentNodes = std::move(argumentNodes);
}

//FunctionCallStatementNode
FunctionCallExpressionNode* FunctionCallStatementNode::getFunctionCallExpression() const {
    return this->m_functionCallExpression.get();
}

const std::vector<Node*> FunctionCallStatementNode::getChildren() const {
    return { this->m_functionCallExpression.get() };
}

std::unique_ptr<FunctionCallExpressionNode> FunctionCallStatementNode::takeFunctionCallExpression() {
    return std::move(this->m_functionCallExpression);
}

void FunctionCallStatementNode::setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression) {
    this->m_functionCallExpression = std::move(functionCallExpression);
}

//NumberLiteralNode
int NumberLiteralNode::getValue() const {
    return std::stoi(std::string(this->m_numberLiteralToken->getSourceString()));
}

Token* NumberLiteralNode::getNumberLiteralToken() const {
    return this->m_numberLiteralToken.get();
}

//BooleanLiteralNode
bool BooleanLiteralNode::getValue() const {
    return std::string(this->m_booleanLiteralToken->getSourceString()) == "true";
}

Token* BooleanLiteralNode::getBooleanLiteralToken() const {
    return this->m_booleanLiteralToken.get();
}

//EmptyLiteralNode
Token* EmptyLiteralNode::getEmptyLiteralToken() const {
    return this->m_emptyLiteralToken.get();
}

//StringLiteralNode
std::string StringLiteralNode::getValue() const {
    return std::string(this->m_stringLiteralToken->getSourceString());
}

//BinaryOperatorExpressionNode
ExpressionNode* BinaryOperatorExpressionNode::getLeft() const {
    return this->m_left.get();
}

ExpressionNode* BinaryOperatorExpressionNode::getRight() const {
    return this->m_right.get();
}

Token* BinaryOperatorExpressionNode::getOperatorToken() const {
    return this->m_operatorToken.get();
}

const std::vector<Node*> BinaryOperatorExpressionNode::getChildren() const {
    return { this->m_left.get(), this->m_right.get() };
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeLeft() {
    return std::move(this->m_left);
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeRight() {
    return std::move(this->m_right);
}

void BinaryOperatorExpressionNode::setLeft(std::unique_ptr<ExpressionNode> left) {
    this->m_left = std::move(left);
}

void BinaryOperatorExpressionNode::setRight(std::unique_ptr<ExpressionNode> right) {
    this->m_right = std::move(right);
}

//UnaryOperatorExpressionNode
ExpressionNode* UnaryOperatorExpressionNode::getOperand() const {
    return this->m_operand.get();
}

Token* UnaryOperatorExpressionNode::getOperatorToken() const {
    return this->m_operatorToken.get();
}

const std::vector<Node*> UnaryOperatorExpressionNode::getChildren() const {
    return { this->m_operand.get() };
}

std::unique_ptr<ExpressionNode> UnaryOperatorExpressionNode::takeOperand() {
    return std::move(this->m_operand);
}

void UnaryOperatorExpressionNode::setOperand(std::unique_ptr<ExpressionNode> operand) {
    this->m_operand = std::move(operand);
}

//IfExpressionNode
ExpressionNode* IfExpressionNode::getCondition() const {
    return this->m_condition.get();
}

ExpressionNode* IfExpressionNode::getThenBranch() const {
    return this->m_thenBranch.get();
}

ExpressionNode* IfExpressionNode::getElseBranch() const {
    return this->m_elseBranch.get();
}

const std::vector<Node*> IfExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->m_condition.get());
    children.push_back(this->m_thenBranch.get());
    if (this->m_elseBranch) {
        children.push_back(this->m_elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeCondition() {
    return std::move(this->m_condition);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeThenBranch() {
    return std::move(this->m_thenBranch);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeElseBranch() {
    return std::move(this->m_elseBranch);
}

void IfExpressionNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->m_condition = std::move(condition);
}

void IfExpressionNode::setThenBranch(std::unique_ptr<ExpressionNode> thenBranch) {
    this->m_thenBranch = std::move(thenBranch);
}

void IfExpressionNode::setElseBranch(std::unique_ptr<ExpressionNode> elseBranch) {
    this->m_elseBranch = std::move(elseBranch);
}

//TypeExpressionNode
PrimitiveTypeKind TypeExpressionNode::getPrimitiveTypeKind() const {
    switch (this->m_token->getTokenKind()) {
        case TokenKind::TypePrimitiveBoolean:
            return PrimitiveTypeKind::Boolean;
        case TokenKind::TypePrimitiveEmpty:
            return PrimitiveTypeKind::Empty;
        case TokenKind::TypePrimitiveFloat:
            return PrimitiveTypeKind::Float;
        /*case TokenKind::TypePrimitiveNumber:
            return PrimitiveTypeKind::Number;*/
        case TokenKind::TypePrimitiveInteger:
            return PrimitiveTypeKind::Integer;
        case TokenKind::TypePrimitiveString:
            return PrimitiveTypeKind::String;
        default:
            return PrimitiveTypeKind::Invalid;
    }
}
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
    return this->nodeKind;
}

void Node::setReachable(bool reachable) {
    this->reachable = reachable;
}

bool Node::isReachable() const {
    return this->reachable;
}

bool Node::operator==(const NodeKind& rhs) const {
    return this->nodeKind == rhs;
}

bool Node::operator!=(const NodeKind& rhs) const {
    return !(*this == rhs);
}

bool Node::isCompilerCreated() const {
    return this->compilerCreated;
}

FlowNode* Node::getFlowNode() {
    return this->flowNode;
}

void Node::setFlowNode(FlowNode* flowNode) {
    this->flowNode = flowNode;
}

Type* Node::getType() const {
    return this->type;
}

void Node::setType(Type* type) {
    this->type = type;
}

void Node::addToken(std::unique_ptr<Token> token) {
    this->tokens.push_back(std::move(token));
}

std::vector<Token*> Node::getTokens() {
    auto returnTokens = std::vector<Token*>();
    for (auto& token : this->tokens) {
        returnTokens.push_back(token.get());
    }
    return returnTokens;
}

SourceCodeLocationSpan Node::getSourceCodeLocationSpan() const {
    return this->sourceCodeLocationSpan;
}

// ProgramNode
/*void ProgramNode::addNode(std::unique_ptr<Node> child) {
    this->nodes.push_back(std::move(child));
}*/

FlowGraph* ProgramNode::getFlowGraph() {
    return this->flowGraph;
}

void ProgramNode::setFlowGraph(FlowGraph* flowGraph) {
    this->flowGraph = flowGraph;
}

const std::vector<Node*> ProgramNode::getChildren() const {
    return { this->executionListNode.get() };
}

ExecutionListNode* ProgramNode::getExecutionListNode() const {
    return this->executionListNode.get();
}

std::unique_ptr<ExecutionListNode> ProgramNode::takeExecutionListNode() {
    return std::move(this->executionListNode);
}

void ProgramNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    this->executionListNode = std::move(executionListNode);
}

//VariableDeclarationNode
IdentifierNode* VariableDeclarationNode::getIdentifier() const {
    return this->identifierNode.get();
}

std::unique_ptr<IdentifierNode> VariableDeclarationNode::takeIdentifier() {
    return std::move(this->identifierNode);
}

void VariableDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifierNode) {
    this->identifierNode = std::move(identifierNode);
}

ExpressionNode* VariableDeclarationNode::getExpression() const {
    return this->expressionNode.get();
}

std::unique_ptr<ExpressionNode> VariableDeclarationNode::takeExpression() {
    return std::move(this->expressionNode);
}

void VariableDeclarationNode::setExpression(std::unique_ptr<ExpressionNode> expressionNode) {
    this->expressionNode = std::move(expressionNode);
}

TypeExpressionNode* VariableDeclarationNode::getTypeExpression() const {
    return this->typeExpression.get();
}

const std::vector<Node*> VariableDeclarationNode::getChildren() const {
    return {this->typeExpression.get(), this->expressionNode.get() };
}

//FunctionDeclarationNode
void FunctionDeclarationNode::setFlowGraph(FlowGraph* flowGraph) {
    this->flowGraph = flowGraph;
}

FlowGraph* FunctionDeclarationNode::getFlowGraph() {
    return this->flowGraph;
}

IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

TypeExpressionNode* FunctionDeclarationNode::getReturnTypeExpression() const {
    return this->returnTypeExpression.get();
}

std::string FunctionDeclarationNode::getIdentifierName() const {
    return std::string(this->getIdentifier()->getName());
}

const std::vector<IdentifierWithPossibleAnnotationNode*> FunctionDeclarationNode::getParameters() const {
    std::vector<IdentifierWithPossibleAnnotationNode*> parameterPointers;
    for (const auto& parameter : this->parameters) {
        parameterPointers.push_back(parameter.get());
    }
    return parameterPointers;
}

BlockStatementNode* FunctionDeclarationNode::getBody() const {
    return this->bodyNode.get();
};

const std::vector<Node*> FunctionDeclarationNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->identifier.get());
    for (const auto& parameter : this->parameters) {
        children.push_back(parameter.get());
    }
    children.push_back(this->bodyNode.get());
    return children;
}

std::unique_ptr<IdentifierNode> FunctionDeclarationNode::takeIdentifier() {
    return std::move(this->identifier);
}

std::unique_ptr<TypeExpressionNode> FunctionDeclarationNode::takeReturnTypeExpression() {
    return std::move(this->returnTypeExpression);
}

std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> FunctionDeclarationNode::takeParameters() {
    return std::move(this->parameters);
}

std::unique_ptr<BlockStatementNode> FunctionDeclarationNode::takeBodyNode() {
    return std::move(this->bodyNode);
}

void FunctionDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->identifier = std::move(identifier);
}

void FunctionDeclarationNode::setReturnTypeExpression(std::unique_ptr<TypeExpressionNode> returnTypeExpression) {
    this->returnTypeExpression = std::move(returnTypeExpression);
}

void FunctionDeclarationNode::setParameters(std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters) {
    this->parameters = std::move(parameters);
}

void FunctionDeclarationNode::setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode) {
    this->bodyNode = std::move(bodyNode);
}

//IdentifierWithPossibleAnnotationNode
TypeExpressionNode* IdentifierWithPossibleAnnotationNode::getAnnotation() const {
    return this->annotation.get();
}

std::unique_ptr<TypeExpressionNode> IdentifierWithPossibleAnnotationNode::takeAnnotation() {
    return std::move(this->annotation);
}

void IdentifierWithPossibleAnnotationNode::setAnnotation(std::unique_ptr<TypeExpressionNode> annotation) {
    this->annotation = std::move(annotation);
}

//BlockStatementNode
ExecutionListNode* BlockStatementNode::getExecutionListNode() const {
    return this->executionListNode.get();
}

const std::vector<Node*> BlockStatementNode::getChildren() const {
    return { this->executionListNode.get() };
}

std::unique_ptr<ExecutionListNode> BlockStatementNode::takeExecutionListNode() {
    return std::move(this->executionListNode);
}

void BlockStatementNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    this->executionListNode = std::move(executionListNode);
}

//ExecutionListNode
const std::vector<Node*> ExecutionListNode::getChildren() const {
    std::vector<Node*> children;
    for (const auto& node : this->nodes) {
        children.push_back(node.get());
    }
    return children;
}

std::vector<std::unique_ptr<Node>> ExecutionListNode::takeChildren() {
    return std::move(this->nodes);
}

void ExecutionListNode::setChildren(std::vector<std::unique_ptr<Node>> children) {
    this->nodes = std::move(children);
}

void ExecutionListNode::addNode(std::unique_ptr<Node> node) {
    this->nodes.push_back(std::move(node));
}

//IfStatementNode
ExpressionNode* IfStatementNode::getCondition() const {
    return this->condition.get();
}

Node* IfStatementNode::getThenBranch() const {
    return this->thenBranch.get();
}

Node* IfStatementNode::getElseBranch() const {
    return this->elseBranch.get();
}

const std::vector<Node*> IfStatementNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->condition.get());
    if (this->thenBranch) {
        children.push_back(this->thenBranch.get());
    }
    if (this->elseBranch) {
        children.push_back(this->elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfStatementNode::takeCondition() {
    return std::move(this->condition);
}

std::unique_ptr<Node> IfStatementNode::takeThenBranch() {
    return std::move(this->thenBranch);
}

std::unique_ptr<Node> IfStatementNode::takeElseBranch() {
    return std::move(this->elseBranch);
}

void IfStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->condition = std::move(condition);
}

void IfStatementNode::setThenBranch(std::unique_ptr<Node> thenBranch) {
    this->thenBranch = std::move(thenBranch);
}

void IfStatementNode::setElseBranch(std::unique_ptr<Node> elseBranch) {
    this->elseBranch = std::move(elseBranch);
}

//WhileStatementNode
ExpressionNode* WhileStatementNode::getCondition() const {
    return this->condition.get();
}

BlockStatementNode* WhileStatementNode::getBody() const {
    return this->body.get();
}

const std::vector<Node*> WhileStatementNode::getChildren() const {
    return { this->condition.get(), this->body.get() };
}

std::unique_ptr<ExpressionNode> WhileStatementNode::takeCondition() {
    return std::move(this->condition);
}

std::unique_ptr<BlockStatementNode> WhileStatementNode::takeBody() {
    return std::move(this->body);
}

void WhileStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->condition = std::move(condition);
}

void WhileStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    this->body = std::move(body);
}

//LoopStatementNode
BlockStatementNode* LoopStatementNode::getBody() const {
    return this->body.get();
}

const std::vector<Node*> LoopStatementNode::getChildren() const {
    return { this->body.get() };
}

std::unique_ptr<BlockStatementNode> LoopStatementNode::takeBody() {
    return std::move(this->body);
}

void LoopStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    this->body = std::move(body);
}

//BreakStatementNode
Symbol* BreakStatementNode::getLoopNameReference() const {
    return this->loopName;
}

void BreakStatementNode::setLoopNameReference(Symbol* name) {
    this->loopName = name;
}

//ContinueStatementNode
Symbol* ContinueStatementNode::getLoopNameReference() const {
    return this->loopName;
}

void ContinueStatementNode::setLoopNameReference(Symbol* name) {
    this->loopName = name;
}

//ReturnStatementNode
ExpressionNode* ReturnStatementNode::getExpression() const {
    return this->expression.get();
}

const std::vector<Node*> ReturnStatementNode::getChildren() const {
    if (this->expression) {
        return { this->expression.get() };
    }
    return {};
}

std::unique_ptr<ExpressionNode> ReturnStatementNode::takeExpression() {
    return std::move(this->expression);
}

void ReturnStatementNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    this->expression = std::move(expression);
}

Symbol* ReturnStatementNode::getFunctionNameReference() {
    return this->functionName;
}

void ReturnStatementNode::setFunctionNameReference(Symbol* name) {
    this->functionName = name;
}

//AssignmentExpressionNode
IdentifierNode* AssignmentExpressionNode::getIdentifier() const {
    return this->identifier.get();
}

ExpressionNode* AssignmentExpressionNode::getExpression() const {
    return this->expression.get();
}

const std::vector<Node*> AssignmentExpressionNode::getChildren() const {
    return { this->identifier.get(), this->expression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentExpressionNode::takeIdentifier() {
    return std::move(this->identifier);
}

std::unique_ptr<ExpressionNode> AssignmentExpressionNode::takeExpression() {
    return std::move(this->expression);
}

void AssignmentExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->identifier = std::move(identifier);
}

void AssignmentExpressionNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    this->expression = std::move(expression);
}

//AssignmentStatementNode
IdentifierNode* AssignmentStatementNode::getIdentifier() const {
    return this->assignmentExpression->getIdentifier();
}

AssignmentExpressionNode* AssignmentStatementNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

const std::vector<Node*> AssignmentStatementNode::getChildren() const {
    return { this->assignmentExpression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentStatementNode::takeIdentifier() {
    return this->assignmentExpression->takeIdentifier();
}

std::unique_ptr<AssignmentExpressionNode> AssignmentStatementNode::takeAssignmentExpression() {
    return std::move(this->assignmentExpression);
}

void AssignmentStatementNode::setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) {
    this->assignmentExpression = std::move(assignmentExpression);
}

void AssignmentStatementNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->assignmentExpression->setIdentifier(std::move(identifier));
}

//IdentifierNode
std::string_view IdentifierNode::getName() const {
    return this->identifierToken->getSourceString();
}

const std::vector<Node*> IdentifierNode::getChildren() const {
    return {};
}

Token* IdentifierNode::getIdentifierToken() const {
    return this->identifierToken.get();
}

Symbol* IdentifierNode::getSymbolReference() const {
    return this->symbol;
}

void IdentifierNode::setSymbolReference(Symbol* symbol) {
    this->symbol = symbol;
}

//FunctionCallExpressionNode
IdentifierNode* FunctionCallExpressionNode::getIdentifier() const {
    return this->identifier.get();
}

const std::vector<ExpressionNode*> FunctionCallExpressionNode::getArgumentNodes() const {
    std::vector<ExpressionNode*> argumentPointers;
    for (const auto& argument : this->argumentNodes) {
        argumentPointers.push_back(argument.get());
    }
    return argumentPointers;
}

const std::vector<Node*> FunctionCallExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->identifier.get());
    for (const auto& argument : this->argumentNodes) {
        children.push_back(argument.get());
    }
    return children;
}

std::unique_ptr<IdentifierNode> FunctionCallExpressionNode::takeIdentifier() {
    return std::move(this->identifier);
}

std::vector<std::unique_ptr<ExpressionNode>> FunctionCallExpressionNode::takeArgumentNodes() {
    return std::move(this->argumentNodes);
}

void FunctionCallExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->identifier = std::move(identifier);
}

void FunctionCallExpressionNode::setArgumentNodes(std::vector<std::unique_ptr<ExpressionNode>> argumentNodes) {
    this->argumentNodes = std::move(argumentNodes);
}

//FunctionCallStatementNode
FunctionCallExpressionNode* FunctionCallStatementNode::getFunctionCallExpression() const {
    return this->functionCallExpression.get();
}

const std::vector<Node*> FunctionCallStatementNode::getChildren() const {
    return { this->functionCallExpression.get() };
}

std::unique_ptr<FunctionCallExpressionNode> FunctionCallStatementNode::takeFunctionCallExpression() {
    return std::move(this->functionCallExpression);
}

void FunctionCallStatementNode::setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression) {
    this->functionCallExpression = std::move(functionCallExpression);
}

//NumberLiteralNode
int NumberLiteralNode::getValue() const {
    return std::stoi(std::string(this->numberLiteralToken->getSourceString()));
}

Token* NumberLiteralNode::getNumberLiteralToken() const {
    return this->numberLiteralToken.get();
}

//BooleanLiteralNode
bool BooleanLiteralNode::getValue() const {
    return std::string(this->booleanLiteralToken->getSourceString()) == "true";
}

Token* BooleanLiteralNode::getBooleanLiteralToken() const {
    return this->booleanLiteralToken.get();
}

//EmptyLiteralNode
Token* EmptyLiteralNode::getEmptyLiteralToken() const {
    return this->emptyLiteralToken.get();
}

//StringLiteralNode
std::string StringLiteralNode::getValue() const {
    return std::string(this->stringLiteralToken->getSourceString());
}

//BinaryOperatorExpressionNode
ExpressionNode* BinaryOperatorExpressionNode::getLeft() const {
    return this->left.get();
}

ExpressionNode* BinaryOperatorExpressionNode::getRight() const {
    return this->right.get();
}

Token* BinaryOperatorExpressionNode::getOperatorToken() const {
    return this->operatorToken.get();
}

const std::vector<Node*> BinaryOperatorExpressionNode::getChildren() const {
    return { this->left.get(), this->right.get() };
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeLeft() {
    return std::move(this->left);
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeRight() {
    return std::move(this->right);
}

void BinaryOperatorExpressionNode::setLeft(std::unique_ptr<ExpressionNode> left) {
    this->left = std::move(left);
}

void BinaryOperatorExpressionNode::setRight(std::unique_ptr<ExpressionNode> right) {
    this->right = std::move(right);
}

//UnaryOperatorExpressionNode
ExpressionNode* UnaryOperatorExpressionNode::getOperand() const {
    return this->operand.get();
}

Token* UnaryOperatorExpressionNode::getOperatorToken() const {
    return this->operatorToken.get();
}

const std::vector<Node*> UnaryOperatorExpressionNode::getChildren() const {
    return { this->operand.get() };
}

std::unique_ptr<ExpressionNode> UnaryOperatorExpressionNode::takeOperand() {
    return std::move(this->operand);
}

void UnaryOperatorExpressionNode::setOperand(std::unique_ptr<ExpressionNode> operand) {
    this->operand = std::move(operand);
}

//IfExpressionNode
ExpressionNode* IfExpressionNode::getCondition() const {
    return this->condition.get();
}

ExpressionNode* IfExpressionNode::getThenBranch() const {
    return this->thenBranch.get();
}

ExpressionNode* IfExpressionNode::getElseBranch() const {
    return this->elseBranch.get();
}

const std::vector<Node*> IfExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(this->condition.get());
    children.push_back(this->thenBranch.get());
    if (this->elseBranch) {
        children.push_back(this->elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeCondition() {
    return std::move(this->condition);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeThenBranch() {
    return std::move(this->thenBranch);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeElseBranch() {
    return std::move(this->elseBranch);
}

void IfExpressionNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    this->condition = std::move(condition);
}

void IfExpressionNode::setThenBranch(std::unique_ptr<ExpressionNode> thenBranch) {
    this->thenBranch = std::move(thenBranch);
}

void IfExpressionNode::setElseBranch(std::unique_ptr<ExpressionNode> elseBranch) {
    this->elseBranch = std::move(elseBranch);
}

//TypeExpressionNode
PrimitiveTypeKind TypeExpressionNode::getPrimitiveTypeKind() const {
    switch (this->token->getTokenKind()) {
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
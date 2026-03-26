#include <memory>
#include <vector>

#include "parser/node.hpp"
#include "binder/name.hpp"
#include "common/sourcecodelocation.hpp"

std::string nodeKindToString(NodeKind nodeKind) {
    switch (nodeKind) {
        case NodeKind::AssignmentExpression:
            return "AssignmentExpression";
        case NodeKind::FunctionCallExpression:
            return "FunctionCallExpression";
        case NodeKind::BinaryOperatorExpression:
            return "BinaryOperatorExpression";
        case NodeKind::UnaryOperatorExpression:
            return "UnaryOperatorExpression";
        case NodeKind::Program:
            return "Program";
        case NodeKind::Invalid:
            return "Invalid";
        case NodeKind::FunctionDeclaration:
            return "FunctionDeclaration";
        case NodeKind::VariableDeclaration:
            return "VariableDeclaration";
        case NodeKind::BlockStatement:
            return "BlockStatement";
        case NodeKind::IfStatement:
            return "IfStatement";
        case NodeKind::WhileStatement:
            return "WhileStatement";
        case NodeKind::LoopStatement:
            return "LoopStatement";
        case NodeKind::BreakStatement:
            return "BreakStatement";
        case NodeKind::ContinueStatement:
            return "ContinueStatement";
        case NodeKind::ReturnStatement:
            return "ReturnStatement";
        case NodeKind::AssignmentStatement:
            return "AssignmentStatement";
        case NodeKind::FunctionCallStatement:
            return "FunctionCallStatement";
        case NodeKind::Identifier:
            return "Identifier";
        case NodeKind::NumberLiteral:
            return "NumberLiteral";
        case NodeKind::StringLiteral:
            return "StringLiteral";
        case NodeKind::BooleanLiteral:
            return "BooleanLiteral";
        case NodeKind::EmptyLiteral:
            return "EmptyLiteral";
        case NodeKind::IfExpression:
            return "IfExpression";
    }
}

// Node
NodeKind Node::getNodeKind() const {
    return this->nodeKind;
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

SourceCodeLocation Node::getFirstSourceCodeLocation() {
    if (!this->tokens.size()) {
        return {-1, -1, -1};
    }
    return this->tokens.front()->getFirstSourceCodeLocation();
}

SourceCodeLocation Node::getLastSourceCodeLocation() {
    if (!this->tokens.size()) {
        return {-1, -1, -1};
    }
    return this->tokens.back()->getLastSourceCodeLocation();
}

// ProgramNode
void ProgramNode::addNode(std::unique_ptr<Node> child) {
    this->nodes.push_back(std::move(child));
}

const std::vector<Node*> ProgramNode::getChildren() const {
    std::vector<Node*> children;
    for (const auto& child : this->nodes) {
        children.push_back(child.get());
    }
    return children;
}

std::vector<std::unique_ptr<Node>> ProgramNode::takeChildren() {
    return std::move(this->nodes);
}

void ProgramNode::setChildren(std::vector<std::unique_ptr<Node>> children) {
    this->nodes = std::move(children);
}

//VariableDeclarationNode
VariableDeclarationNode::VariableDeclarationNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) : Node(NodeKind::VariableDeclaration), assignmentExpression(std::move(assignmentExpression)) { };

AssignmentExpressionNode* VariableDeclarationNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

const std::vector<Node*> VariableDeclarationNode::getChildren() const {
    return {this->assignmentExpression.get() };
}

std::unique_ptr<AssignmentExpressionNode> VariableDeclarationNode::takeAssignmentExpression() {
    return std::move(this->assignmentExpression);
}

void VariableDeclarationNode::setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) {
    this->assignmentExpression = std::move(assignmentExpression);
}

//FunctionDeclarationNode
FunctionDeclarationNode::FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode) : Node(NodeKind::FunctionDeclaration), identifier(std::move(identifier)), parameters(std::move(parameters)), bodyNode(std::move(bodyNode)) {};

IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

std::string FunctionDeclarationNode::getIdentifierName() const {
    return this->getIdentifier()->getName();
}

const std::vector<IdentifierNode*> FunctionDeclarationNode::getParameters() const {
    std::vector<IdentifierNode*> parameterPointers;
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

std::vector<std::unique_ptr<IdentifierNode>> FunctionDeclarationNode::takeParameters() {
    return std::move(this->parameters);
}

std::unique_ptr<BlockStatementNode> FunctionDeclarationNode::takeBodyNode() {
    return std::move(this->bodyNode);
}

void FunctionDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    this->identifier = std::move(identifier);
}

void FunctionDeclarationNode::setParameters(std::vector<std::unique_ptr<IdentifierNode>> parameters) {
    this->parameters = std::move(parameters);
}

void FunctionDeclarationNode::setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode) {
    this->bodyNode = std::move(bodyNode);
}

//BlockStatementNode
BlockStatementNode::BlockStatementNode(std::unique_ptr<ProgramNode> programNode) : Node(NodeKind::BlockStatement), programNode(std::move(programNode)) {};

ProgramNode* BlockStatementNode::getProgramNode() const {
    return this->programNode.get();
}

const std::vector<Node*> BlockStatementNode::getChildren() const {
    return { this->programNode.get() };
}

std::unique_ptr<ProgramNode> BlockStatementNode::takeProgramNode() {
    return std::move(this->programNode);
}

void BlockStatementNode::setProgramNode(std::unique_ptr<ProgramNode> programNode) {
    this->programNode = std::move(programNode);
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

//AssignmentExpressionNode
AssignmentExpressionNode::AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression) : ExpressionNode(NodeKind::AssignmentExpression), identifier(std::move(identifier)), expression(std::move(expression)) {};

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
std::string IdentifierNode::getName() const {
    return this->identifierToken->getSourceString();
}

const std::vector<Node*> IdentifierNode::getChildren() const {
    return {};
}

Token* IdentifierNode::getIdentifierToken() const {
    return this->identifierToken.get();
}

Name* IdentifierNode::getNameReference() const {
    return this->name;
}

void IdentifierNode::setNameReference(Name* name) {
    this->name = name;
}

//FunctionCallExpressionNode
IdentifierNode* FunctionCallExpressionNode::getIdentifier() const {
    return this->identifier.get();
}

const std::vector<const ExpressionNode*> FunctionCallExpressionNode::getArgumentNodes() const {
    std::vector<const ExpressionNode*> argumentPointers;
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
    return std::stoi(this->numberLiteralToken->getSourceString());
}

//BooleanLiteralNode
Token* BooleanLiteralNode::getBooleanLiteralToken() const {
    return this->booleanLiteralToken.get();
}

bool BooleanLiteralNode::getValue() const {
    return this->booleanLiteralToken->getSourceString() == "true";
}

//EmptyLiteralNode
Token* EmptyLiteralNode::getEmptyLiteralToken() const {
    return this->emptyLiteralToken.get();
}

//StringLiteralNode
std::string StringLiteralNode::getValue() const {
    return this->stringLiteralToken->getSourceString();
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
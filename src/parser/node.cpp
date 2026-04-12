#include <memory>
#include <vector>
#include <string_view>

#include "parser/node.hpp"
#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "flowbuilder/flownode.hpp"
#include "lexer/token.hpp"

// Node
NodeKind Node::getNodeKind() const {
    return m_nodeKind;
}

bool Node::operator==(const NodeKind& rhs) const {
    return m_nodeKind == rhs;
}

bool Node::operator!=(const NodeKind& rhs) const {
    return !(*this == rhs);
}

bool Node::isCompilerCreated() const {
    return m_compilerCreated;
}

void Node::addToken(std::unique_ptr<Token> token) {
    m_tokens.push_back(std::move(token));
}

std::vector<Token*> Node::getTokens() {
    auto returnTokens = std::vector<Token*>();
    for (auto& token : m_tokens) {
        returnTokens.push_back(token.get());
    }
    return returnTokens;
}

SourceCodeLocationSpan Node::getSourceCodeLocationSpan() const {
    return m_sourceCodeLocationSpan;
}

int Node::getId() const {
    return m_id;
}

// ProgramNode
const std::vector<Node*> ProgramNode::getChildren() const {
    return { m_executionListNode.get() };
}

ExecutionListNode* ProgramNode::getExecutionListNode() const {
    return m_executionListNode.get();
}

std::unique_ptr<ExecutionListNode> ProgramNode::takeExecutionListNode() {
    return std::move(m_executionListNode);
}

void ProgramNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    m_executionListNode = std::move(executionListNode);
}

//VariableDeclarationNode
IdentifierNode* VariableDeclarationNode::getIdentifier() const {
    return m_identifierNode.get();
}

std::unique_ptr<IdentifierNode> VariableDeclarationNode::takeIdentifier() {
    return std::move(m_identifierNode);
}

void VariableDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifierNode) {
    m_identifierNode = std::move(identifierNode);
}

ExpressionNode* VariableDeclarationNode::getExpression() const {
    return m_expressionNode.get();
}

std::unique_ptr<ExpressionNode> VariableDeclarationNode::takeExpression() {
    return std::move(m_expressionNode);
}

void VariableDeclarationNode::setExpression(std::unique_ptr<ExpressionNode> expressionNode) {
    m_expressionNode = std::move(expressionNode);
}

Node* VariableDeclarationNode::getTypeExpression() const {
    return m_typeExpression.get();
}

const std::vector<Node*> VariableDeclarationNode::getChildren() const {
    return {m_typeExpression.get(), m_expressionNode.get() };
}

//FunctionDeclarationNode
IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return m_identifier.get();
};

Node* FunctionDeclarationNode::getReturnTypeExpression() const {
    return m_returnTypeExpression.get();
}

std::string FunctionDeclarationNode::getIdentifierName() const {
    return std::string(this->getIdentifier()->getName());
}

const std::vector<IdentifierWithPossibleAnnotationNode*> FunctionDeclarationNode::getParameters() const {
    std::vector<IdentifierWithPossibleAnnotationNode*> parameterPointers;
    for (const auto& parameter : m_parameters) {
        parameterPointers.push_back(parameter.get());
    }
    return parameterPointers;
}

BlockStatementNode* FunctionDeclarationNode::getBody() const {
    return m_bodyNode.get();
};

const std::vector<Node*> FunctionDeclarationNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(m_identifier.get());
    for (const auto& parameter : m_parameters) {
        children.push_back(parameter.get());
    }
    children.push_back(m_bodyNode.get());
    return children;
}

std::unique_ptr<IdentifierNode> FunctionDeclarationNode::takeIdentifier() {
    return std::move(m_identifier);
}

std::unique_ptr<Node> FunctionDeclarationNode::takeReturnTypeExpression() {
    return std::move(m_returnTypeExpression);
}

std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> FunctionDeclarationNode::takeParameters() {
    return std::move(m_parameters);
}

std::unique_ptr<BlockStatementNode> FunctionDeclarationNode::takeBodyNode() {
    return std::move(m_bodyNode);
}

void FunctionDeclarationNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    m_identifier = std::move(identifier);
}

void FunctionDeclarationNode::setReturnTypeExpression(std::unique_ptr<Node> returnTypeExpression) {
    m_returnTypeExpression = std::move(returnTypeExpression);
}

void FunctionDeclarationNode::setParameters(std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters) {
    m_parameters = std::move(parameters);
}

void FunctionDeclarationNode::setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode) {
    m_bodyNode = std::move(bodyNode);
}

//IdentifierWithPossibleAnnotationNode
Node* IdentifierWithPossibleAnnotationNode::getAnnotation() const {
    return m_annotation.get();
}

std::unique_ptr<Node> IdentifierWithPossibleAnnotationNode::takeAnnotation() {
    return std::move(m_annotation);
}

void IdentifierWithPossibleAnnotationNode::setAnnotation(std::unique_ptr<Node> annotation) {
    m_annotation = std::move(annotation);
}

//BlockStatementNode
ExecutionListNode* BlockStatementNode::getExecutionListNode() const {
    return m_executionListNode.get();
}

const std::vector<Node*> BlockStatementNode::getChildren() const {
    return { m_executionListNode.get() };
}

std::unique_ptr<ExecutionListNode> BlockStatementNode::takeExecutionListNode() {
    return std::move(m_executionListNode);
}

void BlockStatementNode::setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode) {
    m_executionListNode = std::move(executionListNode);
}

//ExecutionListNode
const std::vector<Node*> ExecutionListNode::getChildren() const {
    std::vector<Node*> children;
    for (const auto& node : m_nodes) {
        children.push_back(node.get());
    }
    return children;
}

std::vector<std::unique_ptr<Node>> ExecutionListNode::takeChildren() {
    return std::move(m_nodes);
}

void ExecutionListNode::setChildren(std::vector<std::unique_ptr<Node>> children) {
    m_nodes = std::move(children);
}

void ExecutionListNode::addNode(std::unique_ptr<Node> node) {
    m_nodes.push_back(std::move(node));
}

//IfStatementNode
ExpressionNode* IfStatementNode::getCondition() const {
    return m_condition.get();
}

Node* IfStatementNode::getThenBranch() const {
    return m_thenBranch.get();
}

Node* IfStatementNode::getElseBranch() const {
    return m_elseBranch.get();
}

const std::vector<Node*> IfStatementNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(m_condition.get());
    if (m_thenBranch) {
        children.push_back(m_thenBranch.get());
    }
    if (m_elseBranch) {
        children.push_back(m_elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfStatementNode::takeCondition() {
    return std::move(m_condition);
}

std::unique_ptr<Node> IfStatementNode::takeThenBranch() {
    return std::move(m_thenBranch);
}

std::unique_ptr<Node> IfStatementNode::takeElseBranch() {
    return std::move(m_elseBranch);
}

void IfStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    m_condition = std::move(condition);
}

void IfStatementNode::setThenBranch(std::unique_ptr<Node> thenBranch) {
    m_thenBranch = std::move(thenBranch);
}

void IfStatementNode::setElseBranch(std::unique_ptr<Node> elseBranch) {
    m_elseBranch = std::move(elseBranch);
}

//WhileStatementNode
ExpressionNode* WhileStatementNode::getCondition() const {
    return m_condition.get();
}

BlockStatementNode* WhileStatementNode::getBody() const {
    return m_body.get();
}

const std::vector<Node*> WhileStatementNode::getChildren() const {
    return { m_condition.get(), m_body.get() };
}

std::unique_ptr<ExpressionNode> WhileStatementNode::takeCondition() {
    return std::move(m_condition);
}

std::unique_ptr<BlockStatementNode> WhileStatementNode::takeBody() {
    return std::move(m_body);
}

void WhileStatementNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    m_condition = std::move(condition);
}

void WhileStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    m_body = std::move(body);
}

//LoopStatementNode
BlockStatementNode* LoopStatementNode::getBody() const {
    return m_body.get();
}

const std::vector<Node*> LoopStatementNode::getChildren() const {
    return { m_body.get() };
}

std::unique_ptr<BlockStatementNode> LoopStatementNode::takeBody() {
    return std::move(m_body);
}

void LoopStatementNode::setBody(std::unique_ptr<BlockStatementNode> body) {
    m_body = std::move(body);
}

//ReturnStatementNode
ExpressionNode* ReturnStatementNode::getExpression() const {
    return m_expression.get();
}

const std::vector<Node*> ReturnStatementNode::getChildren() const {
    if (m_expression) {
        return { m_expression.get() };
    }
    return {};
}

std::unique_ptr<ExpressionNode> ReturnStatementNode::takeExpression() {
    return std::move(m_expression);
}

void ReturnStatementNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    m_expression = std::move(expression);
}

//AssignmentExpressionNode
IdentifierNode* AssignmentExpressionNode::getIdentifier() const {
    return m_identifier.get();
}

ExpressionNode* AssignmentExpressionNode::getExpression() const {
    return m_expression.get();
}

const std::vector<Node*> AssignmentExpressionNode::getChildren() const {
    return { m_identifier.get(), m_expression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentExpressionNode::takeIdentifier() {
    return std::move(m_identifier);
}

std::unique_ptr<ExpressionNode> AssignmentExpressionNode::takeExpression() {
    return std::move(m_expression);
}

void AssignmentExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    m_identifier = std::move(identifier);
}

void AssignmentExpressionNode::setExpression(std::unique_ptr<ExpressionNode> expression) {
    m_expression = std::move(expression);
}

//AssignmentStatementNode
IdentifierNode* AssignmentStatementNode::getIdentifier() const {
    return m_assignmentExpression->getIdentifier();
}

AssignmentExpressionNode* AssignmentStatementNode::getAssignmentExpression() const {
    return m_assignmentExpression.get();
}

const std::vector<Node*> AssignmentStatementNode::getChildren() const {
    return { m_assignmentExpression.get() };
}

std::unique_ptr<IdentifierNode> AssignmentStatementNode::takeIdentifier() {
    return m_assignmentExpression->takeIdentifier();
}

std::unique_ptr<AssignmentExpressionNode> AssignmentStatementNode::takeAssignmentExpression() {
    return std::move(m_assignmentExpression);
}

void AssignmentStatementNode::setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) {
    m_assignmentExpression = std::move(assignmentExpression);
}

void AssignmentStatementNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    m_assignmentExpression->setIdentifier(std::move(identifier));
}

//IdentifierNode
std::string_view IdentifierNode::getName() const {
    return m_identifierToken->getSourceString();
}

const std::vector<Node*> IdentifierNode::getChildren() const {
    return {};
}

Token* IdentifierNode::getIdentifierToken() const {
    return m_identifierToken.get();
}

//FunctionCallExpressionNode
IdentifierNode* FunctionCallExpressionNode::getIdentifier() const {
    return m_identifier.get();
}

const std::vector<ExpressionNode*> FunctionCallExpressionNode::getArgumentNodes() const {
    std::vector<ExpressionNode*> argumentPointers;
    for (const auto& argument : m_argumentNodes) {
        argumentPointers.push_back(argument.get());
    }
    return argumentPointers;
}

const std::vector<Node*> FunctionCallExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(m_identifier.get());
    for (const auto& argument : m_argumentNodes) {
        children.push_back(argument.get());
    }
    return children;
}

std::unique_ptr<IdentifierNode> FunctionCallExpressionNode::takeIdentifier() {
    return std::move(m_identifier);
}

std::vector<std::unique_ptr<ExpressionNode>> FunctionCallExpressionNode::takeArgumentNodes() {
    return std::move(m_argumentNodes);
}

void FunctionCallExpressionNode::setIdentifier(std::unique_ptr<IdentifierNode> identifier) {
    m_identifier = std::move(identifier);
}

void FunctionCallExpressionNode::setArgumentNodes(std::vector<std::unique_ptr<ExpressionNode>> argumentNodes) {
    m_argumentNodes = std::move(argumentNodes);
}

//FunctionCallStatementNode
FunctionCallExpressionNode* FunctionCallStatementNode::getFunctionCallExpression() const {
    return m_functionCallExpression.get();
}

const std::vector<Node*> FunctionCallStatementNode::getChildren() const {
    return { m_functionCallExpression.get() };
}

std::unique_ptr<FunctionCallExpressionNode> FunctionCallStatementNode::takeFunctionCallExpression() {
    return std::move(m_functionCallExpression);
}

void FunctionCallStatementNode::setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression) {
    m_functionCallExpression = std::move(functionCallExpression);
}

//IntegerLiteralNode
long long IntegerLiteralNode::getValue() const {
    return m_value;
}

Token* IntegerLiteralNode::getIntegerLiteralToken() const {
    return m_integerLiteralToken.get();
}

//FloatLiteralNode
double FloatLiteralNode::getValue() const {
    return m_value;
}

Token* FloatLiteralNode::getFloatLiteralToken() const {
    return m_floatLiteralToken.get();
}

//BooleanLiteralNode
bool BooleanLiteralNode::getValue() const {
    return m_value;
}

Token* BooleanLiteralNode::getBooleanLiteralToken() const {
    return m_booleanLiteralToken.get();
}

//EmptyLiteralNode
Token* EmptyLiteralNode::getEmptyLiteralToken() const {
    return m_emptyLiteralToken.get();
}

//StringLiteralNode
std::string StringLiteralNode::getValue() const {
    return std::string(m_stringLiteralToken->getSourceString());
}

//BinaryOperatorTypeExpressionNode
const std::vector<Node*> BinaryOperatorTypeExpressionNode::getChildren() const {
    return { m_left.get(), m_right.get() };
}

Node* BinaryOperatorTypeExpressionNode::getLeft() const {
    return m_left.get();
}

Node* BinaryOperatorTypeExpressionNode::getRight() const {
    return m_right.get();
}

Token* BinaryOperatorTypeExpressionNode::getOperatorToken() const {
    return m_operatorToken.get();
}

std::unique_ptr<Node> BinaryOperatorTypeExpressionNode::takeLeft() {
    return std::move(m_left);
}

std::unique_ptr<Node> BinaryOperatorTypeExpressionNode::takeRight() {
    return std::move(m_right);
}

void BinaryOperatorTypeExpressionNode::setLeft(std::unique_ptr<Node> left) {
    m_left = std::move(left);
}

void BinaryOperatorTypeExpressionNode::setRight(std::unique_ptr<Node> right) {
    m_right = std::move(right);
}

void BinaryOperatorTypeExpressionNode::setOperatorToken(std::unique_ptr<Token> operatorToken) {
    m_operatorToken = std::move(operatorToken);
}

//BinaryOperatorExpressionNode
ExpressionNode* BinaryOperatorExpressionNode::getLeft() const {
    return m_left.get();
}

ExpressionNode* BinaryOperatorExpressionNode::getRight() const {
    return m_right.get();
}

Token* BinaryOperatorExpressionNode::getOperatorToken() const {
    return m_operatorToken.get();
}

const std::vector<Node*> BinaryOperatorExpressionNode::getChildren() const {
    return { m_left.get(), m_right.get() };
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeLeft() {
    return std::move(m_left);
}

std::unique_ptr<ExpressionNode> BinaryOperatorExpressionNode::takeRight() {
    return std::move(m_right);
}

void BinaryOperatorExpressionNode::setLeft(std::unique_ptr<ExpressionNode> left) {
    m_left = std::move(left);
}

void BinaryOperatorExpressionNode::setRight(std::unique_ptr<ExpressionNode> right) {
    m_right = std::move(right);
}

//UnaryOperatorExpressionNode
ExpressionNode* UnaryOperatorExpressionNode::getOperand() const {
    return m_operand.get();
}

Token* UnaryOperatorExpressionNode::getOperatorToken() const {
    return m_operatorToken.get();
}

const std::vector<Node*> UnaryOperatorExpressionNode::getChildren() const {
    return { m_operand.get() };
}

std::unique_ptr<ExpressionNode> UnaryOperatorExpressionNode::takeOperand() {
    return std::move(m_operand);
}

void UnaryOperatorExpressionNode::setOperand(std::unique_ptr<ExpressionNode> operand) {
    m_operand = std::move(operand);
}

//IfExpressionNode
ExpressionNode* IfExpressionNode::getCondition() const {
    return m_condition.get();
}

ExpressionNode* IfExpressionNode::getThenBranch() const {
    return m_thenBranch.get();
}

ExpressionNode* IfExpressionNode::getElseBranch() const {
    return m_elseBranch.get();
}

const std::vector<Node*> IfExpressionNode::getChildren() const {
    std::vector<Node*> children;
    children.push_back(m_condition.get());
    children.push_back(m_thenBranch.get());
    if (m_elseBranch) {
        children.push_back(m_elseBranch.get());
    }
    return children;
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeCondition() {
    return std::move(m_condition);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeThenBranch() {
    return std::move(m_thenBranch);
}

std::unique_ptr<ExpressionNode> IfExpressionNode::takeElseBranch() {
    return std::move(m_elseBranch);
}

void IfExpressionNode::setCondition(std::unique_ptr<ExpressionNode> condition) {
    m_condition = std::move(condition);
}

void IfExpressionNode::setThenBranch(std::unique_ptr<ExpressionNode> thenBranch) {
    m_thenBranch = std::move(thenBranch);
}

void IfExpressionNode::setElseBranch(std::unique_ptr<ExpressionNode> elseBranch) {
    m_elseBranch = std::move(elseBranch);
}

//PrimaryTypeExpressionNode

//TypePrimitiveNode
PrimitiveTypeKind TypePrimitiveNode::getPrimitiveTypeKind() const {
    switch (m_token->getTokenKind()) {
        case TokenKind::TypePrimitiveFloat:
            return PrimitiveTypeKind::Float;
        case TokenKind::TypePrimitiveInteger:
            return PrimitiveTypeKind::Integer;
        case TokenKind::TypePrimitiveString:
            return PrimitiveTypeKind::String;
        case TokenKind::TypePrimitiveEmpty:
            return PrimitiveTypeKind::Empty;
        case TokenKind::TypePrimitiveBoolean:
            return PrimitiveTypeKind::Boolean;
        /*case TokenKind::TypePrimitiveAny:
            return PrimitiveTypeKind::Any;
        case TokenKind::TypePrimitiveVoid:
            return PrimitiveTypeKind::Void;*/ // TODO !!! FIXME
        case TokenKind::TypePrimitiveAny:
        case TokenKind::TypePrimitiveVoid:
        default:
            return PrimitiveTypeKind::Invalid; // TODO Add error or something
    }
}

//TypeDeclarationNode
IdentifierNode* TypeDeclarationNode::getIdentifier() const {
    return m_identifierNode.get();
}

Node* TypeDeclarationNode::getTypeExpression() const {
    return m_typeExpressionNode.get();
}

const std::vector<Node*> TypeDeclarationNode::getChildren() const {
    return { m_identifierNode.get(), m_typeExpressionNode.get() };
}


#pragma once

#include <memory>
#include <string>
#include <vector>

#include "token.hpp"


/*
Rough Grammar:
Program ::= (Declaration | Statement)*
Declaration ::= VariableDeclaration | FunctionDeclaration
VariableDeclaration ::= 'var' Identifier AssignmentExpression? ';'
FunctionDeclaration ::= 'function' Identifier '(' ParameterList? ')' BlockStatement
ParameterList ::= Identifier (',' Identifier)*
Statement ::= BlockStatement | IfStatement | WhileStatement | BreakStatement | ContinueStatement | ReturnStatement | AssignmentStatement | FunctionCallStatement
BlockStatement ::= '{' Program '}'
IfStatement ::= 'if' '(' Expression ')' Statement ('else' Statement)?
WhileStatement ::= 'while' '(' Expression ')' Statement
BreakStatement ::= 'break' ';'
ContinueStatement ::= 'continue' ';'
ReturnStatement ::= 'return' Expression? ';'
AssignmentStatement ::= AssignmentExpression ';'
AssignmentExpression ::= Identifier '=' Expression
FunctionCallStatement ::= FunctionCallExpression ';'
FunctionCallExpression ::= Identifier '(' ArgumentList? ')'
ArgumentList ::= Expression (',' Expression)*
Expression ::= PrimaryExpression | BinaryOperatorExpression
BinaryOperator ::= '+' | '-' | '*' | '/' | '==' | '!='
UnaryOperator ::= '-' | '!'
BinaryOperatorExpression ::= Expression BinaryOperator Expression
UnaryOperatorExpression ::= UnaryOperator Expression
ObjectLiteral ::= '[' ':' | ((Identifier ':' Expression) (',' Identifier ':' Expression)*) ']'
ArrayLiteral ::= '[' ',' | (Expression (',' Expression)*) ']'
Literal ::= NumberLiteral | StringLiteral | ObjectLiteral | ArrayLiteral
PrimaryExpression ::= Identifier | FunctionCallExpression | UnaryOperatorExpression | Literal | '(' Expression ')'
*/

enum class NodeType {
    Program,
    Invalid,
    FunctionDeclaration,
    VariableDeclaration,
    BlockStatement,
    IfStatement,
    WhileStatement,
    BreakStatement,
    ContinueStatement,
    ReturnStatement,
    AssignmentStatement,
    FunctionCallStatement,
    //ExpressionStatement,
    //BinaryExpression,
    Identifier,
    NumberLiteral,
    StringLiteral,
    //ObjectLiteral, // TODO later
    //ArrayLiteral, // TODO later
    AssignmentExpression,
    FunctionCallExpression,
    BinaryOperatorExpression,
    UnaryOperatorExpression,
};

class Node {
public:
    Node(NodeType type) : nodeType(type) {};
    virtual ~Node() = default;
    NodeType getNodeType() const;
    bool operator==(const NodeType& rhs) const;
    bool operator!=(const NodeType& rhs) const;
private:
    NodeType nodeType;
};

class InvalidNode : public Node {
public:
    InvalidNode() : Node(NodeType::Invalid) {};
};

class ProgramNode : public Node {
public:
    ProgramNode() : Node(NodeType::Program) {};
    const std::vector<std::unique_ptr<Node>>& getChildren() const;
    void addChild(std::unique_ptr<Node> child);
private:
    std::vector<std::unique_ptr<Node>> children;
};

class ExpressionNode : public Node {
public:
    ExpressionNode(NodeType type) : Node(type) {};
};

class PrimaryExpressionNode : public ExpressionNode {
public:    PrimaryExpressionNode(NodeType type) : ExpressionNode(type) {};
};

class IdentifierNode : public PrimaryExpressionNode {
public:
    IdentifierNode(std::unique_ptr<Token> identifierToken) : PrimaryExpressionNode(NodeType::Identifier), identifierToken(std::move(identifierToken)) {};
    std::string getName() const;
    Token* getIdentifierToken() const;
private:
    std::unique_ptr<Token> identifierToken;
};

class AssignmentExpressionNode : public ExpressionNode {
public:
    AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression) : ExpressionNode(NodeType::AssignmentExpression), identifier(std::move(identifier)), expression(std::move(expression)) {};
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    ExpressionNode* getExpression() const;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<ExpressionNode> expression;
};

class VariableDeclarationNode : public Node {
public:
    VariableDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<AssignmentExpressionNode> assignmentExpression) : Node(NodeType::VariableDeclaration), identifier(std::move(identifier)), assignmentExpression(std::move(assignmentExpression)) {};
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    AssignmentExpressionNode* getAssignmentExpression() const;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<AssignmentExpressionNode> assignmentExpression;
};

class BlockStatementNode : public Node {
public:
    BlockStatementNode(std::unique_ptr<ProgramNode> programNode) : Node(NodeType::BlockStatement), programNode(std::move(programNode)) {};
    ProgramNode* getProgramNode() const;
private:
    std::unique_ptr<ProgramNode> programNode;
};

class FunctionDeclarationNode : public Node {
public:
    FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode) : Node(NodeType::FunctionDeclaration), identifier(std::move(identifier)), parameters(std::move(parameters)), bodyNode(std::move(bodyNode)) {};
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    const std::vector<std::unique_ptr<IdentifierNode>>& getParameters() const;
    BlockStatementNode* getBody() const;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::vector<std::unique_ptr<IdentifierNode>> parameters;
    std::unique_ptr<BlockStatementNode> bodyNode;
};

class IfStatementNode : public Node {
public:
    IfStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch) : Node(NodeType::IfStatement), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {};
    ExpressionNode* getCondition() const;
    Node* getThenBranch() const; // TODO maybe not 'Node'
    Node* getElseBranch() const;
private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<Node> thenBranch;
    std::unique_ptr<Node> elseBranch;
};

class WhileStatementNode : public Node {
public:
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BlockStatementNode> body) : Node(NodeType::WhileStatement), condition(std::move(condition)), body(std::move(body)) {};
    ExpressionNode* getCondition() const;
    BlockStatementNode* getBody() const;
private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockStatementNode> body;
};

class BreakStatementNode : public Node {
public:
    BreakStatementNode() : Node(NodeType::BreakStatement) {};
};

class ContinueStatementNode : public Node {
public:
    ContinueStatementNode() : Node(NodeType::ContinueStatement) {};
};

class ReturnStatementNode : public Node {
public:
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression) : Node(NodeType::ReturnStatement), expression(std::move(expression)) {};
    ExpressionNode* getExpression() const;
private:
    std::unique_ptr<ExpressionNode> expression;
};

class AssignmentStatementNode : public Node {
public:
    AssignmentStatementNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) : Node(NodeType::AssignmentStatement), assignmentExpression(std::move(assignmentExpression)) {};
    IdentifierNode* getIdentifier() const;
    AssignmentExpressionNode* getAssignmentExpression() const;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<AssignmentExpressionNode> assignmentExpression;
};

class FunctionCallExpressionNode : public PrimaryExpressionNode {
public:
    FunctionCallExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<ExpressionNode>> argumentNodes) : PrimaryExpressionNode(NodeType::FunctionCallExpression), identifier(std::move(identifier)), argumentNodes(std::move(argumentNodes)) {};
    IdentifierNode* getIdentifier() const;
    const std::vector<std::unique_ptr<ExpressionNode>>& getArgumentNodes() const;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::vector<std::unique_ptr<ExpressionNode>> argumentNodes;
};

class FunctionCallStatementNode : public Node {
public:
    FunctionCallStatementNode(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression) : Node(NodeType::FunctionCallStatement), functionCallExpression(std::move(functionCallExpression)) {};
    FunctionCallExpressionNode* getFunctionCallExpression() const;
private:
    std::unique_ptr<FunctionCallExpressionNode> functionCallExpression;
};

class StringLiteralNode : public PrimaryExpressionNode {
public:
    StringLiteralNode(std::unique_ptr<Token> stringLiteralToken) : PrimaryExpressionNode(NodeType::StringLiteral), stringLiteralToken(std::move(stringLiteralToken)) {};
    std::string getValue() const;
    Token* getStringLiteralToken() const;
private:
    std::unique_ptr<Token> stringLiteralToken;
};

class NumberLiteralNode : public PrimaryExpressionNode {
public:
    NumberLiteralNode(std::unique_ptr<Token> numberLiteralToken) : PrimaryExpressionNode(NodeType::NumberLiteral), numberLiteralToken(std::move(numberLiteralToken)) {};
    int getValue() const;
    Token* getNumberLiteralToken() const;
private:
    std::unique_ptr<Token> numberLiteralToken;
};

class BinaryOperatorExpressionNode : public ExpressionNode {
public:
    BinaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, std::unique_ptr<Token> operatorToken) : ExpressionNode(NodeType::BinaryOperatorExpression), left(std::move(left)), right(std::move(right)), operatorToken(std::move(operatorToken)) {};
    ExpressionNode* getLeft() const;
    ExpressionNode* getRight() const;
    Token* getOperatorToken() const;
private:
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    std::unique_ptr<Token> operatorToken;
};

class UnaryOperatorExpressionNode : public PrimaryExpressionNode {
public:
    UnaryOperatorExpressionNode(std::unique_ptr<PrimaryExpressionNode> operand, std::unique_ptr<Token> operatorToken) : PrimaryExpressionNode(NodeType::UnaryOperatorExpression), operand(std::move(operand)), operatorToken(std::move(operatorToken)) {};
    PrimaryExpressionNode* getOperand() const;
    Token* getOperatorToken() const;
private:
    std::unique_ptr<PrimaryExpressionNode> operand;
    std::unique_ptr<Token> operatorToken;
};
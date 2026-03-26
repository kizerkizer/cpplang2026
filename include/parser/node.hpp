#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/sourcecodelocation.hpp"
#include "lexer/token.hpp"


/*
Rough Grammar:
Program ::= (Declaration | Statement)*
Declaration ::= VariableDeclaration | FunctionDeclaration
VariableDeclaration ::= 'var' Identifier AssignmentExpression? ';'
FunctionDeclaration ::= 'function' Identifier '(' ParameterList? ')' BlockStatement
ParameterList ::= Identifier (',' Identifier)*
Statement ::= BlockStatement | IfStatement | WhileStatement | LoopStatement | BreakStatement | ContinueStatement | ReturnStatement | AssignmentStatement | FunctionCallStatement
BlockStatement ::= '{' Program '}'
IfStatement ::= 'if' Expression 'then' BlockStatement ('else' BlockStatement)?
WhileStatement ::= 'while' '(' Expression ')' Statement
LoopStatement ::= 'loop' '(' Expression ')' Statement
BreakStatement ::= 'break' ';'
ContinueStatement ::= 'continue' ';'
ReturnStatement ::= 'return' Expression? ';'
AssignmentStatement ::= AssignmentExpression ';'
AssignmentExpression ::= Identifier '=' Expression
FunctionCallStatement ::= FunctionCallExpression ';'
FunctionCallExpression ::= Identifier '(' ArgumentList? ')'
IfExpression ::= 'if' Expression 'then' Expression ('else' Expression)?
ArgumentList ::= Expression (',' Expression)*
Expression ::= PrimaryExpression | BinaryOperatorExpression
BinaryOperator ::= '+' | '-' | '*' | '/' | '==' | '!=' | '&&' | '||'
UnaryOperator ::= '-' | '!'
BinaryOperatorExpression ::= Expression BinaryOperator Expression
UnaryOperatorExpression ::= UnaryOperator Expression
ObjectLiteral ::= '[' ':' | ((Identifier ':' Expression) (',' Identifier ':' Expression)*) ']'
ArrayLiteral ::= '[' ',' | (Expression (',' Expression)*) ']'
Literal ::= NumberLiteral | StringLiteral | BooleanLiteral | EmptyLiteral | ObjectLiteral | ArrayLiteral
PrimaryExpression ::= Identifier | FunctionCallExpression | IfExpression | UnaryOperatorExpression | Literal | '(' Expression ')'
*/

class Name; // in binder/name.hpp

enum class NodeKind {
    Program,
    Invalid,
    FunctionDeclaration,
    VariableDeclaration,
    BlockStatement,
    IfStatement,
    WhileStatement,
    LoopStatement,
    BreakStatement,
    ContinueStatement,
    ReturnStatement,
    AssignmentStatement,
    FunctionCallStatement,
    Identifier,
    NumberLiteral,
    StringLiteral,
    BooleanLiteral,
    EmptyLiteral,
    //ObjectLiteral, // TODO later
    //ArrayLiteral, // TODO later
    AssignmentExpression,
    FunctionCallExpression,
    BinaryOperatorExpression,
    UnaryOperatorExpression,
    IfExpression,
};

std::string nodeKindToString(NodeKind nodeKind);

#define IS_NODEKIND_LITERAL(nodeKind) (nodeKind == NodeKind::NumberLiteral || nodeKind == NodeKind::StringLiteral || nodeKind == NodeKind::BooleanLiteral || nodeKind == NodeKind::EmptyLiteral)

class Node {
public:
    Node(NodeKind type, bool compilerCreated = false) : nodeKind(type), compilerCreated(compilerCreated) {};
    virtual ~Node() = default;
    NodeKind getNodeKind() const;
    bool operator==(const NodeKind& rhs) const;
    bool operator!=(const NodeKind& rhs) const;
    virtual const std::vector<Node*> getChildren() const = 0;
    bool isCompilerCreated() const;
    void addToken(std::unique_ptr<Token> token);
    std::vector<Token*> getTokens();
    SourceCodeLocation getFirstSourceCodeLocation();
    SourceCodeLocation getLastSourceCodeLocation();
private:
    NodeKind nodeKind;
    bool compilerCreated;
    std::vector<std::unique_ptr<Token>> tokens;
};

class InvalidNode : public Node {
public:
    InvalidNode() : Node(NodeKind::Invalid) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class ProgramNode : public Node {
public:
    ProgramNode() : Node(NodeKind::Program) {};
    void addNode(std::unique_ptr<Node> child);
    const std::vector<Node*> getChildren() const override;
    std::vector<std::unique_ptr<Node>> takeChildren();
    void setChildren(std::vector<std::unique_ptr<Node>> children);
private:
    std::vector<std::unique_ptr<Node>> nodes;
};

class ExpressionNode : public Node {
public:
    ExpressionNode(NodeKind type) : Node(type) {};
};

class PrimaryExpressionNode : public ExpressionNode {
public:
    PrimaryExpressionNode(NodeKind type) : ExpressionNode(type) {};
};

class IdentifierNode : public PrimaryExpressionNode {
public:
    IdentifierNode(std::unique_ptr<Token> identifierToken) : PrimaryExpressionNode(NodeKind::Identifier), identifierToken(std::move(identifierToken)) {};
    std::string getName() const;
    Token* getIdentifierToken() const;
    const std::vector<Node*> getChildren() const override;
    Name* getNameReference() const;
    void setNameReference(Name* name);
private:
    std::unique_ptr<Token> identifierToken;
    Name* name = nullptr; // Set during binding
};

class AssignmentExpressionNode : public ExpressionNode {
public:
    AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression);
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    ExpressionNode* getExpression() const;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<ExpressionNode> takeExpression();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setExpression(std::unique_ptr<ExpressionNode> expression);
    const std::vector<Node*> getChildren() const override;
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<ExpressionNode> expression;
};

class VariableDeclarationNode : public Node {
public:
    VariableDeclarationNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression);
    AssignmentExpressionNode* getAssignmentExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<AssignmentExpressionNode> takeAssignmentExpression();
    void setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression);
private:
    std::unique_ptr<AssignmentExpressionNode> assignmentExpression;
};

class BlockStatementNode : public Node {
public:
    BlockStatementNode(std::unique_ptr<ProgramNode> programNode);
    ProgramNode* getProgramNode() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ProgramNode> takeProgramNode();
    void setProgramNode(std::unique_ptr<ProgramNode> programNode);
private:
    std::unique_ptr<ProgramNode> programNode;
};

class FunctionDeclarationNode : public Node {
public:
    FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode);
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    const std::vector<IdentifierNode*> getParameters() const;
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::vector<std::unique_ptr<IdentifierNode>> takeParameters();
    std::unique_ptr<BlockStatementNode> takeBodyNode();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setParameters(std::vector<std::unique_ptr<IdentifierNode>> parameters);
    void setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode);
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::vector<std::unique_ptr<IdentifierNode>> parameters;
    std::unique_ptr<BlockStatementNode> bodyNode;
};

class IfStatementNode : public Node {
public:
    IfStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch) : Node(NodeKind::IfStatement), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {};
    ExpressionNode* getCondition() const;
    Node* getThenBranch() const;
    Node* getElseBranch() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeCondition();
    std::unique_ptr<Node> takeThenBranch();
    std::unique_ptr<Node> takeElseBranch();
    void setCondition(std::unique_ptr<ExpressionNode> condition);
    void setThenBranch(std::unique_ptr<Node> thenBranch);
    void setElseBranch(std::unique_ptr<Node> elseBranch);
private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<Node> thenBranch;
    std::unique_ptr<Node> elseBranch;
};

class LoopStatementNode : public Node {
public:    LoopStatementNode(std::unique_ptr<BlockStatementNode> body) : Node(NodeKind::LoopStatement), body(std::move(body)) {};
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<BlockStatementNode> takeBody();
    void setBody(std::unique_ptr<BlockStatementNode> body);
private:
    std::unique_ptr<BlockStatementNode> body;
};

class WhileStatementNode : public Node {
public:
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BlockStatementNode> body) : Node(NodeKind::WhileStatement), condition(std::move(condition)), body(std::move(body)) {};
    ExpressionNode* getCondition() const;
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeCondition();
    std::unique_ptr<BlockStatementNode> takeBody();
    void setCondition(std::unique_ptr<ExpressionNode> condition);
    void setBody(std::unique_ptr<BlockStatementNode> body);
private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockStatementNode> body;
};

class BreakStatementNode : public Node {
public:
    BreakStatementNode() : Node(NodeKind::BreakStatement) {};
    Name* getLoopNameReference() const;
    void setLoopNameReference(Name* name);
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    Name* loopName; // set during binding
};

class ContinueStatementNode : public Node {
public:
    ContinueStatementNode() : Node(NodeKind::ContinueStatement) {};
    Name* getLoopNameReference() const;
    void setLoopNameReference(Name* name);
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    Name* loopName; // set during binding
};

class ReturnStatementNode : public Node {
public:
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression) : Node(NodeKind::ReturnStatement), expression(std::move(expression)) {};
    ExpressionNode* getExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeExpression();
    void setExpression(std::unique_ptr<ExpressionNode> expression);
    Name *getFunctionNameReference();
    void setFunctionNameReference(Name *functionName);
private:
    std::unique_ptr<ExpressionNode> expression;
    Name *functionName = nullptr; // Set during binding
};

class AssignmentStatementNode : public Node {
public:
    AssignmentStatementNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression) : Node(NodeKind::AssignmentStatement), assignmentExpression(std::move(assignmentExpression)) {};
    IdentifierNode* getIdentifier() const;
    AssignmentExpressionNode* getAssignmentExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<AssignmentExpressionNode> takeAssignmentExpression();
    void setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression);
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::unique_ptr<AssignmentExpressionNode> assignmentExpression;
};

class FunctionCallExpressionNode : public PrimaryExpressionNode {
public:
    FunctionCallExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<ExpressionNode>> argumentNodes) : PrimaryExpressionNode(NodeKind::FunctionCallExpression), identifier(std::move(identifier)), argumentNodes(std::move(argumentNodes)) {};
    IdentifierNode* getIdentifier() const;
    const std::vector<const ExpressionNode*> getArgumentNodes() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::vector<std::unique_ptr<ExpressionNode>> takeArgumentNodes();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setArgumentNodes(std::vector<std::unique_ptr<ExpressionNode>> argumentNodes);
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::vector<std::unique_ptr<ExpressionNode>> argumentNodes;
};

class FunctionCallStatementNode : public Node {
public:
    FunctionCallStatementNode(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression) : Node(NodeKind::FunctionCallStatement), functionCallExpression(std::move(functionCallExpression)) {};
    FunctionCallExpressionNode* getFunctionCallExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<FunctionCallExpressionNode> takeFunctionCallExpression();
    void setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression);
private:
    std::unique_ptr<FunctionCallExpressionNode> functionCallExpression;
};

class StringLiteralNode : public PrimaryExpressionNode {
public:
    StringLiteralNode(std::unique_ptr<Token> stringLiteralToken) : PrimaryExpressionNode(NodeKind::StringLiteral), stringLiteralToken(std::move(stringLiteralToken)) {};
    std::string getValue() const;
    Token* getStringLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    std::unique_ptr<Token> stringLiteralToken;
};

class BooleanLiteralNode : public PrimaryExpressionNode {
public:
    BooleanLiteralNode(std::unique_ptr<Token> booleanLiteralToken) : PrimaryExpressionNode(NodeKind::BooleanLiteral), booleanLiteralToken(std::move(booleanLiteralToken)) {};
    bool getValue() const;
    Token* getBooleanLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    std::unique_ptr<Token> booleanLiteralToken;
};

class EmptyLiteralNode : public PrimaryExpressionNode {
public:
    EmptyLiteralNode(std::unique_ptr<Token> emptyLiteralToken) : PrimaryExpressionNode(NodeKind::EmptyLiteral), emptyLiteralToken(std::move(emptyLiteralToken)) {};
    Token* getEmptyLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    std::unique_ptr<Token> emptyLiteralToken;
};

class NumberLiteralNode : public PrimaryExpressionNode {
public:
    NumberLiteralNode(std::unique_ptr<Token> numberLiteralToken) : PrimaryExpressionNode(NodeKind::NumberLiteral), numberLiteralToken(std::move(numberLiteralToken)) {};
    int getValue() const;
    Token* getNumberLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    std::unique_ptr<Token> numberLiteralToken;
};

class BinaryOperatorExpressionNode : public ExpressionNode {
public:
    BinaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, std::unique_ptr<Token> operatorToken) : ExpressionNode(NodeKind::BinaryOperatorExpression), left(std::move(left)), right(std::move(right)), operatorToken(std::move(operatorToken)) {};
    ExpressionNode* getLeft() const;
    ExpressionNode* getRight() const;
    Token* getOperatorToken() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeLeft();
    std::unique_ptr<ExpressionNode> takeRight();
    void setLeft(std::unique_ptr<ExpressionNode> left);
    void setRight(std::unique_ptr<ExpressionNode> right);
private:
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;
    std::unique_ptr<Token> operatorToken;
};

class UnaryOperatorExpressionNode : public PrimaryExpressionNode {
public:
    UnaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> operand, std::unique_ptr<Token> operatorToken) : PrimaryExpressionNode(NodeKind::UnaryOperatorExpression), operand(std::move(operand)), operatorToken(std::move(operatorToken)) {};
    ExpressionNode* getOperand() const;
    Token* getOperatorToken() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeOperand();
    void setOperand(std::unique_ptr<ExpressionNode> operand);
private:
    std::unique_ptr<ExpressionNode> operand;
    std::unique_ptr<Token> operatorToken;
};

class IfExpressionNode : public ExpressionNode {
public:
    IfExpressionNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<ExpressionNode> thenBranch, std::unique_ptr<ExpressionNode> elseBranch) : ExpressionNode(NodeKind::IfExpression), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {};
    ExpressionNode* getCondition() const;
    ExpressionNode* getThenBranch() const;
    ExpressionNode* getElseBranch() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeCondition();
    std::unique_ptr<ExpressionNode> takeThenBranch();
    std::unique_ptr<ExpressionNode> takeElseBranch();
    void setCondition(std::unique_ptr<ExpressionNode> condition);
    void setThenBranch(std::unique_ptr<ExpressionNode> thenBranch);
    void setElseBranch(std::unique_ptr<ExpressionNode> elseBranch);
private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ExpressionNode> thenBranch;
    std::unique_ptr<ExpressionNode> elseBranch;
};
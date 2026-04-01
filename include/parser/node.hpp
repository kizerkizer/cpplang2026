#pragma once

#include <memory>
#include <string>
#include <vector>

#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "common/programuniqueid.hpp"
#include "flowbuilder/flowgraph.hpp"
#include "flowbuilder/flownode.hpp"
#include "lexer/token.hpp"

/*
Rough Grammar:
Program ::= (Declaration | Statement)*
Declaration ::= VariableDeclaration | FunctionDeclaration
VariableDeclaration ::= 'var' Identifier (':' TypeExpression)? AssignmentExpression? ';'
FunctionDeclaration ::= 'function' Identifier '(' ParameterList? ')' (':' TypeExpression)? BlockStatement
TypeExpression = PrimitiveType//PrimaryTypeExpression | BinaryOperatorTypeExpression
//UnionTypeExpression = TypeExpression '|' TypeExpression
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

class Symbol; // in binder/symbol.hpp

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
    TypeExpression,
    IdentifierWithPossibleAnnotation,
};

std::string nodeKindToString(NodeKind nodeKind);

#define IS_NODEKIND_LITERAL(nodeKind) (nodeKind == NodeKind::NumberLiteral || nodeKind == NodeKind::StringLiteral || nodeKind == NodeKind::BooleanLiteral || nodeKind == NodeKind::EmptyLiteral)

class Node {
public:
    Node(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan, bool compilerCreated = false) : id(getNextId()), nodeKind(type), sourceCodeLocationSpan(sourceCodeLocationSpan), compilerCreated(compilerCreated) {};
    virtual ~Node() = default;
    int getId() const;
    NodeKind getNodeKind() const;
    bool operator==(const NodeKind& rhs) const;
    bool operator!=(const NodeKind& rhs) const;
    virtual const std::vector<Node*> getChildren() const = 0;
    bool isCompilerCreated() const;
    void addToken(std::unique_ptr<Token> token);
    std::vector<Token*> getTokens();
    /*SourceCodeLocation getFirstSourceCodeLocation();
    SourceCodeLocation getLastSourceCodeLocation();*/
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
    FlowNode* getFlowNode();
    void setFlowNode(FlowNode* flowNode);
    bool isReachable() const; // set by flow builder
    void setReachable(bool reachable); // set by flow builder
    Type* getType() const;
    void setType(Type* type);
private:
    int id;
    NodeKind nodeKind;
    SourceCodeLocationSpan sourceCodeLocationSpan;
    bool compilerCreated;
    FlowNode* flowNode = nullptr; // set by flow builder
    bool reachable = false; // set by flow builder
    Type* type = nullptr; // set by type checker
    std::vector<std::unique_ptr<Token>> tokens;
};

class InvalidNode : public Node {
public:
    InvalidNode() : Node(NodeKind::Invalid, SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1))) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class TypeExpressionNode : public Node {
public:
    TypeExpressionNode(std::unique_ptr<Token> token, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::TypeExpression, sourceCodeLocationSpan), token(std::move(token)) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
    PrimitiveTypeKind getPrimitiveTypeKind() const; // TODO eventually move to TypePrimitiveNode
private:
    std::unique_ptr<Token> token;
};

class ProgramNode : public Node {
public:
    ProgramNode(SourceCodeLocationSpan sourceCodeLocationSpan, bool isRoot = false) : Node(NodeKind::Program, sourceCodeLocationSpan), isRoot(isRoot) {};
    void addNode(std::unique_ptr<Node> child);
    const std::vector<Node*> getChildren() const override;
    std::vector<std::unique_ptr<Node>> takeChildren();
    void setChildren(std::vector<std::unique_ptr<Node>> children);
    bool isRootNode();
    FlowGraph* getFlowGraph(); // only for the root node
    void setFlowGraph(FlowGraph* flowGraph); // only for the root node
private:
    std::vector<std::unique_ptr<Node>> nodes;
    bool isRoot;
    FlowGraph* flowGraph;
};

class ExpressionNode : public Node {
public:
    ExpressionNode(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(type, sourceCodeLocationSpan) {};
};

class PrimaryExpressionNode : public ExpressionNode {
public:
    PrimaryExpressionNode(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(type, sourceCodeLocationSpan) {};
};

class IdentifierNode : public PrimaryExpressionNode {
public:
    IdentifierNode(std::unique_ptr<Token> identifierToken, SourceCodeLocationSpan sourceCodeLocationSpan, NodeKind kind = NodeKind::Identifier) : PrimaryExpressionNode(kind, sourceCodeLocationSpan), identifierToken(std::move(identifierToken)) {};
    std::string getName() const;
    Token* getIdentifierToken() const;
    const std::vector<Node*> getChildren() const override;
    Symbol* getSymbolReference() const;
    void setSymbolReference(Symbol* name);
private:
    std::unique_ptr<Token> identifierToken;
    Symbol* symbol = nullptr; // Set during binding
};

class AssignmentExpressionNode : public ExpressionNode {
public:
    AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::AssignmentExpression, sourceCodeLocationSpan), identifier(std::move(identifier)), expression(std::move(expression)) {};
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
    VariableDeclarationNode(std::unique_ptr<IdentifierNode> identifierNode, std::unique_ptr<TypeExpressionNode> typeExpression, std::unique_ptr<ExpressionNode> expressionNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::VariableDeclaration, sourceCodeLocationSpan), identifierNode(std::move(identifierNode)), typeExpression(std::move(typeExpression)), expressionNode(std::move(expressionNode)) {};
    IdentifierNode* getIdentifier() const;
    TypeExpressionNode* getTypeExpression() const;
    ExpressionNode* getExpression() const;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<ExpressionNode> takeExpression();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifierNode);
    void setExpression(std::unique_ptr<ExpressionNode> expressionNode);
    const std::vector<Node*> getChildren() const override;
private:
    std::unique_ptr<IdentifierNode> identifierNode;
    std::unique_ptr<TypeExpressionNode> typeExpression;
    std::unique_ptr<ExpressionNode> expressionNode;
};

class BlockStatementNode : public Node {
public:
    BlockStatementNode(std::unique_ptr<ProgramNode> programNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::BlockStatement, sourceCodeLocationSpan), programNode(std::move(programNode)) {};
    ProgramNode* getProgramNode() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ProgramNode> takeProgramNode();
    void setProgramNode(std::unique_ptr<ProgramNode> programNode);
private:
    std::unique_ptr<ProgramNode> programNode;
};

class IdentifierWithPossibleAnnotationNode : public IdentifierNode {
public:
    IdentifierWithPossibleAnnotationNode(std::unique_ptr<Token> token, std::unique_ptr<TypeExpressionNode> annotation, SourceCodeLocationSpan sourceCodeLocationSpan) : IdentifierNode(std::move(token), sourceCodeLocationSpan, NodeKind::IdentifierWithPossibleAnnotation), annotation(std::move(annotation)) {};
    TypeExpressionNode* getAnnotation() const;
    std::unique_ptr<TypeExpressionNode> takeAnnotation();
    void setAnnotation(std::unique_ptr<TypeExpressionNode> annotation);
private:
    std::unique_ptr<TypeExpressionNode> annotation;
};

class FunctionDeclarationNode : public Node {
public:
    FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode, std::unique_ptr<TypeExpressionNode> returnTypeExpression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::FunctionDeclaration, sourceCodeLocationSpan), identifier(std::move(identifier)), parameters(std::move(parameters)), returnTypeExpression(std::move(returnTypeExpression)), bodyNode(std::move(bodyNode)) {};
    FlowGraph* getFlowGraph(); // set by flow builder
    void setFlowGraph(FlowGraph* flowGraph); // set by flow builder
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    TypeExpressionNode* getReturnTypeExpression() const;
    const std::vector<IdentifierWithPossibleAnnotationNode*> getParameters() const;
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<TypeExpressionNode> takeReturnTypeExpression();
    std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> takeParameters();
    std::unique_ptr<BlockStatementNode> takeBodyNode();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setReturnTypeExpression(std::unique_ptr<TypeExpressionNode> returnTypeExpression);
    void setParameters(std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters);
    void setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode);
private:
    std::unique_ptr<IdentifierNode> identifier;
    std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters;
    std::unique_ptr<TypeExpressionNode> returnTypeExpression;
    std::unique_ptr<BlockStatementNode> bodyNode;
    FlowGraph* flowGraph = nullptr; // set by flowbuilder
};

class IfStatementNode : public Node {
public:
    IfStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::IfStatement, sourceCodeLocationSpan), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {};
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
public:    LoopStatementNode(std::unique_ptr<BlockStatementNode> body, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::LoopStatement, sourceCodeLocationSpan), body(std::move(body)) {};
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<BlockStatementNode> takeBody();
    void setBody(std::unique_ptr<BlockStatementNode> body);
private:
    std::unique_ptr<BlockStatementNode> body;
};

class WhileStatementNode : public Node {
public:
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BlockStatementNode> body, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::WhileStatement, sourceCodeLocationSpan), condition(std::move(condition)), body(std::move(body)) {};
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
    BreakStatementNode(SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::BreakStatement, sourceCodeLocationSpan) {};
    Symbol* getLoopNameReference() const;
    void setLoopNameReference(Symbol* name);
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    Symbol* loopName; // set during binding
};

class ContinueStatementNode : public Node {
public:
    ContinueStatementNode(SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::ContinueStatement, sourceCodeLocationSpan) {};
    Symbol* getLoopNameReference() const;
    void setLoopNameReference(Symbol* name);
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    Symbol* loopName; // set during binding
};

class ReturnStatementNode : public Node {
public:
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::ReturnStatement, sourceCodeLocationSpan), expression(std::move(expression)) {};
    ExpressionNode* getExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeExpression();
    void setExpression(std::unique_ptr<ExpressionNode> expression);
    Symbol *getFunctionNameReference();
    void setFunctionNameReference(Symbol *functionName);
private:
    std::unique_ptr<ExpressionNode> expression;
    Symbol *functionName = nullptr; // Set during binding
};

class AssignmentStatementNode : public Node {
public:
    AssignmentStatementNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::AssignmentStatement, sourceCodeLocationSpan), assignmentExpression(std::move(assignmentExpression)) {};
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
    FunctionCallExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<ExpressionNode>> argumentNodes, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::FunctionCallExpression, sourceCodeLocationSpan), identifier(std::move(identifier)), argumentNodes(std::move(argumentNodes)) {};
    IdentifierNode* getIdentifier() const;
    const std::vector<ExpressionNode*> getArgumentNodes() const;
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
    FunctionCallStatementNode(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::FunctionCallStatement, sourceCodeLocationSpan), functionCallExpression(std::move(functionCallExpression)) {};
    FunctionCallExpressionNode* getFunctionCallExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<FunctionCallExpressionNode> takeFunctionCallExpression();
    void setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression);
private:
    std::unique_ptr<FunctionCallExpressionNode> functionCallExpression;
};

class StringLiteralNode : public PrimaryExpressionNode {
public:
    StringLiteralNode(std::unique_ptr<Token> stringLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::StringLiteral, sourceCodeLocationSpan), stringLiteralToken(std::move(stringLiteralToken)) {};
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
    BooleanLiteralNode(std::unique_ptr<Token> booleanLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::BooleanLiteral, sourceCodeLocationSpan), booleanLiteralToken(std::move(booleanLiteralToken)) {};
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
    EmptyLiteralNode(std::unique_ptr<Token> emptyLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::EmptyLiteral, sourceCodeLocationSpan), emptyLiteralToken(std::move(emptyLiteralToken)) {};
    Token* getEmptyLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
private:
    std::unique_ptr<Token> emptyLiteralToken;
};

class NumberLiteralNode : public PrimaryExpressionNode {
public:
    NumberLiteralNode(std::unique_ptr<Token> numberLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::NumberLiteral, sourceCodeLocationSpan), numberLiteralToken(std::move(numberLiteralToken)) {};
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
    BinaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, std::unique_ptr<Token> operatorToken, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::BinaryOperatorExpression, sourceCodeLocationSpan), left(std::move(left)), right(std::move(right)), operatorToken(std::move(operatorToken)) {};
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
    UnaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> operand, std::unique_ptr<Token> operatorToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::UnaryOperatorExpression, sourceCodeLocationSpan), operand(std::move(operand)), operatorToken(std::move(operatorToken)) {};
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
    IfExpressionNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<ExpressionNode> thenBranch, std::unique_ptr<ExpressionNode> elseBranch, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::IfExpression, sourceCodeLocationSpan), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {};
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
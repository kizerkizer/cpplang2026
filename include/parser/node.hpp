#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "common/programuniqueid.hpp"
#include "lexer/token.hpp"

/*
Rough Grammar:
Program ::= ExecutionList
ExecutionList ::= (Declaration | Statement)*
Declaration ::= VariableDeclaration | FunctionDeclaration
VariableDeclaration ::= 'var' Identifier (':' TypeExpression)? AssignmentExpression? ';'
FunctionDeclaration ::= 'function' Identifier '(' ParameterList? ')' (':' TypeExpression)? BlockStatement
TypeExpression = PrimaryTypeExpression | BinaryOperatorTypeExpression //PrimaryTypeExpression | BinaryOperatorTypeExpression
TypeDeclaration = 'type' Identifier '=' TypeExpression ';'
BinaryTypeOperator = '|'
BinaryOperatorTypeExpression = TypeExpression BinaryTypeOperator TypeExpression
PrimaryTypeExpression = Identifier | PrimitiveType | '(' TypeExpression ')'
PrimitiveType = 'Boolean' | 'Integer' | 'Float' | 'String' | 'Empty' | 'Number' | 'Any' | 'Void'
ParameterList ::= Identifier (',' Identifier)*
Statement ::= BlockStatement | IfStatement | WhileStatement | LoopStatement | BreakStatement | ContinueStatement | ReturnStatement | AssignmentStatement | FunctionCallStatement
BlockStatement ::= '{' ExecutionList '}'
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

enum class NodeKind {
    Program,
    ExecutionList,
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
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    BooleanLiteral,
    EmptyLiteral,
    //MapLiteralSyntax, // TODO later
    //ListLiteralSyntax, // TODO later
    //SetLiteralSyntax, // TODO later
    AssignmentExpression,
    FunctionCallExpression,
    BinaryOperatorExpression,
    UnaryOperatorExpression,
    IfExpression,
    TypePrimitive,
    TypeIdentifier,
    TypeDeclaration,
    BinaryOperatorTypeExpression,
    IdentifierWithPossibleAnnotation,
};

constexpr const char* nodeKindToString(NodeKind nodeKind) {
    switch (nodeKind) {
        using enum NodeKind;
        case AssignmentExpression:
            return "AssignmentExpression";
        case ExecutionList:
            return "ExecutionList";
        case FunctionCallExpression:
            return "FunctionCallExpression";
        case BinaryOperatorExpression:
            return "BinaryOperatorExpression";
        case UnaryOperatorExpression:
            return "UnaryOperatorExpression";
        case Program:
            return "Program";
        case Invalid:
            return "Invalid";
        case FunctionDeclaration:
            return "FunctionDeclaration";
        case VariableDeclaration:
            return "VariableDeclaration";
        case BlockStatement:
            return "BlockStatement";
        case IfStatement:
            return "IfStatement";
        case WhileStatement:
            return "WhileStatement";
        case LoopStatement:
            return "LoopStatement";
        case BreakStatement:
            return "BreakStatement";
        case ContinueStatement:
            return "ContinueStatement";
        case ReturnStatement:
            return "ReturnStatement";
        case AssignmentStatement:
            return "AssignmentStatement";
        case FunctionCallStatement:
            return "FunctionCallStatement";
        case Identifier:
            return "Identifier";
        case IntegerLiteral:
            return "IntegerLiteral";
        case FloatLiteral:
            return "FloatLiteral";
        case StringLiteral:
            return "StringLiteral";
        case BooleanLiteral:
            return "BooleanLiteral";
        case EmptyLiteral:
            return "EmptyLiteral";
        case IfExpression:
            return "IfExpression";
        case TypePrimitive:
            return "TypePrimitive";
        case TypeIdentifier:
            return "TypeIdentifier";
        case TypeDeclaration:
            return "TypeDeclaration";
        case BinaryOperatorTypeExpression:
            return "BinaryOperatorTypeExpression";
        case IdentifierWithPossibleAnnotation:
            return "IdentifierWithPossibleAnnotation";
    }
}

class Node {
private:
    int m_id;
    NodeKind m_nodeKind;
    SourceCodeLocationSpan m_sourceCodeLocationSpan;
    bool m_compilerCreated;
    std::vector<std::unique_ptr<Token>> m_tokens;
public:
    Node(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan, bool compilerCreated = false) : m_id(getNextId()), m_nodeKind(type), m_sourceCodeLocationSpan(sourceCodeLocationSpan), m_compilerCreated(compilerCreated) {};
    virtual ~Node() = default;
    int getId() const;
    NodeKind getNodeKind() const;
    bool operator==(const NodeKind& rhs) const;
    bool operator!=(const NodeKind& rhs) const;
    virtual const std::vector<Node*> getChildren() const = 0;
    bool isCompilerCreated() const;
    void addToken(std::unique_ptr<Token> token);
    std::vector<Token*> getTokens();
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
};

class InvalidNode : public Node {
public:
    InvalidNode() : Node(NodeKind::Invalid, SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1))) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class ExecutionListNode : public Node {
private:
    std::vector<std::unique_ptr<Node>> m_nodes;
public:
    ExecutionListNode(SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::ExecutionList, sourceCodeLocationSpan) {};
    void addNode(std::unique_ptr<Node> child);
    const std::vector<Node*> getChildren() const override;
    std::vector<std::unique_ptr<Node>> takeChildren();
    void setChildren(std::vector<std::unique_ptr<Node>> children);
};

class ProgramNode : public Node {
private:
    std::unique_ptr<ExecutionListNode> m_executionListNode;
public:
    ProgramNode(std::unique_ptr<ExecutionListNode> executionListNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::Program, sourceCodeLocationSpan), m_executionListNode(std::move(executionListNode)) {};
    ExecutionListNode* getExecutionListNode() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExecutionListNode> takeExecutionListNode();
    void setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode);
};

class ExpressionNode : public Node {
public:
    ExpressionNode(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(type, sourceCodeLocationSpan) {};
};

class PrimaryExpressionNode : public ExpressionNode {
public:
    PrimaryExpressionNode(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(type, sourceCodeLocationSpan) {};
};

// TODO this needed?
class TypeExpressionNode: public ExpressionNode {
public:
    TypeExpressionNode(NodeKind type, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(type, sourceCodeLocationSpan) {};
};

class IdentifierNode : public PrimaryExpressionNode {
private:
    std::unique_ptr<Token> m_identifierToken;
public:
    IdentifierNode(std::unique_ptr<Token> identifierToken, SourceCodeLocationSpan sourceCodeLocationSpan, NodeKind kind = NodeKind::Identifier) : PrimaryExpressionNode(kind, sourceCodeLocationSpan), m_identifierToken(std::move(identifierToken)) {};
    std::string_view getName() const;
    Token* getIdentifierToken() const;
    const std::vector<Node*> getChildren() const override;
};

class TypePrimitiveNode : public TypeExpressionNode {
private:
    std::unique_ptr<Token> m_token;
public:
    TypePrimitiveNode(std::unique_ptr<Token> token, SourceCodeLocationSpan sourceCodeLocationSpan) : TypeExpressionNode(NodeKind::TypePrimitive, sourceCodeLocationSpan), m_token(std::move(token)) {};
    const std::vector<Node*> getChildren() const override { return {}; }
    PrimitiveTypeKind getPrimitiveTypeKind() const;
};

class TypeIdentifierNode : public IdentifierNode {
private:
    std::unique_ptr<Token> m_token;
public:
    TypeIdentifierNode(std::unique_ptr<Token> token, SourceCodeLocationSpan sourceCodeLocationSpan) : IdentifierNode(std::move(token), sourceCodeLocationSpan, NodeKind::TypeIdentifier), m_token(std::move(token)) {};
    /*std::string_view getName() const;
    Token* getIdentifierToken() const;
    const std::vector<Node*> getChildren() const override { return {}; }*/
};

class TypeDeclarationNode : public Node {
private:
    std::unique_ptr<IdentifierNode> m_identifierNode;
    std::unique_ptr<Node> m_typeExpressionNode;
public:
    TypeDeclarationNode(std::unique_ptr<IdentifierNode> identifierNode, std::unique_ptr<Node> typeExpressionNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::TypeDeclaration, sourceCodeLocationSpan), m_identifierNode(std::move(identifierNode)), m_typeExpressionNode(std::move(typeExpressionNode)) {};
    IdentifierNode* getIdentifier() const;
    Node* getTypeExpression() const;
    const std::vector<Node*> getChildren() const override;
};

class BinaryOperatorTypeExpressionNode : public TypeExpressionNode {
private:
    std::unique_ptr<Node> m_left;
    std::unique_ptr<Node> m_right;
    std::unique_ptr<Token> m_operatorToken;
public:
    BinaryOperatorTypeExpressionNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right, std::unique_ptr<Token> operatorToken, SourceCodeLocationSpan sourceCodeLocationSpan) : TypeExpressionNode(NodeKind::BinaryOperatorTypeExpression, sourceCodeLocationSpan), m_left(std::move(left)), m_right(std::move(right)), m_operatorToken(std::move(operatorToken)) {};
    const std::vector<Node*> getChildren() const override;
    Node* getLeft() const;
    Node* getRight() const;
    Token* getOperatorToken() const;
    TokenKind getOperatorTokenKind() const;
    std::unique_ptr<Node> takeLeft();
    std::unique_ptr<Node> takeRight();
    void setLeft(std::unique_ptr<Node> left);
    void setRight(std::unique_ptr<Node> right);
    void setOperatorToken(std::unique_ptr<Token> operatorToken);
};

class AssignmentExpressionNode : public ExpressionNode {
private:
    std::unique_ptr<IdentifierNode> m_identifier;
    std::unique_ptr<ExpressionNode> m_expression;
public:
    AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::AssignmentExpression, sourceCodeLocationSpan), m_identifier(std::move(identifier)), m_expression(std::move(expression)) {};
    std::string_view getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    ExpressionNode* getExpression() const;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<ExpressionNode> takeExpression();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setExpression(std::unique_ptr<ExpressionNode> expression);
    const std::vector<Node*> getChildren() const override;
};

class VariableDeclarationNode : public Node {
private:
    std::unique_ptr<IdentifierNode> m_identifierNode;
    std::unique_ptr<Node> m_typeAnnotation;
    std::unique_ptr<ExpressionNode> m_expressionNode;
public:
    VariableDeclarationNode(std::unique_ptr<IdentifierNode> identifierNode, std::unique_ptr<Node> typeAnnotation, std::unique_ptr<ExpressionNode> expressionNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::VariableDeclaration, sourceCodeLocationSpan), m_identifierNode(std::move(identifierNode)), m_typeAnnotation(std::move(typeAnnotation)), m_expressionNode(std::move(expressionNode)) {};
    IdentifierNode* getIdentifier() const;
    Node* getTypeAnnotation() const;
    ExpressionNode* getExpression() const;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<Node> takeTypeAnnotation();
    std::unique_ptr<ExpressionNode> takeExpression();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifierNode);
    void setTypeAnnotation(std::unique_ptr<Node> typeAnnotation);
    void setExpression(std::unique_ptr<ExpressionNode> expressionNode);
    const std::vector<Node*> getChildren() const override;
};

class BlockStatementNode : public Node {
private:
    std::unique_ptr<ExecutionListNode> m_executionListNode;
public:
    BlockStatementNode(std::unique_ptr<ExecutionListNode> executionListNode, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::BlockStatement, sourceCodeLocationSpan), m_executionListNode(std::move(executionListNode)) {};
    ExecutionListNode* getExecutionListNode() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExecutionListNode> takeExecutionListNode();
    void setExecutionListNode(std::unique_ptr<ExecutionListNode> executionListNode);
};

class IdentifierWithPossibleAnnotationNode : public IdentifierNode {
private:
    std::unique_ptr<Node> m_annotation;
public:
    IdentifierWithPossibleAnnotationNode(std::unique_ptr<Token> token, std::unique_ptr<Node> annotation, SourceCodeLocationSpan sourceCodeLocationSpan) : IdentifierNode(std::move(token), sourceCodeLocationSpan, NodeKind::IdentifierWithPossibleAnnotation), m_annotation(std::move(annotation)) {};
    Node* getAnnotation() const;
    std::unique_ptr<Node> takeAnnotation();
    void setAnnotation(std::unique_ptr<Node> annotation);
};

class FunctionDeclarationNode : public Node {
private:
    std::unique_ptr<IdentifierNode> m_identifier;
    std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> m_parameters;
    std::unique_ptr<Node> m_returnTypeAnnotation;
    std::unique_ptr<BlockStatementNode> m_bodyNode;
public:
    FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode, std::unique_ptr<Node> returnTypeAnnotation, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::FunctionDeclaration, sourceCodeLocationSpan), m_identifier(std::move(identifier)), m_parameters(std::move(parameters)), m_returnTypeAnnotation(std::move(returnTypeAnnotation)), m_bodyNode(std::move(bodyNode)) {};
    std::string getIdentifierName() const;
    IdentifierNode* getIdentifier() const;
    Node* getReturnTypeAnnotation() const;
    const std::vector<IdentifierWithPossibleAnnotationNode*> getParameters() const;
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<Node> takeReturnTypeAnnotation();
    std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> takeParameters();
    std::unique_ptr<BlockStatementNode> takeBodyNode();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setReturnTypeAnnotation(std::unique_ptr<Node> returnTypeAnnotation);
    void setParameters(std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters);
    void setBodyNode(std::unique_ptr<BlockStatementNode> bodyNode);
};

class IfStatementNode : public Node {
private:
    std::unique_ptr<ExpressionNode> m_condition;
    std::unique_ptr<Node> m_thenBranch;
    std::unique_ptr<Node> m_elseBranch;
public:
    IfStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::IfStatement, sourceCodeLocationSpan), m_condition(std::move(condition)), m_thenBranch(std::move(thenBranch)), m_elseBranch(std::move(elseBranch)) {};
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
};

class LoopStatementNode : public Node {
private:
    std::unique_ptr<BlockStatementNode> m_body;
public:
    LoopStatementNode(std::unique_ptr<BlockStatementNode> body, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::LoopStatement, sourceCodeLocationSpan), m_body(std::move(body)) {};
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<BlockStatementNode> takeBody();
    void setBody(std::unique_ptr<BlockStatementNode> body);
};

class WhileStatementNode : public Node {
private:
    std::unique_ptr<ExpressionNode> m_condition;
    std::unique_ptr<BlockStatementNode> m_body;
public:
    WhileStatementNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<BlockStatementNode> body, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::WhileStatement, sourceCodeLocationSpan), m_condition(std::move(condition)), m_body(std::move(body)) {};
    ExpressionNode* getCondition() const;
    BlockStatementNode* getBody() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeCondition();
    std::unique_ptr<BlockStatementNode> takeBody();
    void setCondition(std::unique_ptr<ExpressionNode> condition);
    void setBody(std::unique_ptr<BlockStatementNode> body);
};

class BreakStatementNode : public Node {
public:
    BreakStatementNode(SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::BreakStatement, sourceCodeLocationSpan) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class ContinueStatementNode : public Node {
public:
    ContinueStatementNode(SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::ContinueStatement, sourceCodeLocationSpan) {};
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class ReturnStatementNode : public Node {
private:
    std::unique_ptr<ExpressionNode> m_expression;
public:
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::ReturnStatement, sourceCodeLocationSpan), m_expression(std::move(expression)) {};
    ExpressionNode* getExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeExpression();
    void setExpression(std::unique_ptr<ExpressionNode> expression);
};

class AssignmentStatementNode : public Node {
private:
    std::unique_ptr<IdentifierNode> m_identifier;
    std::unique_ptr<AssignmentExpressionNode> m_assignmentExpression;
public:
    AssignmentStatementNode(std::unique_ptr<AssignmentExpressionNode> assignmentExpression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::AssignmentStatement, sourceCodeLocationSpan), m_assignmentExpression(std::move(assignmentExpression)) {};
    IdentifierNode* getIdentifier() const;
    AssignmentExpressionNode* getAssignmentExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::unique_ptr<AssignmentExpressionNode> takeAssignmentExpression();
    void setAssignmentExpression(std::unique_ptr<AssignmentExpressionNode> assignmentExpression);
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
};

class FunctionCallExpressionNode : public PrimaryExpressionNode {
private:
    std::unique_ptr<IdentifierNode> m_identifier;
    std::vector<std::unique_ptr<ExpressionNode>> m_argumentNodes;
public:
    FunctionCallExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<ExpressionNode>> argumentNodes, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::FunctionCallExpression, sourceCodeLocationSpan), m_identifier(std::move(identifier)), m_argumentNodes(std::move(argumentNodes)) {};
    IdentifierNode* getIdentifier() const;
    const std::vector<ExpressionNode*> getArgumentNodes() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<IdentifierNode> takeIdentifier();
    std::vector<std::unique_ptr<ExpressionNode>> takeArgumentNodes();
    void setIdentifier(std::unique_ptr<IdentifierNode> identifier);
    void setArgumentNodes(std::vector<std::unique_ptr<ExpressionNode>> argumentNodes);
};

class FunctionCallStatementNode : public Node {
private:
    std::unique_ptr<FunctionCallExpressionNode> m_functionCallExpression;
public:
    FunctionCallStatementNode(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression, SourceCodeLocationSpan sourceCodeLocationSpan) : Node(NodeKind::FunctionCallStatement, sourceCodeLocationSpan), m_functionCallExpression(std::move(functionCallExpression)) {};
    FunctionCallExpressionNode* getFunctionCallExpression() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<FunctionCallExpressionNode> takeFunctionCallExpression();
    void setFunctionCallExpression(std::unique_ptr<FunctionCallExpressionNode> functionCallExpression);
};

class StringLiteralNode : public PrimaryExpressionNode {
private:
    std::unique_ptr<Token> m_stringLiteralToken;
public:
    StringLiteralNode(std::unique_ptr<Token> stringLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::StringLiteral, sourceCodeLocationSpan), m_stringLiteralToken(std::move(stringLiteralToken)) {};
    std::string getValue() const;
    Token* getStringLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class BooleanLiteralNode : public PrimaryExpressionNode {
private:
    bool m_value;
    std::unique_ptr<Token> m_booleanLiteralToken;
public:
    BooleanLiteralNode(std::unique_ptr<Token> booleanLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::BooleanLiteral, sourceCodeLocationSpan), m_value(std::string(booleanLiteralToken->getSourceString()) == "true"), m_booleanLiteralToken(std::move(booleanLiteralToken)) {};
    bool getValue() const;
    Token* getBooleanLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class EmptyLiteralNode : public PrimaryExpressionNode {
private:
    std::unique_ptr<Token> m_emptyLiteralToken;
public:
    EmptyLiteralNode(std::unique_ptr<Token> emptyLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::EmptyLiteral, sourceCodeLocationSpan), m_emptyLiteralToken(std::move(emptyLiteralToken)) {};
    Token* getEmptyLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class IntegerLiteralNode : public PrimaryExpressionNode {
private:
    long long m_value;
    std::unique_ptr<Token> m_integerLiteralToken;
public:
    IntegerLiteralNode(std::unique_ptr<Token> integerLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::IntegerLiteral, sourceCodeLocationSpan), m_value(std::stoll(std::string(integerLiteralToken->getSourceString()))), m_integerLiteralToken(std::move(integerLiteralToken)) {};
    long long getValue() const;
    Token* getIntegerLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class FloatLiteralNode : public PrimaryExpressionNode {
private:
    double m_value;
    std::unique_ptr<Token> m_floatLiteralToken;
public:
    FloatLiteralNode(std::unique_ptr<Token> floatLiteralToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::FloatLiteral, sourceCodeLocationSpan), m_value(std::stod(std::string(floatLiteralToken->getSourceString()))), m_floatLiteralToken(std::move(floatLiteralToken)) {};
    double getValue() const;
    Token* getFloatLiteralToken() const;
    const std::vector<Node*> getChildren() const override {
        return {};
    }
};

class BinaryOperatorExpressionNode : public ExpressionNode {
private:
    std::unique_ptr<ExpressionNode> m_left;
    std::unique_ptr<ExpressionNode> m_right;
    std::unique_ptr<Token> m_operatorToken;
public:
    BinaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right, std::unique_ptr<Token> operatorToken, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::BinaryOperatorExpression, sourceCodeLocationSpan), m_left(std::move(left)), m_right(std::move(right)), m_operatorToken(std::move(operatorToken)) {};
    ExpressionNode* getLeft() const;
    ExpressionNode* getRight() const;
    Token* getOperatorToken() const;
    TokenKind getOperatorTokenKind() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeLeft();
    std::unique_ptr<ExpressionNode> takeRight();
    void setLeft(std::unique_ptr<ExpressionNode> left);
    void setRight(std::unique_ptr<ExpressionNode> right);
};

class UnaryOperatorExpressionNode : public PrimaryExpressionNode {
private:
    std::unique_ptr<ExpressionNode> m_operand;
    std::unique_ptr<Token> m_operatorToken;
public:
    UnaryOperatorExpressionNode(std::unique_ptr<ExpressionNode> operand, std::unique_ptr<Token> operatorToken, SourceCodeLocationSpan sourceCodeLocationSpan) : PrimaryExpressionNode(NodeKind::UnaryOperatorExpression, sourceCodeLocationSpan), m_operand(std::move(operand)), m_operatorToken(std::move(operatorToken)) {};
    ExpressionNode* getOperand() const;
    Token* getOperatorToken() const;
    TokenKind getOperatorTokenKind() const;
    const std::vector<Node*> getChildren() const override;
    std::unique_ptr<ExpressionNode> takeOperand();
    void setOperand(std::unique_ptr<ExpressionNode> operand);
};

class IfExpressionNode : public ExpressionNode {
private:
    std::unique_ptr<ExpressionNode> m_condition;
    std::unique_ptr<ExpressionNode> m_thenBranch;
    std::unique_ptr<ExpressionNode> m_elseBranch;
public:
    IfExpressionNode(std::unique_ptr<ExpressionNode> condition, std::unique_ptr<ExpressionNode> thenBranch, std::unique_ptr<ExpressionNode> elseBranch, SourceCodeLocationSpan sourceCodeLocationSpan) : ExpressionNode(NodeKind::IfExpression, sourceCodeLocationSpan), m_condition(std::move(condition)), m_thenBranch(std::move(thenBranch)), m_elseBranch(std::move(elseBranch)) {};
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
};
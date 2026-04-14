#pragma once

#include "binder/binder.hpp"
#include "checker/checker.hpp"
#include "flow/flowbuilder.hpp"
#include "parser/node.hpp"
#include "treewalker/environment.hpp"
#include "treewalker/outputstream.hpp"
#include "treewalker/valuestore.hpp"

#define PERFORM_INTEGER_OP(tokenKind, leftValue, rightValue) \
    (tokenKind == TokenKind::Plus) ? (leftValue + rightValue) : \
    (tokenKind == TokenKind::Dash) ? (leftValue - rightValue) : \
    (tokenKind == TokenKind::Asterisk) ? (leftValue * rightValue) : \
    (tokenKind == TokenKind::Slash) ? (leftValue / rightValue) : \
    (tokenKind == TokenKind::AsteriskAsterisk) ? (static_cast<int>(std::pow(leftValue, rightValue))) : \
    (tokenKind == TokenKind::EqualEqual) ? (leftValue == rightValue) : \
    (tokenKind == TokenKind::NotEqual) ? (leftValue != rightValue) : \
    (tokenKind == TokenKind::LessThan) ? (leftValue < rightValue) : \
    (tokenKind == TokenKind::LessThanEqual) ? (leftValue <= rightValue) : \
    (tokenKind == TokenKind::GreaterThan) ? (leftValue > rightValue) : \
    (tokenKind == TokenKind::GreaterThanEqual) ? (leftValue >= rightValue) : \
    (leftValue)
    
#define PERFORM_RELATIONAL_OP(tokenKind, leftValue, rightValue) \
    (tokenKind == TokenKind::EqualEqual) ? (leftValue == rightValue) : \
    (tokenKind == TokenKind::NotEqual) ? (leftValue != rightValue) : \
    (tokenKind == TokenKind::LessThan) ? (leftValue < rightValue) : \
    (tokenKind == TokenKind::LessThanEqual) ? (leftValue <= rightValue) : \
    (tokenKind == TokenKind::GreaterThan) ? (leftValue > rightValue) : \
    (tokenKind == TokenKind::GreaterThanEqual) ? (leftValue >= rightValue) : \
    (false)

#define PERFORM_EQUALITY_OP(tokenKind, leftValue, rightValue) \
    (tokenKind == TokenKind::EqualEqual) ? (leftValue == rightValue) : \
    (tokenKind == TokenKind::NotEqual) ? (leftValue != rightValue) : \
    (false)

#define PERFORM_LOGICAL_OP(tokenKind, leftValue, rightValue) \
    (tokenKind == TokenKind::AmpersandAmpersand) ? (leftValue && rightValue) : \
    (tokenKind == TokenKind::PipePipe) ? (leftValue || rightValue) : \
    (false)

class Interpreter {
private:
    std::unique_ptr<Environment> m_globalEnvironment;
    TypeCheckerResult* m_typeCheckerResult;
    BinderResult* m_binderResult;
    FlowAnalyzerResult* m_flowBuilderResult;
    std::unique_ptr<ValueStore> m_valueStore;
    OutputStream* m_outputStream;
    Value* _interpret(Node* node, Environment* environment);
    Value* interpretIdentifier(IdentifierNode* identifierNode, Environment* environment);
    Type* getTypeOfASTNode(Node* node);
public:
    Interpreter(TypeCheckerResult* typeCheckerResult, BinderResult* binderResult, FlowAnalyzerResult* flowBuilderResult, OutputStream* outputStream);
    Value* interpret(Node* rootNode);
};
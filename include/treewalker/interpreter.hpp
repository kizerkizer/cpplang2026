#pragma once

#include "parser/node.hpp"
#include "treewalker/environment.hpp"
#include "treewalker/outputstream.hpp"

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
    (tokenKind == TokenKind::And) ? (leftValue && rightValue) : \
    (tokenKind == TokenKind::Or) ? (leftValue || rightValue) : \
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
    (tokenKind == TokenKind::And) ? (leftValue && rightValue) : \
    (tokenKind == TokenKind::Or) ? (leftValue || rightValue) : \
    (false)

/*class InterpreterContext {
private:

public:
};*/

class Interpreter {
public:
    Interpreter(OutputStream* outputStream);
    Value* interpret(Node* rootNode);
private:
    std::unique_ptr<Environment> globalEnvironment;
    std::unique_ptr<ValueStore> valueStore;
    OutputStream* outputStream;
    Value* _interpret(Node* node, Environment* environment);
    Value* interpretIdentifier(IdentifierNode* identifierNode, Environment* environment);
};
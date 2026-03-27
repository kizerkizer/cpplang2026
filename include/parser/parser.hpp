#pragma once

#include <vector>
#include <deque>

#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"

class Parser {
public:
    Parser(Lexer* lexer, std::vector<std::string>& errorMessages_out) : lexer(lexer), errorMessages(errorMessages_out) {};
    std::unique_ptr<Node> parse();
private:
    Lexer* lexer;
    size_t index = 0;
    std::deque<std::unique_ptr<Token>> tokenBuffer;
    std::vector<std::string> &errorMessages;

    /**
     * @brief Expects the next token to have the specified token name, and advances the index. If the next token does not have the expected token name, an error message is added to errorMessages_out and std::nullopt is returned.
     * 
     * @param expectedTokenName Name of the expected token
     * @return std::optional<Token> 
     */
    std::unique_ptr<Token> expectAndAdvance(const TokenKind& expectedTokenName);
    Token peek(size_t offset = 0);
    std::unique_ptr<Token> consumeCurrentToken();

    /**
     * @brief Matches the next token with the specified token name and advances the index. If the next token does not have the expected token name, nothing happens.
     * 
     * @param expectedTokenName Name of the expected token
     * @return std::optional<Token> 
     */
    std::unique_ptr<Token> matchAndAdvance(const TokenKind& expectedTokenName);
    bool isPastTokensEnd() const;
    int insideLoop = 0;
    int insideFunction = 0;
    int insideBlock = 0;
    void enterLoop();
    void exitLoop();
    void enterFunction();
    void exitFunction();
    void enterBlock();
    void exitBlock();
    void addErrorMessageParseFailure(const std::string& failedToParse);
    void addErrorMessageExpected(const std::string& expected);
    void addErrorMessageUnexpected(const std::string& unexpected);
    std::unique_ptr<ProgramNode> parseProgram();
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<WhileStatementNode> parseWhileStatement();
    std::unique_ptr<LoopStatementNode> parseLoopStatement();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<BlockStatementNode> parseBlockStatement();
    std::unique_ptr<BreakStatementNode> parseBreakStatement();
    std::unique_ptr<ContinueStatementNode> parseContinueStatement();
    std::unique_ptr<FunctionCallStatementNode> parseFunctionCallStatement();
    std::unique_ptr<AssignmentStatementNode> parseAssignmentStatement();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parsePrimaryExpression();
    std::unique_ptr<TypeExpressionNode> parseTypeExpression();
    std::unique_ptr<IdentifierWithPossibleAnnotationNode> parseIdentifierWithPossibleAnnotation();
    std::unique_ptr<AssignmentExpressionNode> parseAssignmentExpression();
    std::unique_ptr<FunctionCallExpressionNode> parseFunctionCallExpression();
    std::unique_ptr<IfExpressionNode> parseIfExpression();
    std::unique_ptr<ExpressionNode> parseExpressionClimbing(std::unique_ptr<ExpressionNode> lhs, int minPrecedence);
};
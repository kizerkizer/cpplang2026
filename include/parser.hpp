#pragma once

#include <optional>
#include <vector>

#include "token.hpp"
#include "node.hpp"

class Parser {
public:
    Parser(const std::vector<Token>& tokens, std::vector<std::string>& errorMessages_out) : tokens(tokens), errorMessages(errorMessages_out) {};
    std::unique_ptr<Node> parse ();
private:
    size_t index = 0;
    const std::vector<Token> &tokens;
    std::vector<std::string> &errorMessages;

    /**
     * @brief Expects the next token to have the specified token name, and advances the index. If the next token does not have the expected token name, an error message is added to errorMessages_out and std::nullopt is returned.
     * 
     * @param expectedTokenName Name of the expected token
     * @return std::optional<Token> 
     */
    std::optional<Token> expectAndAdvance(const TokenName& expectedTokenName);
    Token peek(int offset = 0) const;

    /**
     * @brief Matches the next token with the specified token name and advances the index. If the next token does not have the expected token name, nothing happens.
     * 
     * @param expectedTokenName Name of the expected token
     * @return std::optional<Token> 
     */
    std::optional<Token> matchAndAdvance(const TokenName& expectedTokenName);
    bool isPastTokensEnd() const;
    void skipTrivia();
    std::unique_ptr<ProgramNode> parseProgram(bool insideBlock = false, bool insideWhile = false); // TODO add insideFunction
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<WhileStatementNode> parseWhileStatement();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<BlockStatementNode> parseBlockStatement(bool insideWhile = false);
    std::unique_ptr<BreakStatementNode> parseBreakStatement();
    std::unique_ptr<ContinueStatementNode> parseContinueStatement();
    std::unique_ptr<FunctionCallStatementNode> parseFunctionCallStatement();
    std::unique_ptr<AssignmentStatementNode> parseAssignmentStatement();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<AssignmentExpressionNode> parseAssignmentExpression();
    std::unique_ptr<FunctionCallExpressionNode> parseFunctionCallExpression();
};
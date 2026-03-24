#include "node.hpp"
#include <memory>
#include <vector>

// Node
NodeType Node::getNodeType() const {
    return this->nodeType;
}

bool Node::operator==(const NodeType& rhs) const {
    return this->nodeType == rhs;
}

bool Node::operator!=(const NodeType& rhs) const {
    return !(*this == rhs);
}

// ProgramNode
void ProgramNode::addNode(std::unique_ptr<Node> child) {
    this->nodes.push_back(std::move(child));
}

const std::vector<const Node*> ProgramNode::getChildren() const {
    std::vector<const Node*> children;
    for (const auto& child : this->nodes) {
        children.push_back(child.get());
    }
    return children;
}

//VariableDeclarationNode
VariableDeclarationNode::VariableDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<AssignmentExpressionNode> assignmentExpression) : Node(NodeType::VariableDeclaration), identifier(std::move(identifier)), assignmentExpression(std::move(assignmentExpression)) { };

IdentifierNode* VariableDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

std::string VariableDeclarationNode::getIdentifierName() const {
    return this->getIdentifier()->getName();
}

AssignmentExpressionNode* VariableDeclarationNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

const std::vector<const Node*> VariableDeclarationNode::getChildren() const {
    return { this->identifier.get(), this->assignmentExpression.get() };
}

//FunctionDeclarationNode
FunctionDeclarationNode::FunctionDeclarationNode(std::unique_ptr<IdentifierNode> identifier, std::vector<std::unique_ptr<IdentifierNode>> parameters, std::unique_ptr<BlockStatementNode> bodyNode) : Node(NodeType::FunctionDeclaration), identifier(std::move(identifier)), parameters(std::move(parameters)), bodyNode(std::move(bodyNode)) {};

IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

std::string FunctionDeclarationNode::getIdentifierName() const {
    return this->getIdentifier()->getName();
}

const std::vector<const IdentifierNode*> FunctionDeclarationNode::getParameters() const {
    std::vector<const IdentifierNode*> parameterPointers;
    for (const auto& parameter : this->parameters) {
        parameterPointers.push_back(parameter.get());
    }
    return parameterPointers;
}

BlockStatementNode* FunctionDeclarationNode::getBody() const {
    return this->bodyNode.get();
};

const std::vector<const Node*> FunctionDeclarationNode::getChildren() const {
    std::vector<const Node*> children;
    children.push_back(this->identifier.get());
    for (const auto& parameter : this->parameters) {
        children.push_back(parameter.get());
    }
    children.push_back(this->bodyNode.get());
    return children;
}

//BlockStatementNode
BlockStatementNode::BlockStatementNode(std::unique_ptr<ProgramNode> programNode) : Node(NodeType::BlockStatement), programNode(std::move(programNode)) {};

ProgramNode* BlockStatementNode::getProgramNode() const {
    return this->programNode.get();
}

const std::vector<const Node*> BlockStatementNode::getChildren() const {
    return { this->programNode.get() };
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

const std::vector<const Node*> IfStatementNode::getChildren() const {
    std::vector<const Node*> children;
    children.push_back(this->condition.get());
    if (this->thenBranch) {
        children.push_back(this->thenBranch.get());
    }
    if (this->elseBranch) {
        children.push_back(this->elseBranch.get());
    }
    return children;
}

//WhileStatementNode
ExpressionNode* WhileStatementNode::getCondition() const {
    return this->condition.get();
}

BlockStatementNode* WhileStatementNode::getBody() const {
    return this->body.get();
}

const std::vector<const Node*> WhileStatementNode::getChildren() const {
    return { this->condition.get(), this->body.get() };
}

//ReturnStatementNode
ExpressionNode* ReturnStatementNode::getExpression() const {
    return this->expression.get();
}

const std::vector<const Node*> ReturnStatementNode::getChildren() const {
    if (this->expression) {
        return { this->expression.get() };
    }
    return {};
}

//AssignmentExpressionNode
AssignmentExpressionNode::AssignmentExpressionNode(std::unique_ptr<IdentifierNode> identifier, std::unique_ptr<ExpressionNode> expression) : ExpressionNode(NodeType::AssignmentExpression), identifier(std::move(identifier)), expression(std::move(expression)) {};

IdentifierNode* AssignmentExpressionNode::getIdentifier() const {
    return this->identifier.get();
}

ExpressionNode* AssignmentExpressionNode::getExpression() const {
    return this->expression.get();
}

const std::vector<const Node*> AssignmentExpressionNode::getChildren() const {
    return { this->identifier.get(), this->expression.get() };
}

//AssignmentStatementNode
IdentifierNode* AssignmentStatementNode::getIdentifier() const {
    return this->assignmentExpression->getIdentifier();
}

AssignmentExpressionNode* AssignmentStatementNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

const std::vector<const Node*> AssignmentStatementNode::getChildren() const {
    return { this->assignmentExpression.get() };
}

//IdentifierNode
std::string IdentifierNode::getName() const {
    return this->identifierToken->getSourceString();
}

const std::vector<const Node*> IdentifierNode::getChildren() const {
    return {};
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

const std::vector<const Node*> FunctionCallExpressionNode::getChildren() const {
    std::vector<const Node*> children;
    children.push_back(this->identifier.get());
    for (const auto& argument : this->argumentNodes) {
        children.push_back(argument.get());
    }
    return children;
}

//FunctionCallStatementNode
FunctionCallExpressionNode* FunctionCallStatementNode::getFunctionCallExpression() const {
    return this->functionCallExpression.get();
}

const std::vector<const Node*> FunctionCallStatementNode::getChildren() const {
    return { this->functionCallExpression.get() };
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

const std::vector<const Node*> BinaryOperatorExpressionNode::getChildren() const {
    return { this->left.get(), this->right.get() };
}

//UnaryOperatorExpressionNode
PrimaryExpressionNode* UnaryOperatorExpressionNode::getOperand() const {
    return this->operand.get();
}

Token* UnaryOperatorExpressionNode::getOperatorToken() const {
    return this->operatorToken.get();
}

const std::vector<const Node*> UnaryOperatorExpressionNode::getChildren() const {
    return { this->operand.get() };
}
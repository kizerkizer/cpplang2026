#include "node.hpp"
#include <memory>

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
const std::vector<std::unique_ptr<Node>>& ProgramNode::getChildren() const {
    return this->children;
}

void ProgramNode::addChild(std::unique_ptr<Node> child) {
    this->children.push_back(std::move(child));
}

//VariableDeclarationNode
IdentifierNode* VariableDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

std::string VariableDeclarationNode::getIdentifierName() const {
    return this->getIdentifier()->getName();
}

AssignmentExpressionNode* VariableDeclarationNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

//FunctionDeclarationNode
IdentifierNode* FunctionDeclarationNode::getIdentifier() const {
    return this->identifier.get();
};

std::string FunctionDeclarationNode::getIdentifierName() const {
    return this->getIdentifier()->getName();
}

const std::vector<std::unique_ptr<IdentifierNode>>& FunctionDeclarationNode::getParameters() const {
    return this->parameters;
}

BlockStatementNode* FunctionDeclarationNode::getBody() const {
    return this->bodyNode.get();
};


//BlockStatementNode
ProgramNode* BlockStatementNode::getProgramNode() const {
    return this->programNode.get();
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

//WhileStatementNode
ExpressionNode* WhileStatementNode::getCondition() const {
    return this->condition.get();
}

BlockStatementNode* WhileStatementNode::getBody() const {
    return this->body.get();
}

//ReturnStatementNode
ExpressionNode* ReturnStatementNode::getExpression() const {
    return this->expression.get();
}

//AssignmentExpressionNode
IdentifierNode* AssignmentExpressionNode::getIdentifier() const {
    return this->identifier.get();
}

ExpressionNode* AssignmentExpressionNode::getExpression() const {
    return this->expression.get();
}

//AssignmentStatementNode
IdentifierNode* AssignmentStatementNode::getIdentifier() const {
    return this->assignmentExpression->getIdentifier();
}

AssignmentExpressionNode* AssignmentStatementNode::getAssignmentExpression() const {
    return this->assignmentExpression.get();
}

//IdentifierNode
std::string IdentifierNode::getName() const {
    return this->identifierToken->getSourceString();
}

//FunctionCallExpressionNode
IdentifierNode* FunctionCallExpressionNode::getIdentifier() const {
    return this->identifier.get();
}


const std::vector<std::unique_ptr<ExpressionNode>>& FunctionCallExpressionNode::getArgumentNodes() const {
    return this->argumentNodes;
}

//FunctionCallStatementNode
FunctionCallExpressionNode* FunctionCallStatementNode::getFunctionCallExpression() const {
    return this->functionCallExpression.get();
}

//NumberLiteralNode
int NumberLiteralNode::getValue() const {
    return std::stoi(this->numberLiteralToken->getSourceString());
}

//StringLiteralNode
std::string StringLiteralNode::getValue() const {
    return this->stringLiteralToken->getSourceString();
}


#include <memory>
#include <print>

#include "binder/binder.hpp"
#include "binder/scope.hpp"
#include "binder/name.hpp"
#include "parser/node.hpp"

Node* BinderResult::getNode() const {
    return this->node.get();
}

Scope* BinderResult::getRootScope() const {
    return this->rootScope;
}

Binder::Binder(std::vector<std::string>& errorMessages_out) : errorMessages(errorMessages_out) {
    this->rootScope = new Scope(ScopeKind::Root, nullptr, nullptr);
    this->currentScope = this->rootScope;
}

void Binder::addErrorMessage(const std::string& message) {
    this->errorMessages.push_back(message);
}

Scope* Binder::createAndEnterScope(ScopeKind kind) {
    Scope* newScope = new Scope(kind, this->currentScope, nullptr);
    this->currentScope->addChild(newScope);
    this->currentScope = newScope;
    return newScope;
}

void Binder::exitScope() {
    if (this->currentScope != this->rootScope) {
        this->currentScope = this->currentScope->getParent();
    }
}

// TODO at some point handle "imports" of namespaces/modules/packages (whatever we end up calling them) and the fact that they create a new scope and can contain names that are accessible from the current scope
// TODO handle type definitions and corresponding scopes
// TODO handle 'this'/'super'/other contextual keywords

std::unique_ptr<BinderResult> Binder::bind(std::unique_ptr<Node> rootNode) {
    this->currentScope->setNode(rootNode.get());
    this->bindRecursive(rootNode.get());
    return std::make_unique<BinderResult>(std::move(rootNode), this->rootScope);
}

void Binder::bindRecursive(Node* node) {
    switch (node->getNodeKind()) {
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            for (auto& child : programNode->getChildren()) {
                this->bindRecursive(child);
            }
            break;
        }
        case NodeKind::VariableDeclaration: {
            auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
            auto assignmentExpression = variableDeclarationNode->getAssignmentExpression();
            if (assignmentExpression) {
                auto identifierNode = assignmentExpression->getIdentifier();
                if (identifierNode) {
                    auto name = new Name(this->currentScope, variableDeclarationNode, NameKind::Variable, NameModifierFlags::None, identifierNode->getName());
                    if (this->currentScope->hasName(name->getNameString())) {
                        this->addErrorMessage("Name '" + name->getNameString() + "' already in scope");
                    } else {
                        this->currentScope->setName(name);
                    }
                }
                this->bindRecursive(assignmentExpression);
            }
            break;
        }
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            auto identifierNode = functionDeclarationNode->getIdentifier();
            if (identifierNode) {
                auto name = new Name(this->currentScope, functionDeclarationNode, NameKind::Function, NameModifierFlags::None, identifierNode->getName());
                if (this->currentScope->hasName(name->getNameString())) {
                    this->addErrorMessage("Name '" + name->getNameString() + "' already in scope");
                } else {
                    this->currentScope->setName(name);
                }
            }
            this->createAndEnterScope(ScopeKind::Function);
            this->currentScope->setNode(functionDeclarationNode);
            for (auto parameter : functionDeclarationNode->getParameters()) {
                auto parameterIdentifierNode = parameter;
                auto parameterName = new Name(this->currentScope, parameterIdentifierNode, NameKind::Parameter, NameModifierFlags::None, parameterIdentifierNode->getName());
                if (this->currentScope->hasName(parameterName->getNameString())) {
                    this->addErrorMessage("Name '" + parameterName->getNameString() + "' already in scope");
                } else {
                    this->currentScope->setName(parameterName);
                }
            }
            this->bindRecursive(functionDeclarationNode->getBody());
            this->exitScope();
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            this->createAndEnterScope(ScopeKind::Block);
            this->currentScope->setNode(blockStatementNode);
            this->bindRecursive(blockStatementNode->getProgramNode());
            this->exitScope();
            break;
        }
        case NodeKind::IfStatement: {
            auto ifStatementNode = static_cast<IfStatementNode*>(node);
            this->bindRecursive(ifStatementNode->getCondition());
            this->createAndEnterScope(ScopeKind::Block);
            this->currentScope->setNode(ifStatementNode->getThenBranch());
            this->bindRecursive(ifStatementNode->getThenBranch());
            this->exitScope();
            if (ifStatementNode->getElseBranch()) {
                this->createAndEnterScope(ScopeKind::Block);
                this->currentScope->setNode(ifStatementNode->getElseBranch());
                this->bindRecursive(ifStatementNode->getElseBranch());
                this->exitScope();
            }
            break;
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            this->createAndEnterScope(ScopeKind::Block);
            this->currentScope->setNode(loopStatementNode->getBody());
            this->bindRecursive(loopStatementNode->getBody());
            this->exitScope();
            break;
        }
        case NodeKind::Identifier: {
            auto identifierNode = static_cast<IdentifierNode*>(node);
            auto name = this->currentScope->getName(identifierNode->getName());
            if (name == nullptr) {
                this->addErrorMessage("Name '" + identifierNode->getName() + "' not found in scope");
                break;
            }
            identifierNode->setNameReference(name);
            break;
        }
        default:
            for (auto& child : node->getChildren()) {
                this->bindRecursive(child);
            }
            break;
    }
}
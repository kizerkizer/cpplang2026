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

void Binder::bindRecursive(Node* node, bool doNotCreateScope) {
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
                // * Handle assignmentExpression ourselves since this is a declaration
                // * Recurse first because if we have
                //   var xxx = ...
                //   then xxx is not yet defined so ... cannot reference it
                // * Also, only recurse on right side (expression) since we're taking care of the identifier here
                this->bindRecursive(assignmentExpression->getExpression());
                auto identifierNode = assignmentExpression->getIdentifier();
                if (identifierNode) {
                    auto name = new Name(this->currentScope, variableDeclarationNode, NameKind::Variable, NameModifierFlags::None, identifierNode->getName());
                    if (this->currentScope->hasNameShallow(name->getNameString())
                ) { //|| (this->currentScope->getFirstFunctionContainingScope() && this->currentScope->getFirstFunctionContainingScope()->hasNameShallow(name->getNameString()))) {
                        this->addErrorMessage("Name '" + name->getNameString() + "' already in scope");
                    } else {
                        this->currentScope->setName(name);
                    }
                }
            }
            break;
        }
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            auto identifierNode = functionDeclarationNode->getIdentifier();
            if (!identifierNode) {
                // TODO
                // Anonymous function? Don't handle it? Handled by `var xxx = function (...) {...}` ?
                break;
            }
            auto name = new Name(this->currentScope, functionDeclarationNode, NameKind::Function, NameModifierFlags::None, identifierNode->getName());
            if (this->currentScope->hasNameShallow(name->getNameString())
        ) { //|| (this->currentScope->getFirstFunctionContainingScope() && this->currentScope->getFirstFunctionContainingScope()->hasNameShallow(name->getNameString()))) {
                this->addErrorMessage("Name '" + name->getNameString() + "' already in scope");
                break; // TODO ?
            }
            if (this->currentScope->hasName(name->getNameString())) {
                // TODO warn about shadowing
            }
            this->currentScope->setName(name);
            identifierNode->setNameReference(name); // TODO double check
            this->createAndEnterScope(ScopeKind::Function);
            this->currentScope->setNode(functionDeclarationNode);
            this->currentScope->setMyNameReference(name);
            for (auto parameter : functionDeclarationNode->getParameters()) {
                auto parameterIdentifierNode = parameter;
                auto parameterName = new Name(this->currentScope, parameterIdentifierNode, NameKind::Parameter, NameModifierFlags::None, parameterIdentifierNode->getName());
                if (this->currentScope->hasNameShallow(parameterName->getNameString())
            ) { //|| (this->currentScope->getFirstFunctionContainingScope() && this->currentScope->getFirstFunctionContainingScope()->hasNameShallow(parameterName->getNameString()))) {
                    // Can happen if parameter has duplicate name
                    this->addErrorMessage("Name '" + parameterName->getNameString() + "' already in scope");
                } else {
                    this->currentScope->setName(parameterName);
                }
            }
            this->bindRecursive(functionDeclarationNode->getBody(), true);
            this->exitScope();
            break;
        }
        case NodeKind::AssignmentExpression: {
            auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
            auto identifierNode = assignmentExpressionNode->getIdentifier();
            auto expressionNode = assignmentExpressionNode->getExpression();
            if (identifierNode) {
                auto name = this->currentScope->getName(identifierNode->getName());
                if (name == nullptr) {
                   this->addErrorMessage("Name '" + identifierNode->getName() + "' not found in scope");
                    break; 
                }
                identifierNode->setNameReference(name);
            }
            if (expressionNode) {
                this->bindRecursive(expressionNode);
            }
            break;
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            if (!doNotCreateScope) {
                this->createAndEnterScope(ScopeKind::Block);
                this->currentScope->setNode(blockStatementNode);
            }
            this->bindRecursive(blockStatementNode->getProgramNode());
            if (!doNotCreateScope) {
                this->exitScope();
            }
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
            auto name = new Name(this->currentScope, loopStatementNode, NameKind::Loop, NameModifierFlags::None, "");
            this->createAndEnterScope(ScopeKind::Loop);
            this->currentScope->setNode(loopStatementNode);
            this->currentScope->setMyNameReference(name); // Not in parent's scope since it's not a real name. That's OK.
            this->bindRecursive(loopStatementNode->getBody(), true); // Do not create another scope for the block
            this->exitScope();
            break;
        }
        case NodeKind::Identifier: {
            // TODO I Guess make sure this doesn't visit from the children of variable/function declaration etc 
            auto identifierNode = static_cast<IdentifierNode*>(node);
            if (identifierNode) {
                auto name = this->currentScope->getName(identifierNode->getName());
                if (name == nullptr) {
                   this->addErrorMessage("Name '" + identifierNode->getName() + "' not found in scope");
                    break; 
                }
                identifierNode->setNameReference(name);
            }
            break;
        }
        case NodeKind::ReturnStatement: {
            auto returnStatementNode = static_cast<ReturnStatementNode*>(node);
            auto functionScope = this->currentScope->getFirstFunctionContainingScope();
            auto functionNameReference = functionScope->getMyNameReference();
            this->bindRecursive(returnStatementNode->getExpression());
            returnStatementNode->setFunctionNameReference(functionNameReference);
            break;
        }
        case NodeKind::BreakStatement: {
            auto breakStatementNode = static_cast<BreakStatementNode*>(node);
            auto [index, line, column] = breakStatementNode->getFirstSourceCodeLocation();
            auto loopScope = this->currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because breaks inside loops enforced by parser
                this->addErrorMessage("'break' found outside of loop at line " + std::to_string(line) + ", col " + std::to_string(column));
                break; 
            }
            breakStatementNode->setLoopNameReference(loopScope->getMyNameReference());
            break;
        }
        case NodeKind::ContinueStatement: {
            auto continueStatementNode = static_cast<ContinueStatementNode*>(node);
            auto [index, line, column] = continueStatementNode->getFirstSourceCodeLocation();
            auto loopScope = this->currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because continueus inside loops enforced by parser
                this->addErrorMessage("'continue' found outside of loop at line " + std::to_string(line) + ", col " + std::to_string(column));
                break; 
            }
            continueStatementNode->setLoopNameReference(loopScope->getMyNameReference());
            break;
        }
        default:
            for (auto& child : node->getChildren()) {
                this->bindRecursive(child);
            }
            break;
    }
}
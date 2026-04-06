#include <memory>
#include <print>

#include "binder/binder.hpp"
#include "binder/scope.hpp"
#include "binder/symbol.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "parser/node.hpp"

// BinderResult
Node* BinderResult::getNode() const {
    return this->node;
}

Scope* BinderResult::getRootScope() const {
    return this->rootScope.get();
}

// Binder
Binder::Binder(Source* source, Diagnostics& diagnostics) : source(source), diagnostics(diagnostics) {
    this->rootScope = std::make_unique<Scope>(ScopeKind::Root, nullptr, nullptr);
    this->currentScope = this->rootScope.get();
}

void Binder::addErrorMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Error, DiagnosticMessageStage::Binder, span, this->source, message));
}

void Binder::addWarningMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Warning, DiagnosticMessageStage::Binder, span, this->source, message));
}

void Binder::addInfoMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Info, DiagnosticMessageStage::Binder, span, this->source, message));
}

Scope* Binder::createAndEnterScope(ScopeKind kind) {
    auto newScope = std::make_unique<Scope>(kind, this->currentScope, nullptr);
    Scope* newScopePtr = newScope.get();
    this->currentScope->addChildScope(std::move(newScope));
    this->currentScope = newScopePtr;
    return newScopePtr;
}

void Binder::exitScope() {
    if (this->currentScope != this->rootScope.get()) {
        this->currentScope = this->currentScope->getParent();
    }
}

// TODO at some point handle "imports" of namespaces/modules/packages (whatever we end up calling them) and the fact that they create a new scope and can contain names that are accessible from the current scope
// TODO handle type definitions and corresponding scopes
// TODO handle 'this'/'super'/other contextual keywords

std::unique_ptr<BinderResult> Binder::bind(Node* rootNode) {
    this->currentScope->setNode(rootNode);
    this->bindRecursive(rootNode);
    return std::make_unique<BinderResult>(rootNode, std::move(this->rootScope));
}

void Binder::bindVariableDeclaration(Node* node) {
    auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
    auto identifierNode = variableDeclarationNode->getIdentifier();
    auto expressionNode = variableDeclarationNode->getExpression();
    // TODO handle type annotation ?

    // * Recurse first because if we have
    //   var xxx = ...
    //   then xxx is not yet defined so ... cannot reference it
    // * Also, only recurse on right side (expression) since we're taking care of the identifier here
    if (expressionNode) {
        this->bindRecursive(expressionNode);
    }
    auto symbol = std::make_unique<Symbol>(this->currentScope, identifierNode, SymbolKind::Variable, SymbolModifierFlags::None, std::string(identifierNode->getName()));
    auto symbolPointer = symbol.get();
    if (this->currentScope->hasSymbolShallow(symbol->getNameString())) {
        this->addErrorMessage(6, "Redeclaration of name '" + symbol->getNameString() + "', which is already declared in the current scope", identifierNode->getSourceCodeLocationSpan());
    } else {
        this->currentScope->setSymbol(std::move(symbol));
    }
    identifierNode->setSymbolReference(symbolPointer);
}

void Binder::bindFunctionDeclaration(Node* node) {
    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
    auto identifierNode = functionDeclarationNode->getIdentifier();
    /*if (!identifierNode) {
        // TODO
        // Anonymous function? Don't handle it? Handled by `var xxx = function (...) {...}` ?
        break;
    }*/
    auto symbol = std::make_unique<Symbol>(this->currentScope, identifierNode, SymbolKind::Function, SymbolModifierFlags::None, std::string(identifierNode->getName()));
    if (this->currentScope->hasSymbolShallow(symbol->getNameString())) { 
        this->addErrorMessage(6, "Redeclaration of name '" + symbol->getNameString() + "', which is already in scope", identifierNode->getSourceCodeLocationSpan());
        // TODO ?
        return;
    }
    if (this->currentScope->hasSymbol(symbol->getNameString())) {
        // TODO warn about shadowing
    }
    auto symbolPointer = symbol.get();
    this->currentScope->setSymbol(std::move(symbol));
    identifierNode->setSymbolReference(symbolPointer); // TODO double check
    this->createAndEnterScope(ScopeKind::Function);
    this->currentScope->setNode(functionDeclarationNode);
    this->currentScope->setMySymbolReference(symbolPointer);
    for (auto parameter : functionDeclarationNode->getParameters()) {
        auto parameterIdentifierNode = parameter;
        auto parameterSymbol = std::make_unique<Symbol>(this->currentScope, parameterIdentifierNode, SymbolKind::Parameter, SymbolModifierFlags::None, std::string(parameterIdentifierNode->getName()));
        auto parameterSymbolPtr = parameterSymbol.get();
        if (this->currentScope->hasSymbolShallow(parameterSymbol->getNameString())) {
            // Can happen if parameter has duplicate name
            this->addErrorMessage(6, "Parameter name '" + parameterSymbol->getNameString() + "' already in scope", parameterIdentifierNode->getSourceCodeLocationSpan());
        } else {
            this->currentScope->setSymbol(std::move(parameterSymbol));
        }
        parameter->setSymbolReference(parameterSymbolPtr);
    }
    this->bindRecursive(functionDeclarationNode->getBody(), true); // do not create another scope for the function body block
    this->exitScope();
}

void Binder::bindAssignmentExpression(Node* node) {
    auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
    auto identifierNode = assignmentExpressionNode->getIdentifier();
    auto expressionNode = assignmentExpressionNode->getExpression();
    if (identifierNode) {
        auto name = this->currentScope->getSymbol(std::string(identifierNode->getName()));
        if (name == nullptr) {
            this->addErrorMessage(7, "Name '" + std::string(identifierNode->getName()) + "' not found in scope", identifierNode->getSourceCodeLocationSpan());
            return;
        }
        identifierNode->setSymbolReference(name);
    }
    if (expressionNode) {
        this->bindRecursive(expressionNode);
    }
}

void Binder::bindIfStatement(Node* node) {
    // TODO this depends on blocks for then/else branches
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
}

void Binder::bindRecursive(Node* node, bool doNotCreateScope) {
    if (!node) {
        return;
    }
    switch (node->getNodeKind()) {
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            this->bindRecursive(programNode->getExecutionListNode());
            break;
        }
        case NodeKind::ExecutionList: {
            auto executionListNode = static_cast<ExecutionListNode*>(node);
            for (auto& child : executionListNode->getChildren()) {
                this->bindRecursive(child);
            }
            break;
        }
        case NodeKind::VariableDeclaration: return this->bindVariableDeclaration(node);
        case NodeKind::FunctionDeclaration: return this->bindFunctionDeclaration(node);
        case NodeKind::AssignmentExpression: return this->bindAssignmentExpression(node);
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            if (!doNotCreateScope) {
                this->createAndEnterScope(ScopeKind::Block);
                this->currentScope->setNode(blockStatementNode);
            }
            this->bindRecursive(blockStatementNode->getExecutionListNode());
            if (!doNotCreateScope) {
                this->exitScope();
            }
            break;
        }
        case NodeKind::IfStatement: return this->bindIfStatement(node); 
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            auto symbol = std::make_unique<Symbol>(this->currentScope, loopStatementNode, SymbolKind::Loop, SymbolModifierFlags::None, "");
            auto symbolPtr = symbol.get();
            this->createAndEnterScope(ScopeKind::Loop);
            this->currentScope->setNode(loopStatementNode);
            this->currentScope->setMySymbolReference(symbolPtr); // Not in parent's scope since it's not a real name. That's OK.
            this->bindRecursive(loopStatementNode->getBody(), true); // Do not create another scope for the block
            this->exitScope();
            break;
        }
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::Identifier: {
            // TODO I Guess make sure this doesn't visit from the children of variable/function declaration etc 
            auto identifierNode = static_cast<IdentifierNode*>(node);
            if (identifierNode) {
                auto symbol = this->currentScope->getSymbol(std::string(identifierNode->getName()));
                if (symbol == nullptr) {
                   this->addErrorMessage(7, "Name '" + std::string(identifierNode->getName()) + "' not found in scope", identifierNode->getSourceCodeLocationSpan());
                   break; 
                }
                identifierNode->setSymbolReference(symbol);
            }
            break;
        }
        case NodeKind::ReturnStatement: {
            auto returnStatementNode = static_cast<ReturnStatementNode*>(node);
            auto functionScope = this->currentScope->getFirstFunctionContainingScope();
            auto functionSymbolReference = functionScope->getMySymbolReference();
            this->bindRecursive(returnStatementNode->getExpression());
            returnStatementNode->setFunctionNameReference(functionSymbolReference);
            break;
        }
        case NodeKind::BreakStatement: {
            auto breakStatementNode = static_cast<BreakStatementNode*>(node);
            auto loopScope = this->currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because breaks inside loops enforced by parser
                this->addErrorMessage(8, "'break' found outside of loop", breakStatementNode->getSourceCodeLocationSpan());
                break; 
            }
            breakStatementNode->setLoopSymbolReference(loopScope->getMySymbolReference());
            break;
        }
        case NodeKind::ContinueStatement: {
            auto continueStatementNode = static_cast<ContinueStatementNode*>(node);
            auto loopScope = this->currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because continues inside loops enforced by parser
                this->addErrorMessage(8, "'continue' found outside of loop", continueStatementNode->getSourceCodeLocationSpan());
                break; 
            }
            continueStatementNode->setLoopSymbolReference(loopScope->getMySymbolReference());
            break;
        }
        default:
            for (auto& child : node->getChildren()) {
                this->bindRecursive(child);
            }
            break;
    }
}
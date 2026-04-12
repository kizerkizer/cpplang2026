#include <memory>
#include <print>

#include "binder/binder.hpp"
#include "binder/scope.hpp"
#include "binder/symbol.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "parser/node.hpp"
#include "common/util.hpp"

// BinderResult
Node* BinderResult::getNode() const {
    return m_node;
}

Scope* BinderResult::getRootScope() const {
    return m_rootScope.get();
}

Symbol* BinderResult::getSymbol(Node* node) {
    auto value = m_nodeMap->getValue(node);
    if (value) {
        return value->symbol;
    }
    return nullptr;
}

Symbol* BinderResult::getFunctionSymbol(Node* node) {
    auto value = m_nodeMap->getValue(node);
    if (value) {
        return value->functionSymbol;
    }
    return nullptr;
}

Symbol* BinderResult::getLoopSymbol(Node* node) {
    auto value = m_nodeMap->getValue(node);
    if (value) {
        return value->loopSymbol;
    }
    return nullptr;
}

// Binder
ASTNodeInformation* Binder::info(Node* node) {
    return m_nodeMap->getValue(node);
}

void Binder::setInfo(Node* node, std::unique_ptr<ASTNodeInformation> info) {
    m_nodeMap->setValue(node, std::move(info));
}

Symbol* Binder::symbol(Node* node) {
    auto nodeInfo = this->info(node);
    if (nodeInfo) {
        return nodeInfo->symbol;
    }
    return nullptr;
}

void Binder::setNodeSymbol(Node* node, Symbol* symbol) {
    auto nodeInfo = this->info(node);
    if (nodeInfo) {
        nodeInfo->symbol = symbol;
    } else {
        auto newNodeInfo = std::make_unique<ASTNodeInformation>();
        newNodeInfo->symbol = symbol;
        this->setInfo(node, std::move(newNodeInfo));
    }
}

void Binder::setFunctionSymbol(Node* node, Symbol* symbol) {
    auto nodeInfo = this->info(node);
    if (nodeInfo) {
        nodeInfo->functionSymbol = symbol;
    } else {
        auto newNodeInfo = std::make_unique<ASTNodeInformation>();
        newNodeInfo->functionSymbol = symbol;
        this->setInfo(node, std::move(newNodeInfo));
    }
}

void Binder::setLoopSymbol(Node* node, Symbol* symbol) {
    auto nodeInfo = this->info(node);
    if (nodeInfo) {
        nodeInfo->loopSymbol = symbol;
    } else {
        auto newNodeInfo = std::make_unique<ASTNodeInformation>();
        newNodeInfo->loopSymbol = symbol;
        this->setInfo(node, std::move(newNodeInfo));
    }
}

Binder::Binder(Source* source, Diagnostics& diagnostics) : m_source(source), m_diagnostics(diagnostics) {
    m_rootScope = std::make_unique<Scope>(ScopeKind::Root, nullptr, nullptr);
    m_currentScope = m_rootScope.get();
    m_nodeMap = std::make_unique<NodeMap<ASTNodeInformation>>();
}

void Binder::addErrorMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    m_diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Error, DiagnosticMessageStage::Binder, span, m_source, message));
}

void Binder::addWarningMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    m_diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Warning, DiagnosticMessageStage::Binder, span, m_source, message));
}

void Binder::addInfoMessage(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeLocationSpan) {
    auto span = sourceCodeLocationSpan.value_or(SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    m_diagnostics.addDiagnosticMessage(DiagnosticMessage(code, DiagnosticMessageKind::Info, DiagnosticMessageStage::Binder, span, m_source, message));
}

Scope* Binder::createAndEnterScope(ScopeKind kind) {
    auto newScope = std::make_unique<Scope>(kind, m_currentScope, nullptr);
    Scope* newScopePtr = newScope.get();
    m_currentScope->addChildScope(std::move(newScope));
    m_currentScope = newScopePtr;
    return newScopePtr;
}

void Binder::exitScope() {
    if (!this->atRootScope()) {
        m_currentScope = m_currentScope->getParent();
    }
}

bool Binder::atRootScope() const {
    return m_rootScope.get() == m_currentScope; // Safer than checking if parent null
}

// TODO at some point handle "imports" of namespaces/modules/packages (whatever we end up calling them) and the fact that they create a new scope and can contain names that are accessible from the current scope
// TODO handle type definitions and corresponding scopes
// TODO handle 'this'/'super'/other contextual keywords

std::unique_ptr<BinderResult> Binder::bind(Node* rootNode) {
    m_currentScope->setMyNode(rootNode);
    this->bindRecursive(rootNode);
    return std::make_unique<BinderResult>(rootNode, std::move(m_rootScope), std::move(m_nodeMap));
}

void Binder::bindVariableDeclaration(Node* node) {
    auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
    auto typeExpressionNode = variableDeclarationNode->getTypeExpression();
    auto identifierNode = variableDeclarationNode->getIdentifier();
    auto expressionNode = variableDeclarationNode->getExpression();
    if (typeExpressionNode) {
        this->bindRecursive(typeExpressionNode);
    }
    // * Recurse first because if we have
    //   var xxx = ...
    //   then xxx is not yet defined so ... cannot reference it
    //
    // * Also, only recurse on right side (expression) since we're taking care of the identifier here
    if (expressionNode) {
        this->bindRecursive(expressionNode);
    }
    auto symbol = std::make_unique<Symbol>(m_currentScope, identifierNode, SymbolKind::Variable, SymbolModifierFlags::None, std::string(identifierNode->getName()));
    auto symbolPointer = symbol.get();
    if (m_currentScope->hasSymbolShallow(symbol->getNameString())) {
        this->addErrorMessage(6, "Redeclaration of name '" + symbol->getNameString() + "', which is already declared in the current scope", identifierNode->getSourceCodeLocationSpan());
        // return?
    } else {
        m_currentScope->putSymbol(std::move(symbol));
    }
    setNodeSymbol(identifierNode, symbolPointer);
}

void Binder::bindTypeDeclaration(Node* node) {
    auto typeDeclarationNode = static_cast<TypeDeclarationNode*>(node);
    auto identifierNode = typeDeclarationNode->getIdentifier();
    auto typeExpressionNode = typeDeclarationNode->getTypeExpression();
    if (!typeExpressionNode) {
        this->addErrorMessage(55, "Type declaration must have initial expression");
        return;
    }

    // * Recurse first because if we have
    //   type xxx = ...
    //   then xxx is not yet defined so ... cannot reference it
    //
    // * Also, only recurse on right side (type expression) since we're taking care of the identifier here
    this->bindRecursive(typeExpressionNode);
    auto symbol = std::make_unique<Symbol>(m_currentScope, identifierNode, SymbolKind::Type, SymbolModifierFlags::None, std::string(identifierNode->getName()));
    auto symbolPointer = symbol.get();
    if (m_currentScope->hasSymbolShallow(symbol->getNameString())) {
        this->addErrorMessage(6, "Redeclaration of name '" + symbol->getNameString() + "', which is already declared in the current scope", identifierNode->getSourceCodeLocationSpan());
        // return?
    } else {
        m_currentScope->putSymbol(std::move(symbol));
    }
    setNodeSymbol(identifierNode, symbolPointer);
}

void Binder::bindFunctionDeclaration(Node* node) {
    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
    auto identifierNode = functionDeclarationNode->getIdentifier();
    auto returnTypeExpressionNode = functionDeclarationNode->getReturnTypeExpression();
    if (returnTypeExpressionNode) {
        this->bindRecursive(returnTypeExpressionNode);
    }
    /*if (!identifierNode) {
        // TODO
        // Anonymous function? Don't handle it? Handled by `var xxx = function (...) {...}` ?
        break;
    }*/
    auto symbol = std::make_unique<Symbol>(m_currentScope, identifierNode, SymbolKind::Function, SymbolModifierFlags::None, std::string(identifierNode->getName()));
    if (m_currentScope->hasSymbolShallow(symbol->getNameString())) { 
        this->addErrorMessage(6, "Redeclaration of name '" + symbol->getNameString() + "', which is already in scope", identifierNode->getSourceCodeLocationSpan());
        // TODO ?
        return;
    }
    if (m_currentScope->hasSymbol(symbol->getNameString())) {
        // TODO warn about shadowing
    }
    auto symbolPointer = symbol.get();
    m_currentScope->putSymbol(std::move(symbol));
    setNodeSymbol(identifierNode, symbolPointer);
    this->createAndEnterScope(ScopeKind::Function);
    m_currentScope->setMyNode(functionDeclarationNode);
    m_currentScope->setMySymbolReference(symbolPointer);
    for (auto parameter : functionDeclarationNode->getParameters()) {
        auto parameterIdentifierNode = parameter;
        auto parameterSymbol = std::make_unique<Symbol>(m_currentScope, parameterIdentifierNode, SymbolKind::Parameter, SymbolModifierFlags::None, std::string(parameterIdentifierNode->getName()));
        auto parameterSymbolPtr = parameterSymbol.get();
        if (m_currentScope->hasSymbolShallow(parameterSymbol->getNameString())) {
            // Can happen if parameter has duplicate name
            this->addErrorMessage(6, "Parameter name '" + parameterSymbol->getNameString() + "' already in scope", parameterIdentifierNode->getSourceCodeLocationSpan());
        } else {
            m_currentScope->putSymbol(std::move(parameterSymbol));
        }
        setNodeSymbol(parameterIdentifierNode, parameterSymbolPtr);
    }
    this->bindRecursive(functionDeclarationNode->getBody(), true); // do not create another scope for the function body block
    this->exitScope();
}

void Binder::bindAssignmentExpression(Node* node) {
    auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
    auto identifierNode = assignmentExpressionNode->getIdentifier();
    auto expressionNode = assignmentExpressionNode->getExpression();
    if (identifierNode) {
        auto symbol = m_currentScope->getSymbol(std::string(identifierNode->getName()));
        if (symbol == nullptr) {
            this->addErrorMessage(7, "Symbol '" + std::string(identifierNode->getName()) + "' not found in scope", identifierNode->getSourceCodeLocationSpan());
            return;
        }
        setNodeSymbol(identifierNode, symbol);
    }
    if (expressionNode) {
        this->bindRecursive(expressionNode);
    }
}

void Binder::bindIfStatement(Node* node) {
    // TODO FIXME
    // Ifs currently depends on blocks for then/else branches,
    // so doesn't allow if... else if... else chains. Fix.
    auto ifStatementNode = static_cast<IfStatementNode*>(node);
    this->bindRecursive(ifStatementNode->getCondition());
    this->createAndEnterScope(ScopeKind::Block);
    m_currentScope->setMyNode(ifStatementNode->getThenBranch());
    this->bindRecursive(ifStatementNode->getThenBranch());
    this->exitScope();
    if (ifStatementNode->getElseBranch()) {
        this->createAndEnterScope(ScopeKind::Block);
        m_currentScope->setMyNode(ifStatementNode->getElseBranch());
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
        case NodeKind::TypeDeclaration: return this->bindTypeDeclaration(node);
        case NodeKind::FunctionDeclaration: return this->bindFunctionDeclaration(node);
        case NodeKind::AssignmentExpression: return this->bindAssignmentExpression(node);
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            if (!doNotCreateScope) {
                this->createAndEnterScope(ScopeKind::Block);
                m_currentScope->setMyNode(blockStatementNode);
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
            auto symbol = std::make_unique<Symbol>(m_currentScope, loopStatementNode, SymbolKind::Loop, SymbolModifierFlags::None, "");
            auto symbolPtr = symbol.get();
            this->createAndEnterScope(ScopeKind::Loop);
            m_currentScope->setMyNode(loopStatementNode);
            m_currentScope->setMySymbolReference(symbolPtr); // Not in parent's scope since it's not a real name. That's OK.
            this->bindRecursive(loopStatementNode->getBody(), true); // Do not create another scope for the block
            this->exitScope();
            break;
        }
        case NodeKind::TypeIdentifier:
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::Identifier: {

            // This doesn't visit from the children of variable/function/type declaration etc
            // So, this must be a reference to an already defined name. So, we find the symbol.
            auto identifierNode = static_cast<IdentifierNode*>(node);
            auto symbol = m_currentScope->getSymbol(std::string(identifierNode->getName()));
            if (symbol == nullptr) {
                this->addErrorMessage(7, "Name '" + std::string(identifierNode->getName()) + "' not found in scope", identifierNode->getSourceCodeLocationSpan());
                break; 
            }

            // Associate node with symbol:
            setNodeSymbol(identifierNode, symbol);
            break;
        }
        case NodeKind::ReturnStatement: {
            auto returnStatementNode = static_cast<ReturnStatementNode*>(node);
            auto functionScope = m_currentScope->getFirstFunctionContainingScope();
            auto functionSymbolReference = functionScope->getMySymbolReference();
            this->bindRecursive(returnStatementNode->getExpression());
            this->setFunctionSymbol(returnStatementNode, functionSymbolReference);
            break;
        }
        case NodeKind::BreakStatement: {
            auto breakStatementNode = static_cast<BreakStatementNode*>(node);
            auto loopScope = m_currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because breaks inside loops enforced by parser
                this->addErrorMessage(8, "'break' found outside of loop", breakStatementNode->getSourceCodeLocationSpan());
                break; 
            }
            this->setLoopSymbol(breakStatementNode, loopScope->getMySymbolReference());
            break;
        }
        case NodeKind::ContinueStatement: {
            auto continueStatementNode = static_cast<ContinueStatementNode*>(node);
            auto loopScope = m_currentScope->getFirstLoopContainingScope();
            if (!loopScope) {
                // Should never reach here because continues inside loops enforced by parser
                unreachable("'continue' found outside of loop");
                this->addErrorMessage(8, "'continue' found outside of loop", continueStatementNode->getSourceCodeLocationSpan());
                break; 
            }
            this->setLoopSymbol(continueStatementNode, loopScope->getMySymbolReference());
            break;
        }
        default:
            for (auto& child : node->getChildren()) {
                this->bindRecursive(child);
            }
            break;
    }
}
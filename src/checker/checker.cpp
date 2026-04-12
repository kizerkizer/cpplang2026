#include <memory>
#include <string>
#include <print>

#include "checker/checker.hpp"
#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "common/util.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"
#include "binder/symbol.hpp"
#include "common/util.hpp"

PrimitiveType* makePrimitive(Type* type) {
    if (type->getTypeKind() == TypeKind::Primitive) {
        return static_cast<PrimitiveType*>(type);
    }
    return nullptr;
}

// TypeChecker
void TypeChecker::addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan) {
    auto span = sourceCodeSpan.has_value() ? sourceCodeSpan.value() : SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1, -1), SourceCodeLocation(-1, -1, -1, -1));
    m_diagnostics.addDiagnosticMessage(DiagnosticMessage(code, kind, DiagnosticMessageStage::TypeChecker, span, m_source, message));
}

void TypeChecker::addError(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan) {
    this->addDiagnostic(DiagnosticMessageKind::Error, code, message, sourceCodeSpan);
}

void TypeChecker::addWarning(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan) {
    this->addDiagnostic(DiagnosticMessageKind::Warning, code, message, sourceCodeSpan);
}

void TypeChecker::addInfo(int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan) {
    this->addDiagnostic(DiagnosticMessageKind::Info, code, message, sourceCodeSpan);
}

Type* TypeChecker::setASTNodeType(Node* node, Type* type) {
    auto nodeInfo = m_nodeMap->getValue(node);
    if (nodeInfo) {
        nodeInfo->type = type;
    } else {
        auto newNodeInfo = std::make_unique<TypeInfo>();
        newNodeInfo->type = type;
        m_nodeMap->setValue(node, std::move(newNodeInfo));
    }
    return type;
}

Type* TypeChecker::getASTNodeType(Node* node) {
    auto nodeInfo = m_nodeMap->getValue(node);
    if (nodeInfo) {
        return nodeInfo->type;
    }
    return nullptr;
}

FlowGraph* TypeChecker::getFlowGraph(Node* node) {
    return m_flowBuilderResult.getFlowGraph(node);
}

FlowNode* TypeChecker::getFlowNode(Node* node) {
    return m_flowBuilderResult.getFlowNode(node);
}

Symbol* TypeChecker::getSymbol(Node* node) {
    return m_binderResult.getSymbol(node);
}

Symbol* TypeChecker::getFunctionSymbol(Node* node) {
    return m_binderResult.getFunctionSymbol(node);
}

Symbol* TypeChecker::getLoopSymbol(Node* node) {
    return m_binderResult.getLoopSymbol(node);
}

void TypeChecker::checkFunctionDeclarationReturns(Type* returnType, FunctionDeclarationNode* functionDeclarationNode) {
    auto flowGraph = this->getFlowGraph(functionDeclarationNode);
    std::vector<Node*> returnStatements;
    auto emptyReturnNode = ReturnStatementNode(nullptr,
        englishbreakfast::emptySourceCodeLocationSpan);
    for (auto node : flowGraph->getExit()->getPredecessors()) {
        if (!flowGraph->isASTNodeReachable(node->getAstNode())) {
            continue;
        }
        if (node->getAstNode()->getNodeKind() == NodeKind::ReturnStatement) {
            returnStatements.push_back(node->getAstNode());
        } else {
            returnStatements.push_back(&emptyReturnNode);
        }
    }
    for (auto returnStatement : returnStatements) {
        auto returnStatementNode = static_cast<ReturnStatementNode*>(returnStatement);
        auto returnStatementType = returnStatementNode->getExpression() == nullptr ? m_typeStore->getVoidType() : this->examine(returnStatementNode->getExpression());
        if (returnStatementType != m_typeStore->getVoidType() && !returnStatementType->isSubtypeOf(returnType)) {
            this->addError(21, "Return type is not compatible with function return type annotation", functionDeclarationNode->getSourceCodeLocationSpan());
            break;
        } else if (returnStatementType == m_typeStore->getVoidType() && returnType->getTypeKind() != TypeKind::Void) {
            this->addError(21, "Non-void function must return a value", functionDeclarationNode->getSourceCodeLocationSpan());
            break;
        }
    }
}

/**
 * @brief Sets types of each parameter on the parameter's symbol, and returns a vector of the parameter symbols.
 * 
 * @param functionDeclarationNode The function declaration node to check.
 * @return std::vector<Symbol*> A vector of the parameter symbols.
 */
std::vector<Symbol*> TypeChecker::checkFunctionDeclarationParameters(FunctionDeclarationNode* functionDeclarationNode) {
    std::vector<Symbol*> parameterSymbols;
    auto parameters = functionDeclarationNode->getParameters();
    for (const auto& parameter : parameters) {
        auto parameterSymbol = this->getSymbol(parameter);
        auto parameterAnnotation = parameter->getAnnotation();
        if (parameterAnnotation) {
            auto parameterTypeType = this->evaluateTypeExpression(parameterAnnotation);
            parameterSymbol->setType(parameterTypeType->getUnderlyingType());
        } else {
            // TODO Should eventually infer from usage of parameter in function body
            // If there's no parameter type annotation, then the parameter type is Any (for now)
            parameterSymbol->setType(m_typeStore->getAnyType());
        }
        parameterSymbols.push_back(parameterSymbol);
    }
    return parameterSymbols;
}

/**
 * @brief Computes the return type of a function declaration.
 * 
 * @param functionDeclarationNode The function declaration node to check.
 * @return Type* The return type of the function.
 */
Type* TypeChecker::computeFunctionDeclarationReturnType(FunctionDeclarationNode* functionDeclarationNode) {
    auto returnTypeExpression = functionDeclarationNode->getReturnTypeExpression();
    if (returnTypeExpression) {
        auto returnType = this->evaluateTypeExpression(returnTypeExpression);
        return returnType->getUnderlyingType();
    } else {
        // TODO Infer return type here eventually instead
        // If there's no return type annotation, then the return type is Void (for now)
        return m_typeStore->getVoidType();
    }
}

/**
 * @brief Gets the full function type (parameters' types and return type) of a function declaration.
 * 
 * @param functionDeclarationNode The function declaration node to check.
 * @return FunctionType* The full function type of the function declaration.
 */
FunctionType* TypeChecker::getFunctionDeclarationFunctionType(FunctionDeclarationNode* functionDeclarationNode) {
    auto parameterSymbols = this->checkFunctionDeclarationParameters(functionDeclarationNode);
    auto returnType = this->computeFunctionDeclarationReturnType(functionDeclarationNode);
    return m_typeStore->createType<FunctionType>(parameterSymbols, returnType);
}

Type* TypeChecker::examineIdentifier(Node* node) {
    auto symbol = this->getSymbol(node);
    if (symbol) {
        if (!symbol->getType()) {
            if (symbol->getDefiningNode() == node) {
                unreachable();
                return nullptr;
            }
            symbol->setType(this->examine(symbol->getDefiningNode()));
        }
        return symbol->getType();
    }
    unreachable();
    return nullptr;
}

Type* TypeChecker::examineBinaryOperatorExpression(Node* node) {
    auto* binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node);
    auto leftType = this->examine(binaryOperatorExpressionNode->getLeft());
    auto rightType = this->examine(binaryOperatorExpressionNode->getRight());
    auto operatorToken = *binaryOperatorExpressionNode->getOperatorToken();

    // First, check for '+' string concatenation
    if (operatorToken == TokenKind::Plus && leftType->isString() && rightType->isString()) {
        return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getStringType());
    }

    // Arithmetic operators
    if (operatorToken == TokenKind::AsteriskAsterisk || operatorToken == TokenKind::Plus || operatorToken == TokenKind::Dash || operatorToken == TokenKind::Asterisk || operatorToken == TokenKind::Slash) {

        // TODO eventually support proper coercion (Integer -> Float). 
        // For now, if both sides are floats, the result is a float. If both sides are integers, the result is an integer.
        if (leftType->isInteger() && rightType->isInteger()) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getIntegerType());
        }
        if (leftType->isFloat() && rightType->isFloat()) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getFloatType());
        }

        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid types for +, -, *, / (arithmetic)
        this->addError(11, "Bad types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr; // TODO maybe return void at some point?
    }

    // Logical operators
    if ((operatorToken == TokenKind::AmpersandAmpersand || operatorToken == TokenKind::PipePipe) && leftType->isBoolean() && rightType->isBoolean()) {
        return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getBooleanType());
    } else if (operatorToken == TokenKind::AmpersandAmpersand || operatorToken == TokenKind::PipePipe) {
        // Invalid types for &&, ||
        this->addError(11, "Non-Boolean types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }

    // Number comparison operators
    if (operatorToken == TokenKind::LessThan || operatorToken == TokenKind::GreaterThan || operatorToken == TokenKind::LessThanEqual || operatorToken == TokenKind::GreaterThanEqual) {
        if ((leftType->isInteger() || leftType->isFloat()) && (rightType->isInteger() || rightType->isFloat())) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getBooleanType());
        }
        // TODO eventually support operator overloading, but for now only allow primitive numeric types.
        // Invalid types for comparison operators
        this->addError(11, "Bad types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }

    // Equality operators
    if (operatorToken == TokenKind::Is) {
        if (!rightType->isTypeType()) {
            this->addError(11, "Right-hand side of 'is' operator must be a type", operatorToken.getSourceCodeLocationSpan());
            return nullptr;
        }
    }
    if (operatorToken == TokenKind::Is || operatorToken == TokenKind::EqualEqual || operatorToken == TokenKind::NotEqual) {
        return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getBooleanType());
    }
    // Unknown binary operator (still need to handle dot and equal operators).
    this->addError(12, "Unknown operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
    return nullptr; // TODO maybe return void at some point?
}

TypeType* TypeChecker::evaluateTypeExpression(Node* node) {
    if (node->getNodeKind() == NodeKind::TypePrimitive) {
        auto typePrimitiveNode = static_cast<TypePrimitiveNode*>(node);
        auto primitiveType = m_typeStore->createType<PrimitiveType>(typePrimitiveNode->getPrimitiveTypeKind());
        this->setASTNodeType(typePrimitiveNode, primitiveType);
        return m_typeStore->makeTypeType(primitiveType);
    }
    if (node->getNodeKind() == NodeKind::TypeIdentifier) {
        auto symbol = this->getSymbol(node);
        ASSERT(symbol != nullptr);
        ASSERT(symbol->getType() != nullptr);
        return m_typeStore->makeTypeType(symbol->getType());
    }
    if (node->getNodeKind() == NodeKind::BinaryOperatorTypeExpression) {
        auto binaryOperatorTypeExpressionNode = static_cast<BinaryOperatorTypeExpressionNode*>(node);
        if (binaryOperatorTypeExpressionNode->getOperatorToken()->getTokenKind() == TokenKind::Pipe) {
            auto leftTypeType = this->evaluateTypeExpression(binaryOperatorTypeExpressionNode->getLeft());
            auto rightTypeType = this->evaluateTypeExpression(binaryOperatorTypeExpressionNode->getRight());
            auto leftType = leftTypeType->getUnderlyingType();
            auto rightType = rightTypeType->getUnderlyingType();
            auto unionType = m_typeStore->createType<UnionType>(leftType, rightType);
            auto simplifiedUnionType = m_typeStore->simplifyType(unionType);
            auto simplifiedUnionTypeType = m_typeStore->makeTypeType(simplifiedUnionType);
            this->setASTNodeType(binaryOperatorTypeExpressionNode, simplifiedUnionTypeType);
            return simplifiedUnionTypeType;
        }
        this->addError(57, "Unsupported operator used in type expression", binaryOperatorTypeExpressionNode->getSourceCodeLocationSpan());
        return nullptr; // TODO
    }
    unreachable();
}

Type* TypeChecker::examineTypeDeclaration(Node* node) {
    auto typeDeclarationNode = static_cast<TypeDeclarationNode*>(node);
    auto symbol = this->getSymbol(typeDeclarationNode->getIdentifier());
    ASSERT(symbol != NULL);
    symbol->setType(this->evaluateTypeExpression(typeDeclarationNode->getTypeExpression())->getUnderlyingType());
    return symbol->getType();
}

Type* TypeChecker::examineUnaryOperatorExpression(Node* node) {
    auto* unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node);
    auto operandType = this->examine(unaryOperatorExpressionNode->getOperand());
    auto operatorToken = *unaryOperatorExpressionNode->getOperatorToken();
    if (operatorToken == TokenKind::Dash) {
        if (operandType->isInteger() || operandType->isFloat()) {
            auto operandPrimitiveType = makePrimitive(operandType);
            return this->setASTNodeType(unaryOperatorExpressionNode, m_typeStore->createType<PrimitiveType>(operandPrimitiveType->getPrimitiveTypeKind()));
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid type for unary -
        this->addError(11, "Bad type for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }
    if (operatorToken == TokenKind::Not) {
        if (operandType->isBoolean()) {
            return this->setASTNodeType(unaryOperatorExpressionNode, m_typeStore->getBooleanType());
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid type for unary !
        this->addError(11, "Bad type for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }
    this->addError(12, "Unknown operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
    return nullptr; // TODO maybe return void at some point?
}

Type* TypeChecker::examineIfExpression(Node* node) {
    auto* ifExpressionNode = static_cast<IfExpressionNode*>(node);
    auto conditionType = this->examine(ifExpressionNode->getCondition());
    if (!(ifExpressionNode->getThenBranch() && ifExpressionNode->getElseBranch())) {
        this->addError(13, "If expressions must have both then and else branches", ifExpressionNode->getSourceCodeLocationSpan());
        return nullptr;
    }
    if (conditionType->isBoolean()) {
        auto thenBranchType = this->examine(ifExpressionNode->getThenBranch());
        auto elseBranchType = this->examine(ifExpressionNode->getElseBranch());
        auto unionType = m_typeStore->createType<UnionType>(thenBranchType, elseBranchType);
        auto type = m_typeStore->simplifyType(unionType);
        return this->setASTNodeType(ifExpressionNode, type);
    }
    // Invalid type for if condition
    this->addError(14, "Bad type for if condition. Expected boolean.", ifExpressionNode->getCondition()->getSourceCodeLocationSpan());
    return nullptr;
}

Type* TypeChecker::examineFunctionCallExpression(Node* node) {
    auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
    auto functionType = static_cast<FunctionType*>(this->getSymbol(functionCallExpressionNode->getIdentifier())->getType());
    auto arguments = functionCallExpressionNode->getArgumentNodes();
    auto parameters = functionType->getParameters();
    if (arguments.size() != parameters.size()) {
        this->addError(15, "Argument count does not match parameter count in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        return functionType->getReturnType();
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        auto argumentType = this->examine(arguments[i]);
        auto parameterType = parameters[i]->getType();
        if (!argumentType->isSubtypeOf(parameterType)) {
            this->addError(16, std::string("Argument type '") + argumentType->toString() + "' is not assignable to parameter type '" + parameterType->toString() + "' in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        }
    }
    return functionType->getReturnType();
}

Type* TypeChecker::examineVariableDeclaration(Node* node) {
    auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
    auto identifierNode = variableDeclarationNode->getIdentifier();
    auto typeExpressionNode = variableDeclarationNode->getTypeExpression();
    auto expressionNode = variableDeclarationNode->getExpression();
    auto symbol = this->getSymbol(identifierNode);
    Type* expressionType = nullptr;
    Type* computedType = nullptr;
    if (expressionNode) {
        expressionType = this->examine(expressionNode);
    } else {
        // if there's no initializer expression, then the type annotation is required, and Empty must be assignable to it.
        expressionType = m_typeStore->getEmptyType();
    }
    if (!expressionType) {
        // if the expression type is null, then there was an error examining the expression, so just return void to avoid spamming errors about incompatible types
        return m_typeStore->getVoidType();
    }
    if (typeExpressionNode) {
        // Set type to what is annotated
        auto typeExpressionType = this->evaluateTypeExpression(typeExpressionNode);
        auto typeExpressionUnderlyingType = typeExpressionType->getUnderlyingType();
        if (!expressionType->isSubtypeOf(typeExpressionUnderlyingType)) {
            this->addError(18, "Initializer expression type is not compatible with type annotation", identifierNode->getSourceCodeLocationSpan());
        }
        computedType = typeExpressionUnderlyingType;
    } else if (!expressionNode) {
        this->addError(19, "Variable declaration with no initializer must have a type annotation that is a supertype of Empty", identifierNode->getSourceCodeLocationSpan());
        computedType = expressionType;
    } else {
        // Infer type as type of expression
        computedType = expressionType;
    }
    if (symbol) {
        symbol->setType(computedType);
    } else {
        // Shouldn't happen since binder should have created a symbol for the variable declaration
        unreachable();
    }
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineFunctionDeclaration(Node* node) {
    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
    auto symbol = this->getSymbol(functionDeclarationNode->getIdentifier());
    auto functionType = this->getFunctionDeclarationFunctionType(functionDeclarationNode);
    if (symbol) {
        symbol->setType(functionType);
    } else {
        // Shouldn't happen since binder should have created a symbol for the function declaration
        unreachable();
    }
    // Examine body
    this->examine(functionDeclarationNode->getBody());
    // Check function returns after body:
    checkFunctionDeclarationReturns(functionType->getReturnType(), functionDeclarationNode);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineIfStatement(Node* node) {
    auto ifStatementNode = static_cast<IfStatementNode*>(node);
    auto conditionType = this->examine(ifStatementNode->getCondition());
    if (conditionType->isBoolean()) {
        this->examine(ifStatementNode->getThenBranch());
        if (ifStatementNode->getElseBranch()) {
            this->examine(ifStatementNode->getElseBranch());
        }
    } else {
        this->addError(14, "Bad type for if condition. Expected Boolean.", ifStatementNode->getCondition()->getSourceCodeLocationSpan());
    }
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examine(Node* node) {
    if (!node) {
        return nullptr;
    }
    switch (node->getNodeKind()) {
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::Identifier: return this->examineIdentifier(node);
        case NodeKind::TypeDeclaration: return this->examineTypeDeclaration(node);
        case NodeKind::TypeIdentifier:
        case NodeKind::TypePrimitive:
        case NodeKind::BinaryOperatorTypeExpression:
            return this->evaluateTypeExpression(node);
        case NodeKind::IntegerLiteral: {
            return this->setASTNodeType(node, m_typeStore->getIntegerType());
        }
        case NodeKind::FloatLiteral: {
            return this->setASTNodeType(node, m_typeStore->getFloatType());
        }
        case NodeKind::StringLiteral: {
            return this->setASTNodeType(node, m_typeStore->getStringType());
        }
        case NodeKind::BooleanLiteral: {
            return this->setASTNodeType(node, m_typeStore->getBooleanType());
        }
        case NodeKind::EmptyLiteral: {
            return this->setASTNodeType(node, m_typeStore->getEmptyType());
        }
        case NodeKind::BinaryOperatorExpression: return this->examineBinaryOperatorExpression(node);
        case NodeKind::UnaryOperatorExpression: return this->examineUnaryOperatorExpression(node);
        case NodeKind::IfExpression: return this->examineIfExpression(node);
        case NodeKind::FunctionCallStatement: {
            auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
            auto functionCallExpressionNode = functionCallStatementNode->getFunctionCallExpression();
            this->examine(functionCallExpressionNode);
            return m_typeStore->getVoidType();
        }
        case NodeKind::FunctionCallExpression: return this->examineFunctionCallExpression(node);
        case NodeKind::VariableDeclaration: return this->examineVariableDeclaration(node);
        case NodeKind::FunctionDeclaration: return this->examineFunctionDeclaration(node);
        case NodeKind::IfStatement: return this->examineIfStatement(node);
        case NodeKind::ExecutionList: {
            auto executionListNode = static_cast<ExecutionListNode*>(node);
            for (auto child : executionListNode->getChildren()) {
                this->examine(child);
            }
            return m_typeStore->getVoidType();
        }
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            this->examine(programNode->getExecutionListNode());
            return m_typeStore->getVoidType();
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            this->examine(blockStatementNode->getExecutionListNode());
            return m_typeStore->getVoidType();
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            this->examine(loopStatementNode->getBody());
            return m_typeStore->getVoidType();
        }
        case NodeKind::ReturnStatement:
        case NodeKind::Invalid: // shouldn't exist
        case NodeKind::ContinueStatement:
        case NodeKind::BreakStatement:
        case NodeKind::WhileStatement: // shouldn't exist
        {
            return m_typeStore->getVoidType();
        }
        case NodeKind::AssignmentStatement: {
            auto assignmentStatementNode = static_cast<AssignmentStatementNode*>(node);
            this->examine(assignmentStatementNode->getAssignmentExpression());
            return m_typeStore->getVoidType();
        }
        case NodeKind::AssignmentExpression: {
            auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
            auto identifierType = this->examine(assignmentExpressionNode->getIdentifier());
            auto expressionType = this->examine(assignmentExpressionNode->getExpression());
            if (!expressionType->isSubtypeOf(identifierType)) {
                this->addError(22, "Type of right-hand side of assignment is not compatible with type of left-hand side.", assignmentExpressionNode->getSourceCodeLocationSpan());
            }
            return expressionType;
        }
    }
}

std::unique_ptr<TypeCheckerResult> TypeChecker::typeCheck(Node* rootNode) {
    this->examine(rootNode);
    auto typeCheckerResult = std::make_unique<TypeCheckerResult>(std::move(m_nodeMap), std::move(m_typeStore));
    return typeCheckerResult;
}

// TypeCheckerResult
NodeMap<TypeInfo>* TypeCheckerResult::getNodeMap() const {
    return m_nodeMap.get();
}

std::unique_ptr<NodeMap<TypeInfo>> TypeCheckerResult::takeNodeMap() {
    return std::move(m_nodeMap);
}

TypeStore* TypeCheckerResult::getTypeStore() const {
    return m_typeStore.get();
}

std::unique_ptr<TypeStore> TypeCheckerResult::takeTypeStore() {
    return std::move(m_typeStore);
}

void TypeCheckerResult::setTypeStore(std::unique_ptr<TypeStore> typeStore) {
    m_typeStore = std::move(typeStore);
}

void TypeCheckerResult::setNodeMap(std::unique_ptr<NodeMap<TypeInfo>> nodeMap) {
    m_nodeMap = std::move(nodeMap);
}

Type* TypeCheckerResult::getASTNodeType(Node* node) {
    auto nodeInfo = m_nodeMap->getValue(node);
    if (nodeInfo) {
        return nodeInfo->type;
    }
    return nullptr;
}
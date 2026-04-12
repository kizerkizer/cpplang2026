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

/**
 * @brief Checks that all return statements in a function declaration are compatible with the function's return type annotation.
 * 
 * @param returnType The return type to check against.
 * @param functionDeclarationNode The function declaration node to check.
 */
void TypeChecker::checkFunctionDeclarationReturns(Type* returnType, FunctionDeclarationNode* functionDeclarationNode) {
    auto flowGraph = this->getFlowGraph(functionDeclarationNode);
    std::vector<Node*> returnStatements;
    auto emptyReturnNode = ReturnStatementNode(nullptr,
        englishbreakfast::emptySourceCodeLocationSpan);

    // First, gather up all reachable return statements, and consider any implicit "return;" at the end of the function body as well
    for (auto node : flowGraph->getExit()->getPredecessors()) {

        // Only consider reachable nodes
        if (!flowGraph->isASTNodeReachable(node->getAstNode())) {
            continue;
        }

        // If the exit node's predecessor is not a return statement, we treat it as an implicit "return;" at the end of the function body
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
    auto returnTypeAnnotation = functionDeclarationNode->getReturnTypeAnnotation();
    if (returnTypeAnnotation) {
        auto returnType = this->evaluateTypeExpression(returnTypeAnnotation);
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
    ASSERT(symbol != nullptr);

    // Set symbol type if necessary
    if (!symbol->getType()) {
        if (symbol->getDefiningNode() == node) {
            // Shouldn't happen. We don't recurse on defining node. Would cause infinite recursion.
            unreachable();
            return nullptr;
        }
        symbol->setType(this->examine(symbol->getDefiningNode()));
    }

    // Identifier's type is the type of its symbol
    return symbol->getType();
}

Type* TypeChecker::examineBinaryOperatorExpression(Node* node) {
    auto binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node);
    auto leftType = this->examine(binaryOperatorExpressionNode->getLeft());
    auto rightType = this->examine(binaryOperatorExpressionNode->getRight());
    auto operatorToken = *binaryOperatorExpressionNode->getOperatorToken();

    // TODO Eventually support operator overloading, but for now just handle primitive types and string concatenation.

    // First, check for string concatenation using '+'
    if (operatorToken == TokenKind::Plus && leftType->isString() && rightType->isString()) {
        return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getStringType());
    }

    // Arithmetic operators
    if (operatorToken == TokenKind::AsteriskAsterisk || operatorToken == TokenKind::Plus || operatorToken == TokenKind::Dash || operatorToken == TokenKind::Asterisk || operatorToken == TokenKind::Slash) {

        // TODO Eventually support proper coercion (Integer -> Float). 
        // For now, if both sides are floats, the result is a float. If both sides are integers, the result is an integer.
        if (leftType->isInteger() && rightType->isInteger()) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getIntegerType());
        }
        if (leftType->isFloat() && rightType->isFloat()) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getFloatType());
        }

        // Invalid types for arithmetic operators
        this->addError(11, "Bad types for arithmetic operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
    }

    // Logical operators
    if ((operatorToken == TokenKind::AmpersandAmpersand || operatorToken == TokenKind::PipePipe) && leftType->isBoolean() && rightType->isBoolean()) {
        return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getBooleanType());
    } else if (operatorToken == TokenKind::AmpersandAmpersand || operatorToken == TokenKind::PipePipe) {

        // Invalid types for logical operators
        this->addError(11, "Non-Boolean types for logical operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
    }

    // Number comparison operators
    if (operatorToken == TokenKind::LessThan || operatorToken == TokenKind::GreaterThan || operatorToken == TokenKind::LessThanEqual || operatorToken == TokenKind::GreaterThanEqual) {
        if ((leftType->isInteger() || leftType->isFloat()) && (rightType->isInteger() || rightType->isFloat())) {
            return this->setASTNodeType(binaryOperatorExpressionNode, m_typeStore->getBooleanType());
        }

        // Invalid types for number comparison operators
        this->addError(11, "Bad types for number comparison operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
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

    // Unknown binary operator (still need to handle dot and equal operators)
    this->addError(12, "Unknown operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
    return m_typeStore->getErrorType();
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
        if (binaryOperatorTypeExpressionNode->getOperatorTokenKind() == TokenKind::Pipe) {
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
        this->addError(57, "Unknown operator '" + std::string(binaryOperatorTypeExpressionNode->getOperatorToken()->getSourceString()) + "' used in type expression", binaryOperatorTypeExpressionNode->getSourceCodeLocationSpan());
        return nullptr; // TODO
    }
    unreachable();
}

Type* TypeChecker::examineTypeDeclaration(Node* node) {
    auto typeDeclarationNode = static_cast<TypeDeclarationNode*>(node);
    auto identifierNode = typeDeclarationNode->getIdentifier();
    auto typeExpressionNode = typeDeclarationNode->getTypeExpression();
    auto symbol = this->getSymbol(identifierNode);
    ASSERT(symbol != NULL);
    symbol->setType(this->evaluateTypeExpression(typeExpressionNode)->getUnderlyingType());
    return symbol->getType();
}

Type* TypeChecker::examineUnaryOperatorExpression(Node* node) {
    auto* unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node);
    auto operand = unaryOperatorExpressionNode->getOperand();
    auto operandType = this->examine(operand);
    auto operatorToken = *unaryOperatorExpressionNode->getOperatorToken();

    // Unary minus (arithmetic)
    if (operatorToken == TokenKind::Dash) {
        if (operandType->isInteger() || operandType->isFloat()) {
            auto operandPrimitiveType = makePrimitive(operandType);
            return this->setASTNodeType(unaryOperatorExpressionNode, m_typeStore->createType<PrimitiveType>(operandPrimitiveType->getPrimitiveTypeKind()));
        }

        // Invalid type for unary minus
        this->addError(11, "Bad type for operator '-'", operatorToken.getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
    }

    // Unary not (logical)
    if (operatorToken == TokenKind::Not) {
        if (operandType->isBoolean()) {
            return this->setASTNodeType(unaryOperatorExpressionNode, m_typeStore->getBooleanType());
        }

        // Invalid type for Unary not
        this->addError(11, "Bad type for operator '!'", operatorToken.getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
    }
    
    // Otherwise, unknown unary operator
    this->addError(12, "Unknown operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
    return m_typeStore->getErrorType();
}

Type* TypeChecker::examineIfExpression(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::IfExpression);
    auto* ifExpressionNode = static_cast<IfExpressionNode*>(node);
    auto conditionType = this->examine(ifExpressionNode->getCondition());

    // First, check that both then and else branches exist, since if expressions must have both branches.
    if (!(ifExpressionNode->getThenBranch() && ifExpressionNode->getElseBranch())) {
        this->addError(13, "If expressions must have both then and else branches", ifExpressionNode->getSourceCodeLocationSpan());
        return m_typeStore->getErrorType();
    }

    // If condition is boolean, then the type of the if expression is the union of the types of the then and else branches.
    if (conditionType->isBoolean()) {
        auto thenBranchType = this->examine(ifExpressionNode->getThenBranch());
        auto elseBranchType = this->examine(ifExpressionNode->getElseBranch());
        auto unionType = m_typeStore->createType<UnionType>(thenBranchType, elseBranchType);
        auto type = m_typeStore->simplifyType(unionType);
        return this->setASTNodeType(ifExpressionNode, type);
    }

    // Otherwise, we have an invalid type for the if expression condition
    this->addError(14, "Bad type for if expression condition. Expected boolean.", ifExpressionNode->getCondition()->getSourceCodeLocationSpan());
    return m_typeStore->getErrorType();
}

Type* TypeChecker::examineFunctionCallExpression(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::FunctionCallExpression);
    auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
    auto symbol = this->getSymbol(functionCallExpressionNode->getIdentifier());
    ASSERT(symbol != nullptr);
    auto functionType = static_cast<FunctionType*>(symbol->getType());
    auto arguments = functionCallExpressionNode->getArgumentNodes();
    auto parameterSymbols = functionType->getParameters();
    auto returnType = functionType->getReturnType();

    // Check argument count matches parameter count
    if (arguments.size() != parameterSymbols.size()) {
        this->addError(15, "Argument count does not match parameter count in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        return returnType;
    }

    // Check argument types are compatible with parameter types
    for (size_t i = 0; i < arguments.size(); i++) {
        auto argumentType = this->examine(arguments[i]);
        auto parameterType = parameterSymbols[i]->getType();
        if (!argumentType->isSubtypeOf(parameterType)) {
            this->addError(16, std::string("Argument type '") + argumentType->toString() + "' is not assignable to parameter type '" + parameterType->toString() + "' in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        }
    }

    // Type of result of call is return type of function
    return this->setASTNodeType(functionCallExpressionNode, returnType);
}

Type* TypeChecker::examineVariableDeclaration(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::VariableDeclaration);
    auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
    auto identifier = variableDeclarationNode->getIdentifier();
    auto annotation = variableDeclarationNode->getTypeAnnotation();
    auto initializerExpression = variableDeclarationNode->getExpression();
    auto symbol = this->getSymbol(identifier);
    ASSERT(symbol != nullptr);

    Type* expressionType = nullptr;
    Type* variableType = nullptr;

    if (initializerExpression) {
        expressionType = this->examine(initializerExpression);
    } else {

        // If there's no initializer expression, then the type annotation is required, and Empty must be assignable to it.
        // Set the initializer expression type to Empty.
        expressionType = m_typeStore->getEmptyType();
    }

    if (annotation) {

        // Examine type annotation
        auto typeExpressionType = this->evaluateTypeExpression(annotation);
        auto typeExpressionUnderlyingType = typeExpressionType->getUnderlyingType();

        // Check that the initializer expression type is compatible with the type annotation
        if (!expressionType->isSubtypeOf(typeExpressionUnderlyingType)) {
            this->addError(18, "Initializer expression type is not compatible with type annotation", identifier->getSourceCodeLocationSpan());
        }

        // Set variable type to type indicated by annotation
        variableType = typeExpressionUnderlyingType;
    } else if (!initializerExpression) {
        this->addError(19, "Variable declaration with no initializer must have a type annotation, and that annotation must be a supertype of Empty", identifier->getSourceCodeLocationSpan());
        variableType = expressionType;
    } else {

        // No annotation but we have an initializer expression, so we infer variable type as type of expression
        variableType = expressionType;
    }

    symbol->setType(variableType);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineFunctionDeclaration(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::FunctionDeclaration);
    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
    auto symbol = this->getSymbol(functionDeclarationNode->getIdentifier());
    auto functionType = this->getFunctionDeclarationFunctionType(functionDeclarationNode);
    ASSERT(symbol != nullptr);
    symbol->setType(functionType);

    // Examine body
    this->examine(functionDeclarationNode->getBody());

    // Check function returns after examining body
    this->checkFunctionDeclarationReturns(functionType->getReturnType(), functionDeclarationNode);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineIfStatement(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::IfStatement);
    auto ifStatementNode = static_cast<IfStatementNode*>(node);
    auto condition = ifStatementNode->getCondition();
    auto thenBranch = ifStatementNode->getThenBranch();
    auto elseBranch = ifStatementNode->getElseBranch();
    auto conditionType = this->examine(condition);
    
    // Check condition type is boolean
    if (conditionType->isBoolean()) {
        // Check then branch (and else branch if it exists)
        if (thenBranch) {
            this->examine(thenBranch);
            if (elseBranch) {
                this->examine(elseBranch);
            }
        } else {
            this->addError(17, "If statement with no body", ifStatementNode->getSourceCodeLocationSpan());
        }
    } else {
        this->addError(14, "Bad type for if statement condition. Expected Boolean.", condition->getSourceCodeLocationSpan());
    }
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineFunctionCallStatement(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::FunctionCallStatement);
    auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
    auto functionCallExpressionNode = functionCallStatementNode->getFunctionCallExpression();
    this->examine(functionCallExpressionNode);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineExecutionList(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::ExecutionList);
    auto executionListNode = static_cast<ExecutionListNode*>(node);
    for (auto child : executionListNode->getChildren()) {
        this->examine(child);
    }
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineAssignmentStatement(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::AssignmentStatement);
    auto assignmentStatementNode = static_cast<AssignmentStatementNode*>(node);
    auto assignmentExpressionNode = assignmentStatementNode->getAssignmentExpression();
    this->examine(assignmentExpressionNode);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineAssignmentExpression(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::AssignmentExpression);
    auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
    auto identifier = assignmentExpressionNode->getIdentifier();
    auto expression = assignmentExpressionNode->getExpression();
    auto identifierType = this->examine(identifier);
    auto expressionType = this->examine(expression);
    if (!expressionType->isSubtypeOf(identifierType)) {
        this->addError(22, "Type of right-hand side of assignment is not compatible with type of left-hand side.", assignmentExpressionNode->getSourceCodeLocationSpan());
    }
    return expressionType;
}

Type* TypeChecker::examineLoopStatement(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::LoopStatement);
    auto loopStatementNode = static_cast<LoopStatementNode*>(node);
    auto body = loopStatementNode->getBody();
    this->examine(body);
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineProgram(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::Program);
    auto programNode = static_cast<ProgramNode*>(node);
    this->examine(programNode->getExecutionListNode());
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examineBlockStatement(Node* node) {
    ASSERT(node->getNodeKind() == NodeKind::BlockStatement);
    auto blockStatementNode = static_cast<BlockStatementNode*>(node);
    this->examine(blockStatementNode->getExecutionListNode());
    return m_typeStore->getVoidType();
}

Type* TypeChecker::examine(Node* node) {
    if (!node) {
        return nullptr;
    }
    switch (node->getNodeKind()) {
        using enum NodeKind;

        case IdentifierWithPossibleAnnotation:
        case Identifier:
            return this->examineIdentifier(node);

        case TypeIdentifier:
        case TypePrimitive:
        case BinaryOperatorTypeExpression:
            return this->evaluateTypeExpression(node);

        case IntegerLiteral: return this->setASTNodeType(node, m_typeStore->getIntegerType());
        case FloatLiteral: return this->setASTNodeType(node, m_typeStore->getFloatType());
        case StringLiteral: return this->setASTNodeType(node, m_typeStore->getStringType());
        case BooleanLiteral: return this->setASTNodeType(node, m_typeStore->getBooleanType());
        case EmptyLiteral: return this->setASTNodeType(node, m_typeStore->getEmptyType());

        case TypeDeclaration: return this->examineTypeDeclaration(node);
        case BinaryOperatorExpression: return this->examineBinaryOperatorExpression(node);
        case UnaryOperatorExpression: return this->examineUnaryOperatorExpression(node);
        case IfExpression: return this->examineIfExpression(node);
        case FunctionCallStatement: return this->examineFunctionCallStatement(node);
        case FunctionCallExpression: return this->examineFunctionCallExpression(node);
        case VariableDeclaration: return this->examineVariableDeclaration(node);
        case FunctionDeclaration: return this->examineFunctionDeclaration(node);
        case IfStatement: return this->examineIfStatement(node);
        case ExecutionList: return this->examineExecutionList(node);
        case Program: return this->examineProgram(node);
        case BlockStatement: return this->examineBlockStatement(node);
        case LoopStatement: return this->examineLoopStatement(node);
        case AssignmentStatement: return this->examineAssignmentStatement(node);
        case AssignmentExpression: return this->examineAssignmentExpression(node);

        case Invalid: // shouldn't exist
        case WhileStatement: // shouldn't exist
        case ContinueStatement:
        case BreakStatement:
        case ReturnStatement:
            return m_typeStore->getVoidType();
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
#include <memory>
#include <string>

#include "checker/checker.hpp"
#include "checker/type.hpp"
#include "common/sourcecodelocation.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"
#include "binder/symbol.hpp"

template <typename To, typename From>
std::unique_ptr<To> unique_ptr_static_cast(std::unique_ptr<From> from) {
    return std::unique_ptr<To>(static_cast<To*>(from.release()));
}

void TypeChecker::addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocationSpan> sourceCodeSpan) {
    auto span = sourceCodeSpan.has_value() ? sourceCodeSpan.value() : SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1));
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(code, kind, DiagnosticMessageStage::TypeChecker, span, this->source, message));
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

void TypeChecker::checkFunctionDeclarationReturns(Type* returnType,FunctionDeclarationNode* functionDeclarationNode) {
    auto flowGraph = functionDeclarationNode->getFlowGraph();
    std::vector<Node*> returnStatements;
    auto emptyReturnNode = new ReturnStatementNode(nullptr, SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)));
    for (auto node : flowGraph->getExit()->getPredecessors()) {
        if (!node->getAstNode()->isReachable()) {
            continue;
        }
        if (node->getAstNode()->getNodeKind() == NodeKind::ReturnStatement) {
            returnStatements.push_back(node->getAstNode());
        } else {
            returnStatements.push_back(emptyReturnNode);
        }
    }
    for (auto returnStatement : returnStatements) {
        auto returnStatementNode = static_cast<ReturnStatementNode*>(returnStatement);
        auto returnStatementType = this->examine(returnStatementNode->getExpression());
        if (returnStatementType != nullptr && !returnStatementType->isSubtypeOf(returnType)) {
            this->addError(21, "Return type is not compatible with function return type annotation.", functionDeclarationNode->getSourceCodeLocationSpan());
            break;
        } else if (returnStatementType == nullptr && returnType->getTypeKind() != TypeKind::Void) {
            this->addError(21, "Non-void function must return a value.", functionDeclarationNode->getSourceCodeLocationSpan());
            break;
        }
    }
    delete emptyReturnNode;
}

std::vector<Symbol*> TypeChecker::checkFunctionDeclarationParameters(FunctionDeclarationNode* functionDeclarationNode) {
    auto parameters = functionDeclarationNode->getParameters();
    std::vector<Symbol*> parameterSymbols;
    for (const auto& parameter : parameters) {
        auto parameterSymbol = parameter->getSymbolReference();
        auto parameterAnnotation = parameter->getAnnotation();
        if (parameterAnnotation) {
            // TODO eventually properly parse type expression type annotations instead of just primitive types. For now, if there's a type annotation, it must be a primitive type.
            auto parameterType = this->typeStore->createType<PrimitiveType>(parameterAnnotation->getPrimitiveTypeKind());
            parameterSymbol->setType(parameterType);
        } else {
            // If there's no parameter type annotation, then the parameter type is Any.
            // TODO also should eventually infer this (will be harder since code assumes types set for symbols)
            parameterSymbol->setType(this->typeStore->getAnyType());
        }
        if (parameterSymbol) {
            parameterSymbols.push_back(parameterSymbol);
        } else {
            // Shouldn't happen since binder should have created a symbol for the parameter
            // unreachable
            this->addError(20, "Parameter '" + std::string(parameter->getName()) + "' has no symbol reference", parameter->getSourceCodeLocationSpan());
        }
    }
    return parameterSymbols;
}

Type* TypeChecker::getFunctionDeclarationReturnType(FunctionDeclarationNode* functionDeclarationNode) {
    auto returnTypeExpression = functionDeclarationNode->getReturnTypeExpression();
    if (returnTypeExpression) {
        // TODO eventually type expression won't just be primitive types
        return this->typeStore->createType<PrimitiveType>(returnTypeExpression->getPrimitiveTypeKind());
    } else {
        // If there's no return type annotation, then the return type is Void.
        // TODO infer return type here eventually instead
        return this->typeStore->getVoidType();
    }
}

FunctionType* TypeChecker::getFunctionDeclarationFunctionType(FunctionDeclarationNode* functionDeclarationNode) {
    auto parameterSymbols = this->checkFunctionDeclarationParameters(functionDeclarationNode);
    auto returnType = this->getFunctionDeclarationReturnType(functionDeclarationNode);
    return this->typeStore->createType<FunctionType>(parameterSymbols, returnType);
}

Type* TypeChecker::examineIdentifier(Node* node) {
    auto symbol = static_cast<IdentifierNode*>(node)->getSymbolReference();
    if (symbol) {
        if (!symbol->getType()) {
            if (symbol->getDefiningNode() == node) {
                this->addError(51, "Symbol '" + symbol->getNameString() + "' has circular definition", static_cast<IdentifierNode*>(node)->getSourceCodeLocationSpan());
                return nullptr;
            }
            symbol->setType(this->examine(symbol->getDefiningNode()));
        }
        return symbol->getType();
    }
    this->addError(10, "Identifier with no symbol: " + std::string(static_cast<IdentifierNode*>(node)->getName()), static_cast<IdentifierNode*>(node)->getSourceCodeLocationSpan());
    return nullptr;
}

Type* TypeChecker::examineBinaryOperatorExpression(Node* node) {
    auto* binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node);
    auto leftType = this->examine(binaryOperatorExpressionNode->getLeft());
    auto rightType = this->examine(binaryOperatorExpressionNode->getRight());
    auto operatorToken = *binaryOperatorExpressionNode->getOperatorToken();
    // string concatenation
    if (operatorToken == TokenKind::Plus) {
        if (leftType->getTypeKind() == TypeKind::Primitive && rightType->getTypeKind() == TypeKind::Primitive) {
            auto leftPrimitiveType = static_cast<PrimitiveType*>(leftType);
            auto rightPrimitiveType = static_cast<PrimitiveType*>(rightType);
            if (leftPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::String && rightPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::String && operatorToken == TokenKind::Plus) {
                binaryOperatorExpressionNode->setType(this->typeStore->getStringType());
                return binaryOperatorExpressionNode->getType();
            }
        }
    } // otherwise, + is arithmetic --v
    // arithmetic operators
    if (operatorToken == TokenKind::AsteriskAsterisk || operatorToken == TokenKind::Plus || operatorToken == TokenKind::Dash || operatorToken == TokenKind::Asterisk || operatorToken == TokenKind::Slash) {
        if (leftType->getTypeKind() == TypeKind::Primitive && rightType->getTypeKind() == TypeKind::Primitive) {
            auto leftPrimitiveType = static_cast<PrimitiveType*>(leftType);
            auto rightPrimitiveType = static_cast<PrimitiveType*>(rightType);
            // TODO eventually support proper coercion (Integer -> Float). 
            // For now, if both sides are floats, the result is a float. If both sides are integers, the result is an integer.
            if (leftPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Integer && rightPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Integer) {
                binaryOperatorExpressionNode->setType(this->typeStore->getIntegerType());
                return binaryOperatorExpressionNode->getType();
            }
            if (leftPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Float && rightPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Float) {
                binaryOperatorExpressionNode->setType(this->typeStore->getFloatType());
                return binaryOperatorExpressionNode->getType();
            }
            // Fall through --v
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid types for +, -, *, / (arithmetic)
        this->addError(11, "Bad types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr; // TODO maybe return void at some point?
    }
    // logical operators
    if (operatorToken == TokenKind::And || operatorToken == TokenKind::Or) {
        if (leftType->getTypeKind() == TypeKind::Primitive && rightType->getTypeKind() == TypeKind::Primitive) {
            auto leftPrimitiveType = static_cast<PrimitiveType*>(leftType);
            auto rightPrimitiveType = static_cast<PrimitiveType*>(rightType);
            if (leftPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean && rightPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean) {
                binaryOperatorExpressionNode->setType(this->typeStore->getBooleanType());
                return binaryOperatorExpressionNode->getType();
            }
            // Fall through --v
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid types for comparison operators
        // TODO refactor error messages
        this->addError(11, "Bad types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }
    // number comparison operators
    if (operatorToken == TokenKind::LessThan || operatorToken == TokenKind::GreaterThan || operatorToken == TokenKind::LessThanEqual || operatorToken == TokenKind::GreaterThanEqual) {
        if (leftType->getTypeKind() == TypeKind::Primitive && rightType->getTypeKind() == TypeKind::Primitive) {
            auto leftPrimitiveType = static_cast<PrimitiveType*>(leftType);
            auto rightPrimitiveType = static_cast<PrimitiveType*>(rightType);
            if (IS_PRIMITIVE_TYPE_NUMERIC(leftPrimitiveType->getPrimitiveTypeKind()) && IS_PRIMITIVE_TYPE_NUMERIC(rightPrimitiveType->getPrimitiveTypeKind())) {
                binaryOperatorExpressionNode->setType(this->typeStore->getBooleanType());
                return binaryOperatorExpressionNode->getType();
            }
            // Fall through --v
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid types for comparison operators
        // TODO refactor error messages
        this->addError(11, "Bad types for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }
    // TODO eventually handle == and !=? Or just let that be determined at runtime?
    if (operatorToken == TokenKind::EqualEqual || operatorToken == TokenKind::NotEqual) {
        binaryOperatorExpressionNode->setType(this->typeStore->getBooleanType());
        return binaryOperatorExpressionNode->getType();
    }
    // TODO eventually support operator overloading, but for now only allow primitive types.
    // Unknown binary operator (still need to handle dot and equal operators).
    // TODO refactor error messages
    this->addError(12, "Unknown operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
    return nullptr; // TODO maybe return void at some point?
}

Type* TypeChecker::examineTypeExpression(Node* node) {
    auto typeExpressionNode = static_cast<TypeExpressionNode*>(node);
    // TODO eventually type expression won't just be primitive types
    auto primitiveType = this->typeStore->createType<PrimitiveType>(typeExpressionNode->getPrimitiveTypeKind());
    typeExpressionNode->setType(primitiveType);
    return primitiveType;
}

Type* TypeChecker::examineUnaryOperatorExpression(Node* node) {
    auto* unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node);
    auto operandType = this->examine(unaryOperatorExpressionNode->getOperand());
    auto operatorToken = *unaryOperatorExpressionNode->getOperatorToken();
    if (operatorToken == TokenKind::Dash) {
        if (operandType->getTypeKind() == TypeKind::Primitive) {
            auto operandPrimitiveType = static_cast<PrimitiveType*>(operandType);
            if (IS_PRIMITIVE_TYPE_NUMERIC(operandPrimitiveType->getPrimitiveTypeKind())) {
                unaryOperatorExpressionNode->setType(this->typeStore->createType<PrimitiveType>(operandPrimitiveType->getPrimitiveTypeKind()));
                return unaryOperatorExpressionNode->getType();
            }
            // Fall through --v
        }
        // TODO eventually support operator overloading, but for now only allow primitive types.
        // Invalid type for unary -
        this->addError(11, "Bad type for operator '" + std::string(operatorToken.getSourceString()) + "'", operatorToken.getSourceCodeLocationSpan());
        return nullptr;
    }
    if (operatorToken == TokenKind::Not) {
        if (operandType->getTypeKind() == TypeKind::Primitive) {
            auto operandPrimitiveType = static_cast<PrimitiveType*>(operandType);
            if (operandPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean) {
                unaryOperatorExpressionNode->setType(this->typeStore->getBooleanType());
                return unaryOperatorExpressionNode->getType();
            }
            // Fall through --v
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
    if (conditionType->getTypeKind() == TypeKind::Primitive) {
        auto conditionPrimitiveType = static_cast<PrimitiveType*>(conditionType);
        if (conditionPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean) {
            auto thenBranchType = this->examine(ifExpressionNode->getThenBranch());
            auto elseBranchType = this->examine(ifExpressionNode->getElseBranch());
            auto unionType = this->typeStore->createType<UnionType>(thenBranchType, elseBranchType);
            auto type = this->typeStore->simplifyType(unionType);
            ifExpressionNode->setType(type);
            return type;
        }
        // Fall through --v
    }
    // Invalid type for if condition
    this->addError(14, "Bad type for if condition. Expected boolean.", ifExpressionNode->getCondition()->getSourceCodeLocationSpan());
    return nullptr;
}

Type* TypeChecker::examineFunctionCallExpression(Node* node) {
    auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
    auto functionType = static_cast<FunctionType*>(functionCallExpressionNode->getIdentifier()->getSymbolReference()->getType());
    auto arguments = functionCallExpressionNode->getArgumentNodes();
    auto parameters = functionType->getParameters();
    if (arguments.size() != parameters.size()) {
        this->addError(15, "Argument count does not match parameter count in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        return functionType->getReturnType();
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        auto argumentType = this->examine(arguments[i]);
        auto parameterSymbol = parameters[i];
        auto parameterType = parameterSymbol->getType();
        if (!argumentType->isSubtypeOf(parameterType)) {
            this->addError(16, "Argument type is not compatible with parameter type in function call", functionCallExpressionNode->getSourceCodeLocationSpan());
        }
    }
    return functionType->getReturnType();
}

Type* TypeChecker::examineVariableDeclaration(Node* node) {
    auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
    auto identifierNode = variableDeclarationNode->getIdentifier();
    auto typeExpressionNode = variableDeclarationNode->getTypeExpression();
    auto expressionNode = variableDeclarationNode->getExpression();
    auto symbol = identifierNode->getSymbolReference();
    Type* expressionType = nullptr;
    Type* computedType = nullptr;
    if (expressionNode) {
        expressionType = this->examine(expressionNode);
    } else {
        // if there's no initializer expression, then the type annotation is required, and Empty must be assignable to it.
        expressionType = this->typeStore->getEmptyType();
    }
    if (!expressionType) {
        // if the expression type is null, then there was an error examining the expression, so just return void to avoid spamming errors about incompatible types
        return this->typeStore->getVoidType();
    }
    if (typeExpressionNode) {
        // Set type to what is annotated
        auto typeExpressionType = this->typeStore->createType<PrimitiveType>(typeExpressionNode->getPrimitiveTypeKind());
        if (!expressionType->isSubtypeOf(typeExpressionType)) {
            this->addError(17, "Type annotation is not compatible with initializer expression type", identifierNode->getSourceCodeLocationSpan());
        }
        if (!expressionType->isSubtypeOf(typeExpressionType)) {
            this->addError(18, "Initializer expression type is not compatible with type annotation", identifierNode->getSourceCodeLocationSpan());
        }
        computedType = typeExpressionType;
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
        this->addError(20, "Variable declaration identifier has no symbol reference", identifierNode->getSourceCodeLocationSpan());
    }
    return this->typeStore->getVoidType();
}

Type* TypeChecker::examineFunctionDeclaration(Node* node) {
    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
    auto symbol = functionDeclarationNode->getIdentifier()->getSymbolReference();
    auto functionType = this->getFunctionDeclarationFunctionType(functionDeclarationNode);
    if (symbol) {
        symbol->setType(functionType);
    } else {
        // Shouldn't happen since binder should have created a symbol for the function declaration
        this->addError(20, "Function declaration identifier has no symbol reference", functionDeclarationNode->getIdentifier()->getSourceCodeLocationSpan());
    }
    // Examine body
    this->examine(functionDeclarationNode->getBody());
    // Check function returns after body:
    checkFunctionDeclarationReturns(functionType->getReturnType(), functionDeclarationNode);
    return this->typeStore->getVoidType();
}

Type* TypeChecker::examineIfStatement(Node* node) {
    auto ifStatementNode = static_cast<IfStatementNode*>(node);
    auto conditionType = this->examine(ifStatementNode->getCondition());
    if (conditionType->getTypeKind() == TypeKind::Primitive) {
        auto conditionPrimitiveType = static_cast<PrimitiveType*>(conditionType);
        if (conditionPrimitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean) {
            this->examine(ifStatementNode->getThenBranch());
            if (ifStatementNode->getElseBranch()) {
                this->examine(ifStatementNode->getElseBranch());
            }
        } else {
            this->addError(14, "Bad type for if condition. Expected Boolean.", ifStatementNode->getCondition()->getSourceCodeLocationSpan());
        }
    } else {
        this->addError(14, "Bad type for if condition. Expected Boolean.", ifStatementNode->getCondition()->getSourceCodeLocationSpan());
    }
    return this->typeStore->getVoidType();
}

Type* TypeChecker::examine(Node* node) {
    if (!node) {
        return nullptr;
    }
    switch (node->getNodeKind()) {
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::Identifier: return this->examineIdentifier(node);
        case NodeKind::TypeExpression: return this->examineTypeExpression(node);
        case NodeKind::NumberLiteral: {
            // TODO eventually distinguish between integers and floats. Probably get rid of Number primitive type.
            node->setType(this->typeStore->getIntegerType());
            return node->getType();
        }
        case NodeKind::StringLiteral: {
            node->setType(this->typeStore->getStringType());
            return node->getType();
        }
        case NodeKind::BooleanLiteral: {
            node->setType(this->typeStore->getBooleanType());
            return node->getType();
        }
        case NodeKind::EmptyLiteral: {
            node->setType(this->typeStore->getEmptyType());
            return node->getType();
        }
        case NodeKind::BinaryOperatorExpression: return this->examineBinaryOperatorExpression(node);
        case NodeKind::UnaryOperatorExpression: return this->examineUnaryOperatorExpression(node);
        case NodeKind::IfExpression: return this->examineIfExpression(node);
        case NodeKind::FunctionCallStatement: {
            auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
            auto functionCallExpressionNode = functionCallStatementNode->getFunctionCallExpression();
            this->examine(functionCallExpressionNode);
            return this->typeStore->getVoidType();
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
            return this->typeStore->getVoidType();
        }
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            this->examine(programNode->getExecutionListNode());
            return this->typeStore->getVoidType();
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            this->examine(blockStatementNode->getExecutionListNode());
            return this->typeStore->getVoidType();
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            this->examine(loopStatementNode->getBody());
            return this->typeStore->getVoidType();
        }
        case NodeKind::ReturnStatement:
        case NodeKind::Invalid: // shouldn't exist
        case NodeKind::ContinueStatement:
        case NodeKind::BreakStatement:
        case NodeKind::WhileStatement: // shouldn't exist
        {
            return this->typeStore->getVoidType();
        }
        case NodeKind::AssignmentStatement: {
            auto assignmentStatementNode = static_cast<AssignmentStatementNode*>(node);
            this->examine(assignmentStatementNode->getAssignmentExpression());
            return this->typeStore->getVoidType();
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

void TypeChecker::typeCheck(Node* rootNode) {
    this->examine(rootNode);
}
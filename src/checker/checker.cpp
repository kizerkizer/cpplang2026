#include <memory>

#include "checker/checker.hpp"
#include "checker/type.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"
#include "binder/symbol.hpp"

template <typename To, typename From>
std::unique_ptr<To> unique_ptr_static_cast(std::unique_ptr<From> from) {
    return std::unique_ptr<To>(static_cast<To*>(from.release()));
}

void TypeChecker::addErrorMessage(const std::string errorMessage) {
    this->errorMessages.push_back(errorMessage);
}

Type* TypeChecker::examine(Node* node) {
    if (!node) {
        return nullptr;
    }
    switch (node->getNodeKind()) {
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::Identifier: {
            auto symbol = static_cast<IdentifierNode*>(node)->getSymbolReference();
            if (symbol) {
                if (!symbol->getType()) {
                    symbol->setType(this->examine(symbol->getNode()));
                }
                return symbol->getType();
            }
            this->addErrorMessage("Identifier with no symbol: " + static_cast<IdentifierNode*>(node)->getName() + "");
            return nullptr;
        }
        case NodeKind::TypeExpression: {
            auto typeExpressionNode = static_cast<TypeExpressionNode*>(node);
            auto primitiveType = this->typeStore->createType<PrimitiveType>(typeExpressionNode->getPrimitiveTypeKind());
            typeExpressionNode->setType(primitiveType);
            return primitiveType;
        }
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
        case NodeKind::BinaryOperatorExpression: {
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
                this->addErrorMessage("Bad types for operator '" + operatorToken.getSourceString() + "'");
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
                this->addErrorMessage("Bad types for operator '" + operatorToken.getSourceString() + "'");
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
                this->addErrorMessage("Bad types for operator '" + operatorToken.getSourceString() + "'");
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
            this->addErrorMessage("Unknown operator '" + operatorToken.getSourceString() + "'");
            return nullptr; // TODO maybe return void at some point?
        }
        case NodeKind::UnaryOperatorExpression: {
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
                this->addErrorMessage("Bad type for operator '" + operatorToken.getSourceString() + "'");
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
                this->addErrorMessage("Bad type for operator '" + operatorToken.getSourceString() + "'");
                return nullptr;
            }
            this->addErrorMessage("Unknown operator '" + operatorToken.getSourceString() + "'");
            return nullptr; // TODO maybe return void at some point?
        }
        case NodeKind::IfExpression: {
            auto* ifExpressionNode = static_cast<IfExpressionNode*>(node);
            auto conditionType = this->examine(ifExpressionNode->getCondition());
            if (!(ifExpressionNode->getThenBranch() && ifExpressionNode->getElseBranch())) {
                this->addErrorMessage("If expressions must have both then and else branches");
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
            this->addErrorMessage("Bad type for if condition. Expected boolean.");
            return nullptr;
        }
        case NodeKind::FunctionCallStatement: {
            auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
            auto functionCallExpressionNode = functionCallStatementNode->getFunctionCallExpression();
            this->examine(functionCallExpressionNode);
            return this->typeStore->getVoidType();
        }
        case NodeKind::FunctionCallExpression: {
            auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
            auto functionType = static_cast<FunctionType*>(functionCallExpressionNode->getIdentifier()->getSymbolReference()->getType());
            //auto returnType = functionType->getReturnType();
            auto arguments = functionCallExpressionNode->getArgumentNodes();
            auto parameters = functionType->getParameters();
            if (arguments.size() != parameters.size()) {
                this->addErrorMessage("Argument count does not match parameter count in function call");
                return functionType->getReturnType();
            }
            for (size_t i = 0; i < arguments.size(); i++) {
                auto argumentType = this->examine(arguments[i]);
                auto parameterSymbol = parameters[i];
                auto parameterType = parameterSymbol->getType();
                if (!argumentType->isSubtypeOf(parameterType)) {
                    // TODO refactor this mess and all error message construction.
                    std::string message = "Argument type is not compatible with parameter type in function call. ";
                    message += "\nArgument type: " + typeKindToString(argumentType->getTypeKind());
                    if (argumentType->getTypeKind() == TypeKind::Primitive) {
                        auto argumentPrimitiveType = static_cast<PrimitiveType*>(argumentType);
                        message += " (" + primitiveTypeToString(argumentPrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    message += "\nParameter type: " + typeKindToString(parameterType->getTypeKind());
                    if (parameterType->getTypeKind() == TypeKind::Primitive) {
                        auto parameterPrimitiveType = static_cast<PrimitiveType*>(parameterType);
                        message += " (" + primitiveTypeToString(parameterPrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    message += "\nLine " + std::to_string(functionCallExpressionNode->getIdentifier()->getIdentifierToken()->getFirstLine());
                    this->addErrorMessage(message);
                }
            }
            return functionType->getReturnType();
        }
        case NodeKind::VariableDeclaration: {
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
            if (typeExpressionNode) {
                // Set type to what is annotated
                auto typeExpressionType = this->typeStore->createType<PrimitiveType>(typeExpressionNode->getPrimitiveTypeKind());
                if (!expressionType->isSubtypeOf(typeExpressionType)) {
                    // TODO refactor this mess and all error message construction.
                    std::string message = "Type annotation is not compatible with initializer expression type. ";
                    message += "\nType annotation: " + typeKindToString(typeExpressionType->getTypeKind());
                    if (typeExpressionType->getTypeKind() == TypeKind::Primitive) {
                        auto typeExpressionPrimitiveType = static_cast<PrimitiveType*>(typeExpressionType);
                        message += " (" + primitiveTypeToString(typeExpressionPrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    message += "\nInitializer expression type: " + typeKindToString(expressionType->getTypeKind());
                    if (expressionType->getTypeKind() == TypeKind::Primitive) {
                        auto expressionPrimitiveType = static_cast<PrimitiveType*>(expressionType);
                        message += " (" + primitiveTypeToString(expressionPrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    auto line = variableDeclarationNode->getIdentifier()->getIdentifierToken()->getFirstLine();
                    message += "\nLine " + std::to_string(line);
                    this->addErrorMessage(message);
                }
                if (!expressionType->isSubtypeOf(typeExpressionType)) {
                    this->addErrorMessage("Initializer expression type is not compatible with type annotation");
                }
                computedType = typeExpressionType;
            } else if (!expressionNode) {
                this->addErrorMessage("Variable declaration with no initiazlier must have a type annotation that is a supertype of Empty");
                computedType = expressionType;
            } else {
                // Infer type as type of expression
                computedType = expressionType;
            }
            symbol->setType(computedType);
            return this->typeStore->getVoidType();
        }
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            auto returnTypeExpression = functionDeclarationNode->getReturnTypeExpression();
            auto symbol = functionDeclarationNode->getIdentifier()->getSymbolReference();
            Type* returnType = nullptr;
            if (returnTypeExpression) {
                // TODO eventually type expression won't just be primitive types
                returnType = this->typeStore->createType<PrimitiveType>(returnTypeExpression->getPrimitiveTypeKind());
            } else {
                // If there's no return type annotation, then the return type is Void.
                returnType = this->typeStore->getVoidType();
            }
            // Examine parameters
            auto parameters = functionDeclarationNode->getParameters();
            std::vector<Symbol*> parameterSymbols;
            for (const auto& parameter : parameters) {
                auto parameterIdentifierNode = parameter;
                auto parameterSymbol = parameterIdentifierNode->getSymbolReference();
                auto parameterAnnotation = parameter->getAnnotation();
                if (parameterAnnotation) {
                    auto parameterType = this->typeStore->createType<PrimitiveType>(parameterAnnotation->getPrimitiveTypeKind());
                    parameterSymbol->setType(parameterType);
                } else {
                    // If there's no parameter type annotation, then the parameter type is Any.
                    parameterSymbol->setType(this->typeStore->getAnyType());
                }
                if (parameterSymbol) {
                    parameterSymbols.push_back(parameterSymbol);
                } else {
                    // Shouldn't happen since binder should have created a symbol for the parameter
                    this->addErrorMessage("Parameter '" + parameterIdentifierNode->getName() + "' has no symbol reference");
                }
            }
            // check that the return type annotation is compatible with the types of the expressions in return statements in the function body:
            auto flowGraph = functionDeclarationNode->getFlowGraph();
            std::vector<Node*> returnStatements;
            //auto emptyReturnNode = std::make_unique<ReturnStatementNode>(nullptr);
            auto emptyReturnNode = new ReturnStatementNode(nullptr);
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
                    std::string message = "Return type is not compatible with function return type annotation. ";
                    message += "\nReturn type annotation: " + typeKindToString(returnType->getTypeKind());
                    if (returnType->getTypeKind() == TypeKind::Primitive) {
                        auto returnTypePrimitiveType = static_cast<PrimitiveType*>(returnType);
                        message += " (" + primitiveTypeToString(returnTypePrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    message += "\nReturn statement type: " + typeKindToString(returnStatementType->getTypeKind());
                    if (returnStatementType->getTypeKind() == TypeKind::Primitive) {
                        auto returnStatementPrimitiveType = static_cast<PrimitiveType*>(returnStatementType);
                        message += " (" + primitiveTypeToString(returnStatementPrimitiveType->getPrimitiveTypeKind()) + ")";
                    }
                    auto [index, line, column] = returnStatement->getFirstSourceCodeLocation();
                    message += "\nLine " + std::to_string(line);
                    this->addErrorMessage(message);
                } else if (returnStatementType == nullptr && returnType->getTypeKind() != TypeKind::Void) {
                    std::string message = "Non-void function must return a value.";
                    auto [index, line, column] = functionDeclarationNode->getIdentifier()->getIdentifierToken()->getFirstSourceCodeLocation();
                    message += "\nLine " + std::to_string(line);
                    this->addErrorMessage(message);
                }
            }
            delete emptyReturnNode;
            // Examine body
            this->examine(functionDeclarationNode->getBody());
            auto functionType = this->typeStore->createType<FunctionType>(parameterSymbols, returnType);
            symbol->setType(functionType);
            return this->typeStore->getVoidType();
        }
        case NodeKind::IfStatement: {
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
                    this->addErrorMessage("Bad type for if condition. Expected Boolean.");
                }
            } else {
                this->addErrorMessage("Bad type for if condition. Expected Boolean.");
            }
            return this->typeStore->getVoidType();
        }
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            for (auto& child : programNode->getChildren()) {
                this->examine(child);
            }
            return this->typeStore->getVoidType();
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            this->examine(blockStatementNode->getProgramNode());
            return this->typeStore->getVoidType();
        }
        case NodeKind::LoopStatement:
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
                this->addErrorMessage("Type of right-hand side of assignment is not compatible with type of left-hand side");
            }
            return expressionType;
        }
    }
}

void TypeChecker::typeCheck(Node* rootNode) {
    this->examine(rootNode);
}
#include <print>

#include "treewalker/interpreter.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"
#include "treewalker/environment.hpp"
#include "treewalker/value.hpp"

// Interpreter
Interpreter::Interpreter(OutputStream* outputStream) : m_outputStream(outputStream) {
    this->m_valueStore = std::make_unique<ValueStore>();
}

Value* Interpreter::interpretIdentifier(IdentifierNode* identifierNode, Environment* environment) {
    return environment->getVar(std::string(identifierNode->getName()));
}

Value* Interpreter::interpret(Node* rootNode) {
    auto globalEnvironment = std::make_unique<Environment>(nullptr, rootNode);
    auto globalEnvironmentPtr = globalEnvironment.get();
    this->m_globalEnvironment = std::move(globalEnvironment);
    return _interpret(rootNode, globalEnvironmentPtr);
}

Value* Interpreter::_interpret(Node* node, Environment* environment) {
    switch (node->getNodeKind()) {
        case NodeKind::ExecutionList: {
            Value* resultingValue = this->m_valueStore->makeVoidValue();
            for (auto child : node->getChildren()) {
                auto value = _interpret(child, environment);
                if (value->getKind() == ValueKind::Return) {
                    return value;
                }
                if (value->getKind() == ValueKind::Break || value->getKind() == ValueKind::Continue) {
                    // Must be inside loop since that was verified by binder
                    return value;
                }
                if (value->getKind() == ValueKind::Function && this->m_globalEnvironment.get() == environment) {
                    auto functionValue = static_cast<FunctionValue*>(value);
                    auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(functionValue->getNode());
                    if (functionDeclarationNode->getIdentifier()->getName() == "main") {
                        // Call main function immediately
                        this->_interpret(functionDeclarationNode, environment);
                        auto mainFunctionDefiningEnvironment = functionValue->getDefiningEnvironment();
                        auto mainFunctionEnvironment = std::make_unique<Environment>(mainFunctionDefiningEnvironment, functionDeclarationNode);
                        auto mainFunctionEnvironmentPtr = mainFunctionEnvironment.get();
                        mainFunctionDefiningEnvironment->addChildEnvironment(std::move(mainFunctionEnvironment));
                        //resultingValue = _interpret(functionDeclarationNode->getBody(), mainFunctionEnvironmentPtr);
                        auto mainFunctionReturnValue = _interpret(functionDeclarationNode->getBody()->getExecutionListNode(), mainFunctionEnvironmentPtr);
                        if (mainFunctionReturnValue->getKind() == ValueKind::Return) {
                            auto mainFunctionReturnValueCast = static_cast<ReturnValue*>(mainFunctionReturnValue);
                            return mainFunctionReturnValueCast->getValue();
                        } else {
                            // shouldn't happen
                            std::print("Main function return value kind is {}\n", valueKindToString(mainFunctionReturnValue->getKind()));
                            return this->m_valueStore->makeVoidValue();
                        }
                    }
                }
            }
            return resultingValue;

        }
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            return _interpret(programNode->getExecutionListNode(), environment);
        }
        case NodeKind::FunctionCallExpression: {
            auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
            auto arguments = functionCallExpressionNode->getArgumentNodes();
            auto name = functionCallExpressionNode->getIdentifier()->getName();
            auto value = environment->getVar(std::string(name));
            if (!value) {
                // unreachable; handled in type checker
                std::print("Error: Function '{}' not found\n", name);
                return this->m_valueStore->makeVoidValue();
            }
            if (value->getKind() != ValueKind::Function) {
                // unreachable; handled in type checker
                std::print("Error: Attempted to call non-function value '{}'\n", name);
                return this->m_valueStore->makeVoidValue();
            }
            auto functionValue = static_cast<FunctionValue*>(value);
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(functionValue->getNode());
            auto functionDefiningEnvironment = functionValue->getDefiningEnvironment();
            // TODO delete these --v
            if (functionDeclarationNode->getIdentifier()->getName() == "log") {
                auto argumentValue = _interpret(arguments[0], environment);
                auto stringArgumentValue = static_cast<StringValue*>(argumentValue);
                this->m_outputStream->println(std::string(stringArgumentValue->getValue()));
                return this->m_valueStore->makeVoidValue();
            }
            if (functionDeclarationNode->getIdentifier()->getName() == "logi") {
                auto argumentValue = _interpret(arguments[0], environment);
                auto integerArgumentValue = static_cast<IntegerValue*>(argumentValue);
                this->m_outputStream->println(std::to_string(integerArgumentValue->getValue()));
                return this->m_valueStore->makeVoidValue();
            }
            // TODO delete these --^
            auto functionEnvironment = std::make_unique<Environment>(functionDefiningEnvironment, functionDeclarationNode);
            auto functionEnvironmentPtr = functionEnvironment.get();
            functionDefiningEnvironment->addChildEnvironment(std::move(functionEnvironment));
            auto parameters = functionDeclarationNode->getParameters();
            for (size_t i = 0; i < parameters.size(); i++) {
                auto parameter = parameters[i];
                auto argumentValue = _interpret(arguments[i], environment);
                functionEnvironmentPtr->defineVar(std::string(parameter->getName()), argumentValue);
            }
            auto result = _interpret(functionDeclarationNode->getBody()->getExecutionListNode(), functionEnvironmentPtr);
            if (result->getKind() == ValueKind::Return) {
                auto returnValue = static_cast<ReturnValue*>(result);
                return returnValue->getValue();
            } else {
                std::print("FunctionCallExpression return value kind is {}\n", valueKindToString(result->getKind()));
                return this->m_valueStore->makeVoidValue();
            }
        }
        case NodeKind::FunctionCallStatement: {
            auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
            _interpret(functionCallStatementNode->getFunctionCallExpression(), environment);
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::VariableDeclaration: {
            auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
            // TODO handle type annotation [?] or not?
            auto value = _interpret(variableDeclarationNode->getExpression(), environment);
            environment->defineVar(std::string(variableDeclarationNode->getIdentifier()->getName()), value);
            break;
        }
        case NodeKind::WhileStatement: {
            // unreachable; desugared to loop statement
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::Invalid: {
            std::print("Error: Invalid node encountered during interpretation\n");
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::TypeExpression: {
            // irrelevant? only used by type checker?
            // TODO ?
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            auto blockEnvironment = std::make_unique<Environment>(environment, blockStatementNode);
            auto blockEnvironmentPtr = blockEnvironment.get();
            environment->addChildEnvironment(std::move(blockEnvironment));
            return _interpret(blockStatementNode->getExecutionListNode(), blockEnvironmentPtr);
        }
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            auto value = this->m_valueStore->makeFunctionValue(functionDeclarationNode, environment);
            environment->defineVar(std::string(functionDeclarationNode->getIdentifier()->getName()), value);
            return value;
        }
        case NodeKind::AssignmentExpression: {
            auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
            auto value = _interpret(assignmentExpressionNode->getExpression(), environment);
            environment->setVar(std::string(assignmentExpressionNode->getIdentifier()->getName()), value);
            return value;
        }
        case NodeKind::AssignmentStatement: {
            auto assignmentStatementNode = static_cast<AssignmentStatementNode*>(node);
            _interpret(assignmentStatementNode->getAssignmentExpression(), environment);
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::Identifier:
        case NodeKind::IdentifierWithPossibleAnnotation: {
            auto identifierNode = static_cast<IdentifierNode*>(node);
            return interpretIdentifier(identifierNode, environment);
        }
        case NodeKind::NumberLiteral: {
            auto numberLiteralNode = static_cast<NumberLiteralNode*>(node);
            auto value = this->m_valueStore->makeIntegerValue(numberLiteralNode->getValue());
            // TODO floats
            return value;
        }
        case NodeKind::StringLiteral: {
            auto stringLiteralNode = static_cast<StringLiteralNode*>(node);
            auto value = this->m_valueStore->makeStringValue(stringLiteralNode->getValue());
            return value;
        }
        case NodeKind::EmptyLiteral: {
            return this->m_valueStore->makeEmptyValue();
        }
        case NodeKind::UnaryOperatorExpression: {
            auto unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node);
            auto operandValue = _interpret(unaryOperatorExpressionNode->getOperand(), environment);
            if (unaryOperatorExpressionNode->getOperatorToken()->getTokenKind() == TokenKind::Dash) {
                if (operandValue->getKind() == ValueKind::Integer) {
                    auto integerOperandValue = static_cast<IntegerValue*>(operandValue);
                    return this->m_valueStore->makeIntegerValue(-integerOperandValue->getValue());
                } else if (operandValue->getKind() == ValueKind::Float) {
                    auto floatOperandValue = static_cast<FloatValue*>(operandValue);
                    return this->m_valueStore->makeFloatValue(-floatOperandValue->getValue());
                } else {
                    // unreachable
                    std::print("Error: Unary operator '-' applied to non-numeric value\n");
                    return this->m_valueStore->makeVoidValue();
                }
            }
            if (unaryOperatorExpressionNode->getOperatorToken()->getTokenKind() == TokenKind::Not) {
                if (operandValue->getKind() == ValueKind::Boolean) {
                    auto booleanOperandValue = static_cast<BooleanValue*>(operandValue);
                    return this->m_valueStore->makeBooleanValue(!booleanOperandValue->getValue());
                } else {
                    // unreachable
                    std::print("Error: Unary operator '!' applied to non-boolean value\n");
                    std::print("{}\n", valueKindToString(operandValue->getKind()));
                    return this->m_valueStore->makeVoidValue();
                }
            }
            return operandValue;
        }
        case NodeKind::BinaryOperatorExpression: {
            auto binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node);
            auto leftValue = _interpret(binaryOperatorExpressionNode->getLeft(), environment);
            auto rightValue = _interpret(binaryOperatorExpressionNode->getRight(), environment);
            auto tokenKind = binaryOperatorExpressionNode->getOperatorToken()->getTokenKind();
            // string concatentation --v
            if (tokenKind == TokenKind::Plus && leftValue->getKind() == ValueKind::String && rightValue->getKind() == ValueKind::String) {
                auto leftStringValue = static_cast<StringValue*>(leftValue);
                auto rightStringValue = static_cast<StringValue*>(rightValue);
                return this->m_valueStore->makeStringValue(std::string(leftStringValue->getValue()) + std::string(rightStringValue->getValue()));
            }
            // arithmetic --v
            if (tokenKind == TokenKind::Plus || tokenKind == TokenKind::Dash || tokenKind == TokenKind::Asterisk || tokenKind == TokenKind::Slash || tokenKind == TokenKind::AsteriskAsterisk) {
                if (leftValue->getKind() == ValueKind::Integer && rightValue->getKind() == ValueKind::Integer) {
                    auto leftIntegerValue = static_cast<IntegerValue*>(leftValue);
                    auto rightIntegerValue = static_cast<IntegerValue*>(rightValue);
                    return this->m_valueStore->makeIntegerValue(PERFORM_INTEGER_OP(tokenKind, leftIntegerValue->getValue(), rightIntegerValue->getValue()));
                } else {
                    // TODO floats
                    // unreachable
                    std::print("Error: Binary operator applied to incompatible types\n");
                    std::print("Left value kind: {}, Right value kind: {}\n", valueKindToString(leftValue->getKind()), valueKindToString(rightValue->getKind()));
                    return this->m_valueStore->makeVoidValue();
                }
            }
            // equal/not equal --v
            if (tokenKind == TokenKind::EqualEqual || tokenKind == TokenKind::NotEqual) {
                if (leftValue->getKind() == ValueKind::Integer && rightValue->getKind() == ValueKind::Integer) {
                    auto leftIntegerValue = static_cast<IntegerValue*>(leftValue);
                    auto rightIntegerValue = static_cast<IntegerValue*>(rightValue);
                    return this->m_valueStore->makeBooleanValue(PERFORM_EQUALITY_OP(tokenKind, leftIntegerValue->getValue(), rightIntegerValue->getValue()));
                } else if (leftValue->getKind() == ValueKind::String && rightValue->getKind() == ValueKind::String) {
                    auto leftStringValue = static_cast<StringValue*>(leftValue);
                    auto rightStringValue = static_cast<StringValue*>(rightValue);
                    return this->m_valueStore->makeBooleanValue(PERFORM_EQUALITY_OP(tokenKind, leftStringValue->getValue(), rightStringValue->getValue()));
                } else if (leftValue->getKind() == ValueKind::Boolean && rightValue->getKind() == ValueKind::Boolean) {
                    auto leftBooleanValue = static_cast<BooleanValue*>(leftValue);
                    auto rightBooleanValue = static_cast<BooleanValue*>(rightValue);
                    return this->m_valueStore->makeBooleanValue(PERFORM_EQUALITY_OP(tokenKind, leftBooleanValue->getValue(), rightBooleanValue->getValue()));
                } else if (leftValue->getKind() == ValueKind::Empty && rightValue->getKind() == ValueKind::Empty) {
                    return this->m_valueStore->makeBooleanValue(true);
                } else if (leftValue->getKind() == ValueKind::Void && rightValue->getKind() == ValueKind::Void) {
                    std::print("Error: Binary operator '==' applied to void values\n"); // TODO
                    return this->m_valueStore->makeBooleanValue(false);
                } else if (leftValue->getKind() == ValueKind::Function && rightValue->getKind() == ValueKind::Function) {
                    auto leftFunctionValue = static_cast<FunctionValue*>(leftValue);
                    auto rightFunctionValue = static_cast<FunctionValue*>(rightValue);
                    // For now, consider functions equal if they point to same AST node
                    // TODO check if pointer equality works here
                    return this->m_valueStore->makeBooleanValue(leftFunctionValue->getNode() == rightFunctionValue->getNode());
                } else {
                    // unreachable
                    std::print("[BUG] Error: Binary operator '==' applied to incompatible types\n");
                    return this->m_valueStore->makeVoidValue();
                }
            }
            // integer relational ops --v
            if (tokenKind == TokenKind::LessThan || tokenKind == TokenKind::LessThanEqual || tokenKind == TokenKind::GreaterThan || tokenKind == TokenKind::GreaterThanEqual) {
                if (leftValue->getKind() == ValueKind::Integer && rightValue->getKind() == ValueKind::Integer) {
                    auto leftIntegerValue = static_cast<IntegerValue*>(leftValue);
                    auto rightIntegerValue = static_cast<IntegerValue*>(rightValue);
                    return this->m_valueStore->makeBooleanValue(PERFORM_RELATIONAL_OP(tokenKind, leftIntegerValue->getValue(), rightIntegerValue->getValue()));
                } else {
                    // unreachable
                    std::print("[BUG] Error: Binary relational operator applied to incompatible types\n");
                    std::print("Left value kind: {}, Right value kind: {}\n", valueKindToString(leftValue->getKind()), valueKindToString(rightValue->getKind()));
                    return this->m_valueStore->makeVoidValue();
                }
            }
            return leftValue;
        }
        case NodeKind::BooleanLiteral: {
            auto booleanLiteralNode = static_cast<BooleanLiteralNode*>(node);
            auto value = this->m_valueStore->makeBooleanValue(booleanLiteralNode->getValue());
            return value;
        }
        case NodeKind::IfStatement: {
            auto ifStatementNode = static_cast<IfStatementNode*>(node);
            auto conditionValue = _interpret(ifStatementNode->getCondition(), environment);
            auto conditionBooleanValue = static_cast<BooleanValue*>(conditionValue);
            if (conditionBooleanValue->getValue()) {
                return _interpret(ifStatementNode->getThenBranch(), environment);
            } else if (ifStatementNode->getElseBranch()) {
                return _interpret(ifStatementNode->getElseBranch(), environment);
            }
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            /*auto loopEnvironment = std::make_unique<Environment>(environment, loopStatementNode);
            auto loopEnvironmentPtr = loopEnvironment.get();*/
            while (true) {
                auto loopResult = _interpret(loopStatementNode->getBody(), environment);
                if (loopResult->getKind() == ValueKind::Break) {
                    break;
                } else if (loopResult->getKind() == ValueKind::Continue) {
                    continue;
                } else if (loopResult->getKind() == ValueKind::Void) { // last statement executed
                    continue;
                }
                break; // TODO ?
            }
            // TODO
            return this->m_valueStore->makeVoidValue();
        }
        case NodeKind::IfExpression: {
            auto ifExpressionNode = static_cast<IfExpressionNode*>(node);
            auto conditionValue = _interpret(ifExpressionNode->getCondition(), environment);
            auto conditionBooleanValue = static_cast<BooleanValue*>(conditionValue);
            if (conditionBooleanValue->getValue()) {
                return _interpret(ifExpressionNode->getThenBranch(), environment);
            } else {
                return _interpret(ifExpressionNode->getElseBranch(), environment);
            }
        }
        case NodeKind::ReturnStatement: {
            auto returnStatementNode = static_cast<ReturnStatementNode*>(node);
            Value* value = nullptr;
            if (returnStatementNode->getExpression()) {
                value = _interpret(returnStatementNode->getExpression(), environment);
            } else {
                value = this->m_valueStore->makeVoidValue();
            }
            return this->m_valueStore->makeReturnValue(value);
        }
        case NodeKind::BreakStatement: {
            return this->m_valueStore->makeBreakValue();
        }
        case NodeKind::ContinueStatement: {
            return this->m_valueStore->makeContinueValue();
        }
    }
    return this->m_valueStore->makeVoidValue();
}
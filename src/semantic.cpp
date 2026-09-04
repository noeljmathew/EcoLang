#include "semantic.h"

#include <string>

bool SymbolTable::declare(const std::string& name, ValueType type, int line) {
    if (symbols.find(name) != symbols.end()) {
        return false;
    }

    symbols[name] = Symbol{name, type, line};
    return true;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        return nullptr;
    }

    return &it->second;
}

void SemanticAnalyzer::analyze(const Program& program) {
    collectFunctionSignatures(program);
    requireMainFunction();

    for (const std::unique_ptr<FunctionDeclaration>& function : program.functions) {
        analyzeFunction(function.get());
    }
}

void SemanticAnalyzer::collectFunctionSignatures(const Program& program) {
    for (const std::unique_ptr<FunctionDeclaration>& function : program.functions) {
        FunctionSignature signature{function->returnType,
                                    static_cast<int>(function->parameters.size()), function->line};
        if (!functions.insert({function->name, signature}).second) {
            throw error(function->line, "function '" + function->name + "' is already declared.");
        }
    }
}

void SemanticAnalyzer::requireMainFunction() const {
    auto it = functions.find("main");
    if (it == functions.end()) {
        throw std::runtime_error(
            "Semantic error: program must define a function named 'main' "
            "that takes no parameters and returns int.");
    }

    if (it->second.returnType != ReturnType::Int || it->second.parameterCount != 0) {
        throw error(it->second.declaredLine, "'main' must take no parameters and return int.");
    }
}

void SemanticAnalyzer::analyzeFunction(const FunctionDeclaration* function) {
    symbols = SymbolTable{};
    currentFunctionReturnType = function->returnType;

    for (const std::string& parameter : function->parameters) {
        if (!symbols.declare(parameter, ValueType::Int, function->line)) {
            throw error(function->line, "parameter '" + parameter + "' is already declared.");
        }
    }

    analyzeBlock(function->body.get());
}

void SemanticAnalyzer::analyzeBlock(const BlockStatement* block) {
    for (const std::unique_ptr<Statement>& statement : block->statements) {
        analyzeStatement(statement.get());
    }
}

void SemanticAnalyzer::analyzeStatement(const Statement* statement) {
    if (const VariableDeclaration* declaration = dynamic_cast<const VariableDeclaration*>(statement)) {
        analyzeDeclaration(declaration);
    } else if (const AssignmentStatement* assignment =
                   dynamic_cast<const AssignmentStatement*>(statement)) {
        analyzeAssignment(assignment);
    } else if (const PrintfStatement* printfStatement =
                   dynamic_cast<const PrintfStatement*>(statement)) {
        analyzePrintf(printfStatement);
    } else if (const ReturnStatement* returnStatement =
                   dynamic_cast<const ReturnStatement*>(statement)) {
        analyzeReturn(returnStatement);
    } else if (const IfStatement* ifStatement = dynamic_cast<const IfStatement*>(statement)) {
        analyzeIf(ifStatement);
    } else if (const WhileStatement* whileStatement =
                   dynamic_cast<const WhileStatement*>(statement)) {
        analyzeWhile(whileStatement);
    } else if (const ForStatement* forStatement = dynamic_cast<const ForStatement*>(statement)) {
        analyzeFor(forStatement);
    } else if (const CallStatement* callStatement = dynamic_cast<const CallStatement*>(statement)) {
        analyzeCallStatement(callStatement);
    } else if (const BlockStatement* block = dynamic_cast<const BlockStatement*>(statement)) {
        analyzeBlock(block);
    }
}

void SemanticAnalyzer::analyzeDeclaration(const VariableDeclaration* declaration) {
    if (declaration->initializer != nullptr) {
        typeOfExpression(declaration->initializer.get());
    }

    if (!symbols.declare(declaration->name, ValueType::Int, declaration->line)) {
        throw error(declaration->line,
                    "variable '" + declaration->name + "' is already declared.");
    }
}

void SemanticAnalyzer::analyzeAssignment(const AssignmentStatement* assignment) {
    typeOfExpression(assignment->value.get());

    if (symbols.lookup(assignment->name) == nullptr) {
        throw error(assignment->line, "variable '" + assignment->name + "' is not declared.");
    }
}

void SemanticAnalyzer::analyzePrintf(const PrintfStatement* printfStatement) {
    for (const std::unique_ptr<Expression>& argument : printfStatement->arguments) {
        typeOfExpression(argument.get());
    }
}

void SemanticAnalyzer::analyzeReturn(const ReturnStatement* returnStatement) {
    if (returnStatement->value != nullptr) {
        if (currentFunctionReturnType == ReturnType::Void) {
            throw error(returnStatement->line, "void function cannot return a value.");
        }
        typeOfExpression(returnStatement->value.get());
    }
    // A bare "return;" in an int function is allowed without further
    // checking - this project does not verify that every code path
    // returns a value (a documented simplification, not an oversight).
}

void SemanticAnalyzer::analyzeIf(const IfStatement* ifStatement) {
    checkCondition(ifStatement->condition.get());
    analyzeBlock(ifStatement->thenBranch.get());

    if (ifStatement->elseBranch != nullptr) {
        analyzeStatement(ifStatement->elseBranch.get());
    }
}

void SemanticAnalyzer::analyzeWhile(const WhileStatement* whileStatement) {
    checkCondition(whileStatement->condition.get());
    analyzeBlock(whileStatement->body.get());
}

void SemanticAnalyzer::analyzeFor(const ForStatement* forStatement) {
    analyzeStatement(forStatement->init.get());
    checkCondition(forStatement->condition.get());
    analyzeAssignment(forStatement->update.get());
    analyzeBlock(forStatement->body.get());
}

void SemanticAnalyzer::checkCondition(const ComparisonExpression* condition) {
    typeOfExpression(condition->left.get());
    typeOfExpression(condition->right.get());
}

void SemanticAnalyzer::analyzeCallStatement(const CallStatement* callStatement) {
    checkCall(callStatement->call.get());
}

const FunctionSignature& SemanticAnalyzer::checkCall(const CallExpression* call) {
    auto it = functions.find(call->callee);
    if (it == functions.end()) {
        throw error(call->line, "function '" + call->callee + "' is not declared.");
    }

    const FunctionSignature& signature = it->second;
    if (static_cast<int>(call->arguments.size()) != signature.parameterCount) {
        throw error(call->line, "function '" + call->callee + "' expects " +
                                     std::to_string(signature.parameterCount) +
                                     " argument(s) but got " +
                                     std::to_string(call->arguments.size()) + ".");
    }

    for (const std::unique_ptr<Expression>& argument : call->arguments) {
        typeOfExpression(argument.get());
    }

    return signature;
}

ValueType SemanticAnalyzer::typeOfExpression(const Expression* expression) {
    if (dynamic_cast<const NumberExpression*>(expression) != nullptr) {
        return ValueType::Int;
    }

    if (const VariableExpression* variable = dynamic_cast<const VariableExpression*>(expression)) {
        const Symbol* symbol = symbols.lookup(variable->name);
        if (symbol == nullptr) {
            throw error(variable->line, "variable '" + variable->name + "' is not declared.");
        }

        return symbol->type;
    }

    if (const CallExpression* call = dynamic_cast<const CallExpression*>(expression)) {
        const FunctionSignature& signature = checkCall(call);
        if (signature.returnType != ReturnType::Int) {
            throw error(call->line, "cannot use the result of void function '" + call->callee +
                                         "' as a value.");
        }
        return ValueType::Int;
    }

    if (const BinaryExpression* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        ValueType leftType = typeOfExpression(binary->left.get());
        ValueType rightType = typeOfExpression(binary->right.get());

        if (leftType == ValueType::Int && rightType == ValueType::Int) {
            return ValueType::Int;
        }

        // Unreachable while Int is the only ValueType (kept for when a
        // second expression type is introduced in a later phase).
        throw error(binary->line, std::string("incompatible types for operator '") +
                                       binary->operatorSymbol + "'.");
    }

    // Every concrete Expression subtype reachable from the Phase 1 grammar
    // is handled above; this only exists so the function has a
    // well-defined return on every path.
    throw std::runtime_error("Internal error: unrecognized expression node.");
}

std::runtime_error SemanticAnalyzer::error(int line, const std::string& message) const {
    return std::runtime_error("Semantic error on line " + std::to_string(line) + ": " + message);
}

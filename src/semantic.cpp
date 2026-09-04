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
    analyzeBlock(program.mainFunction->body.get());
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
        typeOfExpression(returnStatement->value.get());
    }
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

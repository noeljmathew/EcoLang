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
    for (const std::unique_ptr<Statement>& statement : program.statements) {
        analyzeStatement(statement.get());
    }
}

void SemanticAnalyzer::analyzeStatement(const Statement* statement) {
    if (const LetStatement* let = dynamic_cast<const LetStatement*>(statement)) {
        analyzeLet(let);
    } else if (const PrintStatement* print = dynamic_cast<const PrintStatement*>(statement)) {
        analyzePrint(print);
    } else if (const RepeatStatement* repeat = dynamic_cast<const RepeatStatement*>(statement)) {
        analyzeRepeat(repeat);
    }
}

void SemanticAnalyzer::analyzeLet(const LetStatement* letStatement) {
    ValueType type = typeOfExpression(letStatement->value.get());

    if (!symbols.declare(letStatement->name, type, letStatement->line)) {
        throw error(letStatement->line,
                    "variable '" + letStatement->name + "' is already declared.");
    }
}

void SemanticAnalyzer::analyzePrint(const PrintStatement* printStatement) {
    typeOfExpression(printStatement->expression.get());
}

void SemanticAnalyzer::analyzeRepeat(const RepeatStatement* repeatStatement) {
    ValueType countType = typeOfExpression(repeatStatement->count.get());

    if (countType != ValueType::Int) {
        throw error(repeatStatement->line, "repeat count must be an integer.");
    }

    if (!isConstantExpression(repeatStatement->count.get())) {
        throw error(repeatStatement->line, "repeat count must be a constant expression.");
    }

    for (const std::unique_ptr<Statement>& statement : repeatStatement->body) {
        analyzeStatement(statement.get());
    }
}

ValueType SemanticAnalyzer::typeOfExpression(const Expression* expression) {
    if (dynamic_cast<const NumberExpression*>(expression) != nullptr) {
        return ValueType::Int;
    }

    if (dynamic_cast<const StringExpression*>(expression) != nullptr) {
        return ValueType::String;
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

        if (leftType == ValueType::String && rightType == ValueType::String &&
            binary->operatorSymbol == '+') {
            return ValueType::String;
        }

        throw error(binary->line, std::string("incompatible types for operator '") +
                                       binary->operatorSymbol + "'.");
    }

    // Every concrete Expression subtype is handled above; this only exists
    // so the function has a well-defined return on every path.
    throw std::runtime_error("Internal error: unrecognized expression node.");
}

bool SemanticAnalyzer::isConstantExpression(const Expression* expression) {
    if (dynamic_cast<const NumberExpression*>(expression) != nullptr) {
        return true;
    }

    if (const BinaryExpression* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        return isConstantExpression(binary->left.get()) &&
               isConstantExpression(binary->right.get());
    }

    return false;
}

std::runtime_error SemanticAnalyzer::error(int line, const std::string& message) const {
    return std::runtime_error("Semantic error on line " + std::to_string(line) + ": " + message);
}

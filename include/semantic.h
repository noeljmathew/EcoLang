#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "ast.h"

// Phase 1 has exactly one value type. Kept as an enum (rather than
// dropping type-tracking entirely) since it is a natural extension
// point for later phases.
enum class ValueType { Int };

struct Symbol {
    std::string name;
    ValueType type;
    int declaredLine;
};

// One flat table for the whole program. Phase 1 explicitly uses a single
// function/main scope - a `{ }` block does not open a new scope, it is
// only a grouping construct for statements.
class SymbolTable {
public:
    // Returns false if the name is already declared (caller reports the error).
    bool declare(const std::string& name, ValueType type, int line);

    // Returns nullptr if the name has not been declared.
    const Symbol* lookup(const std::string& name) const;

private:
    std::unordered_map<std::string, Symbol> symbols;
};

class SemanticAnalyzer {
public:
    // Throws std::runtime_error on the first semantic error found.
    void analyze(const Program& program);

private:
    void analyzeBlock(const BlockStatement* block);
    void analyzeStatement(const Statement* statement);
    void analyzeDeclaration(const VariableDeclaration* declaration);
    void analyzeAssignment(const AssignmentStatement* assignment);
    void analyzePrintf(const PrintfStatement* printfStatement);
    void analyzeReturn(const ReturnStatement* returnStatement);

    ValueType typeOfExpression(const Expression* expression);

    std::runtime_error error(int line, const std::string& message) const;

    SymbolTable symbols;
};

#endif

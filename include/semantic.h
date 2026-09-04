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

// One flat table per function. A `{ }` block does not open a new scope,
// it is only a grouping construct for statements - Phase 3's "one flat
// local scope per function" is the same documented simplification Phases
// 1-2 used for the whole program, now scoped per function instead.
class SymbolTable {
public:
    // Returns false if the name is already declared (caller reports the error).
    bool declare(const std::string& name, ValueType type, int line);

    // Returns nullptr if the name has not been declared.
    const Symbol* lookup(const std::string& name) const;

private:
    std::unordered_map<std::string, Symbol> symbols;
};

// Collected once for the whole program before any function body is
// checked, so functions can call each other regardless of declaration
// order (including mutual recursion).
struct FunctionSignature {
    ReturnType returnType;
    int parameterCount;
    int declaredLine;
};

class SemanticAnalyzer {
public:
    // Throws std::runtime_error on the first semantic error found.
    void analyze(const Program& program);

private:
    void collectFunctionSignatures(const Program& program);
    void requireMainFunction() const;
    void analyzeFunction(const FunctionDeclaration* function);

    void analyzeBlock(const BlockStatement* block);
    void analyzeStatement(const Statement* statement);
    void analyzeDeclaration(const VariableDeclaration* declaration);
    void analyzeAssignment(const AssignmentStatement* assignment);
    void analyzePrintf(const PrintfStatement* printfStatement);
    void analyzeReturn(const ReturnStatement* returnStatement);
    void analyzeIf(const IfStatement* ifStatement);
    void analyzeWhile(const WhileStatement* whileStatement);
    void analyzeFor(const ForStatement* forStatement);
    void analyzeCallStatement(const CallStatement* callStatement);

    ValueType typeOfExpression(const Expression* expression);
    void checkCondition(const ComparisonExpression* condition);

    // Looks up callee, checks its argument count and each argument
    // expression's own validity. Does not check what the call's result
    // is used for - that is the caller's job (see typeOfExpression's
    // CallExpression case vs. analyzeCallStatement).
    const FunctionSignature& checkCall(const CallExpression* call);

    std::runtime_error error(int line, const std::string& message) const;

    SymbolTable symbols;
    std::unordered_map<std::string, FunctionSignature> functions;
    ReturnType currentFunctionReturnType = ReturnType::Void;
};

#endif

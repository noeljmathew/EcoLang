#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "ast.h"

// EcoLang currently only ever needs these two value types.
enum class ValueType { Int, String };

struct Symbol {
    std::string name;
    ValueType type;
    int declaredLine;
};

// One flat table for the whole program - EcoLang has no block scoping,
// so a variable declared anywhere (including inside a repeat block)
// stays visible for the rest of the program.
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
    void analyzeStatement(const Statement* statement);
    void analyzeLet(const LetStatement* letStatement);
    void analyzePrint(const PrintStatement* printStatement);
    void analyzeRepeat(const RepeatStatement* repeatStatement);

    ValueType typeOfExpression(const Expression* expression);

    // True for expressions whose value is known at compile time
    // (number literals and arithmetic built only from them).
    bool isConstantExpression(const Expression* expression);

    std::runtime_error error(int line, const std::string& message) const;

    SymbolTable symbols;
};

#endif

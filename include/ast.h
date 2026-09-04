#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Every AST node remembers the source line it came from, so later
// compiler stages (semantic analysis, IR generation, etc.) can report
// useful "error on line N" messages without having to re-derive it.
class Expression {
public:
    explicit Expression(int line) : line(line) {
    }

    virtual ~Expression() = default;

    int line;
};

class Statement {
public:
    explicit Statement(int line) : line(line) {
    }

    virtual ~Statement() = default;

    int line;
};

class NumberExpression : public Expression {
public:
    NumberExpression(int value, int line) : Expression(line), value(value) {
    }

    int value;
};

// Only ever produced as a printf format string in Phase 1 - the general
// expression grammar has no STRING case, so this never appears as a
// sub-expression of arithmetic.
class StringExpression : public Expression {
public:
    StringExpression(const std::string& value, int line) : Expression(line), value(value) {
    }

    std::string value;
};

class VariableExpression : public Expression {
public:
    VariableExpression(const std::string& name, int line) : Expression(line), name(name) {
    }

    std::string name;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left, char operatorSymbol,
                     std::unique_ptr<Expression> right, int line)
        : Expression(line), left(std::move(left)), operatorSymbol(operatorSymbol),
          right(std::move(right)) {
    }

    std::unique_ptr<Expression> left;
    char operatorSymbol;
    std::unique_ptr<Expression> right;
};

// A group of statements executed in order. Reused as main's body in
// Phase 1; intended to also become the body of if/while/for (Phase 2)
// and user-defined functions (Phase 3) without changing shape.
class BlockStatement : public Statement {
public:
    explicit BlockStatement(int line) : Statement(line) {
    }

    std::vector<std::unique_ptr<Statement>> statements;
};

// int x;         -> initializer == nullptr, declares x with no IR emitted.
// int x = expr;  -> initializer != nullptr.
class VariableDeclaration : public Statement {
public:
    VariableDeclaration(const std::string& name, std::unique_ptr<Expression> initializer, int line)
        : Statement(line), name(name), initializer(std::move(initializer)) {
    }

    std::string name;
    std::unique_ptr<Expression> initializer;
};

// x = expr;  - the variable must already be declared (checked by the
// semantic analyzer, not here).
class AssignmentStatement : public Statement {
public:
    AssignmentStatement(const std::string& name, std::unique_ptr<Expression> value, int line)
        : Statement(line), name(name), value(std::move(value)) {
    }

    std::string name;
    std::unique_ptr<Expression> value;
};

// printf(format, arg1, arg2, ...);  - format is always a string literal;
// arguments are ordinary integer expressions.
class PrintfStatement : public Statement {
public:
    PrintfStatement(std::unique_ptr<StringExpression> format,
                    std::vector<std::unique_ptr<Expression>> arguments, int line)
        : Statement(line), format(std::move(format)), arguments(std::move(arguments)) {
    }

    std::unique_ptr<StringExpression> format;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// return;       -> value == nullptr.
// return expr;  -> value != nullptr.
class ReturnStatement : public Statement {
public:
    ReturnStatement(std::unique_ptr<Expression> value, int line)
        : Statement(line), value(std::move(value)) {
    }

    std::unique_ptr<Expression> value;
};

// int main() { ... } - the sole entry point in Phase 1. Not a Statement:
// it is the one top-level construct Program wraps, not something that
// can appear inside a block.
class MainFunction {
public:
    MainFunction(std::unique_ptr<BlockStatement> body, int line)
        : body(std::move(body)), line(line) {
    }

    std::unique_ptr<BlockStatement> body;
    int line;
};

class Program {
public:
    std::unique_ptr<MainFunction> mainFunction;
};

#endif

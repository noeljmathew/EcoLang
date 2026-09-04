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

// name(arg1, arg2, ...) used as a value, e.g. x = add(a, b) + 1;
class CallExpression : public Expression {
public:
    CallExpression(const std::string& callee, std::vector<std::unique_ptr<Expression>> arguments,
                   int line)
        : Expression(line), callee(callee), arguments(std::move(arguments)) {
    }

    std::string callee;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// Shared with ir.h - comparisons only ever need these six kinds, and
// there is no reason for the AST and IR to each define their own copy.
enum class ComparisonOp { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };

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

// A single comparison - the only form a condition may take in this
// phase (no &&/||, no composing comparisons with arithmetic). Kept
// separate from BinaryExpression since operatorSymbol is a single char
// and cannot represent two-character operators like <= or ==.
class ComparisonExpression : public Expression {
public:
    ComparisonExpression(std::unique_ptr<Expression> left, ComparisonOp op,
                         std::unique_ptr<Expression> right, int line)
        : Expression(line), left(std::move(left)), op(op), right(std::move(right)) {
    }

    std::unique_ptr<Expression> left;
    ComparisonOp op;
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

// name(arg1, arg2, ...);  - a call used as a standalone statement, its
// return value (if any) discarded.
class CallStatement : public Statement {
public:
    CallStatement(std::unique_ptr<CallExpression> call, int line)
        : Statement(line), call(std::move(call)) {
    }

    std::unique_ptr<CallExpression> call;
};

// if (condition) { ... } [else ...]
// elseBranch is nullptr (no else), a BlockStatement (else { ... }), or
// another IfStatement (else if ...).
class IfStatement : public Statement {
public:
    IfStatement(std::unique_ptr<ComparisonExpression> condition,
               std::unique_ptr<BlockStatement> thenBranch, std::unique_ptr<Statement> elseBranch,
               int line)
        : Statement(line), condition(std::move(condition)), thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {
    }

    std::unique_ptr<ComparisonExpression> condition;
    std::unique_ptr<BlockStatement> thenBranch;
    std::unique_ptr<Statement> elseBranch;
};

// while (condition) { ... }
class WhileStatement : public Statement {
public:
    WhileStatement(std::unique_ptr<ComparisonExpression> condition,
                   std::unique_ptr<BlockStatement> body, int line)
        : Statement(line), condition(std::move(condition)), body(std::move(body)) {
    }

    std::unique_ptr<ComparisonExpression> condition;
    std::unique_ptr<BlockStatement> body;
};

// for (init; condition; update) { ... }
// init is a VariableDeclaration or an AssignmentStatement; update is
// always an AssignmentStatement.
class ForStatement : public Statement {
public:
    ForStatement(std::unique_ptr<Statement> init, std::unique_ptr<ComparisonExpression> condition,
                std::unique_ptr<AssignmentStatement> update, std::unique_ptr<BlockStatement> body,
                int line)
        : Statement(line), init(std::move(init)), condition(std::move(condition)),
          update(std::move(update)), body(std::move(body)) {
    }

    std::unique_ptr<Statement> init;
    std::unique_ptr<ComparisonExpression> condition;
    std::unique_ptr<AssignmentStatement> update;
    std::unique_ptr<BlockStatement> body;
};

enum class ReturnType { Int, Void };

// int name(int a, int b) { ... }  or  void name(int a) { ... }
// Every parameter is int (the only parameter type this language has), so
// parameters are just names - a fresh SymbolTable seeded with them is
// what gives each function its own flat local scope. Not a Statement:
// functions are only ever top-level, never nested inside a block.
class FunctionDeclaration {
public:
    FunctionDeclaration(ReturnType returnType, const std::string& name,
                        std::vector<std::string> parameters,
                        std::unique_ptr<BlockStatement> body, int line)
        : returnType(returnType), name(name), parameters(std::move(parameters)),
          body(std::move(body)), line(line) {
    }

    ReturnType returnType;
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockStatement> body;
    int line;
};

// A flat list of function declarations. Semantic analysis is responsible
// for requiring one of them to be a valid "main" - there is no longer a
// separate, special-cased entry-point AST node.
class Program {
public:
    std::vector<std::unique_ptr<FunctionDeclaration>> functions;
};

#endif

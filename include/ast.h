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

class LetStatement : public Statement {
public:
    LetStatement(const std::string& name, std::unique_ptr<Expression> value, int line)
        : Statement(line), name(name), value(std::move(value)) {
    }

    std::string name;
    std::unique_ptr<Expression> value;
};

class PrintStatement : public Statement {
public:
    PrintStatement(std::unique_ptr<Expression> expression, int line)
        : Statement(line), expression(std::move(expression)) {
    }

    std::unique_ptr<Expression> expression;
};

class RepeatStatement : public Statement {
public:
    RepeatStatement(std::unique_ptr<Expression> count, int line)
        : Statement(line), count(std::move(count)) {
    }

    std::unique_ptr<Expression> count;
    std::vector<std::unique_ptr<Statement>> body;
};

class Program {
public:
    std::vector<std::unique_ptr<Statement>> statements;
};

#endif

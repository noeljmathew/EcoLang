#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

class Expression {
public:
    virtual ~Expression() = default;
};

class Statement {
public:
    virtual ~Statement() = default;
};

class NumberExpression : public Expression {
public:
    explicit NumberExpression(int value) : value(value) {
    }

    int value;
};

class StringExpression : public Expression {
public:
    explicit StringExpression(const std::string& value) : value(value) {
    }

    std::string value;
};

class VariableExpression : public Expression {
public:
    explicit VariableExpression(const std::string& name) : name(name) {
    }

    std::string name;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left, char operatorSymbol,
                     std::unique_ptr<Expression> right)
        : left(std::move(left)), operatorSymbol(operatorSymbol), right(std::move(right)) {
    }

    std::unique_ptr<Expression> left;
    char operatorSymbol;
    std::unique_ptr<Expression> right;
};

class LetStatement : public Statement {
public:
    LetStatement(const std::string& name, std::unique_ptr<Expression> value)
        : name(name), value(std::move(value)) {
    }

    std::string name;
    std::unique_ptr<Expression> value;
};

class PrintStatement : public Statement {
public:
    explicit PrintStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {
    }

    std::unique_ptr<Expression> expression;
};

class RepeatStatement : public Statement {
public:
    explicit RepeatStatement(std::unique_ptr<Expression> count)
        : count(std::move(count)) {
    }

    std::unique_ptr<Expression> count;
    std::vector<std::unique_ptr<Statement>> body;
};

class Program {
public:
    std::vector<std::unique_ptr<Statement>> statements;
};

#endif

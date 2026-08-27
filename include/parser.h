#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ast.h"
#include "token.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parse();

private:
    std::unique_ptr<Statement> statement();
    std::unique_ptr<Statement> letStatement();
    std::unique_ptr<Statement> printStatement();
    std::unique_ptr<Statement> repeatStatement();

    std::unique_ptr<Expression> expression();
    std::unique_ptr<Expression> term();
    std::unique_ptr<Expression> factor();

    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& consume(TokenType type, const std::string& message);
    std::runtime_error error(const Token& token, const std::string& message) const;

    const std::vector<Token>& tokens;
    int current;
};

#endif

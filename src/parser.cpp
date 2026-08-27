#include "parser.h"

#include <exception>
#include <string>
#include <utility>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {
}

std::unique_ptr<Program> Parser::parse() {
    std::unique_ptr<Program> program = std::make_unique<Program>();

    while (!isAtEnd()) {
        program->statements.push_back(statement());
    }

    return program;
}

std::unique_ptr<Statement> Parser::statement() {
    if (match(TokenType::LET)) {
        return letStatement();
    }
    if (match(TokenType::PRINT)) {
        return printStatement();
    }
    if (match(TokenType::REPEAT)) {
        return repeatStatement();
    }

    throw error(peek(), "unexpected token '" + peek().lexeme + "'.");
}

std::unique_ptr<Statement> Parser::letStatement() {
    const Token& name = consume(TokenType::IDENTIFIER,
                                "expected an identifier after 'let'.");
    consume(TokenType::EQUAL, "expected '=' after variable name.");

    return std::make_unique<LetStatement>(name.lexeme, expression());
}

std::unique_ptr<Statement> Parser::printStatement() {
    return std::make_unique<PrintStatement>(expression());
}

std::unique_ptr<Statement> Parser::repeatStatement() {
    std::unique_ptr<RepeatStatement> repeat =
        std::make_unique<RepeatStatement>(expression());

    while (!check(TokenType::END) && !isAtEnd()) {
        repeat->body.push_back(statement());
    }

    if (isAtEnd()) {
        throw error(peek(), "expected 'end' to close repeat statement.");
    }

    consume(TokenType::END, "expected 'end' to close repeat statement.");
    return repeat;
}

std::unique_ptr<Expression> Parser::expression() {
    std::unique_ptr<Expression> left = term();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        char operatorSymbol = advance().lexeme[0];
        std::unique_ptr<Expression> right = term();
        left = std::make_unique<BinaryExpression>(std::move(left), operatorSymbol,
                                                  std::move(right));
    }

    return left;
}

std::unique_ptr<Expression> Parser::term() {
    std::unique_ptr<Expression> left = factor();

    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        char operatorSymbol = advance().lexeme[0];
        std::unique_ptr<Expression> right = factor();
        left = std::make_unique<BinaryExpression>(std::move(left), operatorSymbol,
                                                  std::move(right));
    }

    return left;
}

std::unique_ptr<Expression> Parser::factor() {
    if (match(TokenType::NUMBER)) {
        try {
            return std::make_unique<NumberExpression>(std::stoi(previous().lexeme));
        } catch (const std::exception&) {
            throw error(previous(), "invalid integer number.");
        }
    }

    if (match(TokenType::STRING)) {
        const std::string& text = previous().lexeme;
        return std::make_unique<StringExpression>(text.substr(1, text.length() - 2));
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpression>(previous().lexeme);
    }

    if (match(TokenType::LEFT_PAREN)) {
        std::unique_ptr<Expression> grouped = expression();
        consume(TokenType::RIGHT_PAREN, "expected ')' after expression.");
        return grouped;
    }

    throw error(peek(), "expected an expression.");
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return type == TokenType::END_OF_FILE;
    }

    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }

    advance();
    return true;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }

    return previous();
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }

    throw error(peek(), message);
}

std::runtime_error Parser::error(const Token& token, const std::string& message) const {
    return std::runtime_error("Syntax error on line " + std::to_string(token.line) +
                              ": " + message);
}

#include "parser.h"

#include <exception>
#include <string>
#include <utility>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {
}

std::unique_ptr<Program> Parser::parse() {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    program->mainFunction = mainFunction();
    consume(TokenType::END_OF_FILE, "expected end of file after the main function.");
    return program;
}

std::unique_ptr<MainFunction> Parser::mainFunction() {
    int line = peek().line;
    consume(TokenType::INT, "expected 'int' to begin the main function.");
    consume(TokenType::MAIN, "expected 'main' after 'int'.");
    consume(TokenType::LEFT_PAREN, "expected '(' after 'main'.");
    consume(TokenType::RIGHT_PAREN, "expected ')' after '('.");

    return std::make_unique<MainFunction>(block(), line);
}

std::unique_ptr<BlockStatement> Parser::block() {
    int line = peek().line;
    consume(TokenType::LEFT_BRACE, "expected '{' to begin a block.");

    std::unique_ptr<BlockStatement> blockStatement = std::make_unique<BlockStatement>(line);
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        blockStatement->statements.push_back(statement());
    }

    consume(TokenType::RIGHT_BRACE, "expected '}' to close a block.");
    return blockStatement;
}

std::unique_ptr<Statement> Parser::statement() {
    if (match(TokenType::INT)) {
        return declaration();
    }
    if (check(TokenType::IDENTIFIER)) {
        return assignment();
    }
    if (match(TokenType::PRINTF)) {
        return printfStatement();
    }
    if (match(TokenType::RETURN)) {
        return returnStatement();
    }
    if (check(TokenType::LEFT_BRACE)) {
        return block();
    }

    throw error(peek(), "unexpected token '" + peek().lexeme + "'.");
}

std::unique_ptr<Statement> Parser::declaration() {
    int line = previous().line;
    const Token& name = consume(TokenType::IDENTIFIER, "expected an identifier after 'int'.");

    std::unique_ptr<Expression> initializer;
    if (match(TokenType::EQUAL)) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "expected ';' after declaration.");
    return std::make_unique<VariableDeclaration>(name.lexeme, std::move(initializer), line);
}

std::unique_ptr<Statement> Parser::assignment() {
    const Token& name = consume(TokenType::IDENTIFIER, "expected an identifier.");
    int line = name.line;
    consume(TokenType::EQUAL, "expected '=' after variable name.");
    std::unique_ptr<Expression> value = expression();
    consume(TokenType::SEMICOLON, "expected ';' after assignment.");

    return std::make_unique<AssignmentStatement>(name.lexeme, std::move(value), line);
}

std::unique_ptr<Statement> Parser::printfStatement() {
    int line = previous().line;
    consume(TokenType::LEFT_PAREN, "expected '(' after 'printf'.");

    const Token& formatToken = consume(TokenType::STRING, "expected a format string after '('.");
    const std::string& text = formatToken.lexeme;
    std::unique_ptr<StringExpression> format =
        std::make_unique<StringExpression>(text.substr(1, text.length() - 2), formatToken.line);

    std::vector<std::unique_ptr<Expression>> arguments;
    while (match(TokenType::COMMA)) {
        arguments.push_back(expression());
    }

    consume(TokenType::RIGHT_PAREN, "expected ')' after printf arguments.");
    consume(TokenType::SEMICOLON, "expected ';' after printf statement.");

    return std::make_unique<PrintfStatement>(std::move(format), std::move(arguments), line);
}

std::unique_ptr<Statement> Parser::returnStatement() {
    int line = previous().line;

    std::unique_ptr<Expression> value;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }

    consume(TokenType::SEMICOLON, "expected ';' after return statement.");
    return std::make_unique<ReturnStatement>(std::move(value), line);
}

std::unique_ptr<Expression> Parser::expression() {
    int line = peek().line;
    std::unique_ptr<Expression> left = term();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        char operatorSymbol = advance().lexeme[0];
        std::unique_ptr<Expression> right = term();
        left = std::make_unique<BinaryExpression>(std::move(left), operatorSymbol,
                                                  std::move(right), line);
    }

    return left;
}

std::unique_ptr<Expression> Parser::term() {
    int line = peek().line;
    std::unique_ptr<Expression> left = factor();

    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        char operatorSymbol = advance().lexeme[0];
        std::unique_ptr<Expression> right = factor();
        left = std::make_unique<BinaryExpression>(std::move(left), operatorSymbol,
                                                  std::move(right), line);
    }

    return left;
}

std::unique_ptr<Expression> Parser::factor() {
    if (match(TokenType::NUMBER)) {
        try {
            return std::make_unique<NumberExpression>(std::stoi(previous().lexeme), previous().line);
        } catch (const std::exception&) {
            throw error(previous(), "invalid integer number.");
        }
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpression>(previous().lexeme, previous().line);
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

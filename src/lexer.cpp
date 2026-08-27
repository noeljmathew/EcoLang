#include "lexer.h"

#include <cctype>

Lexer::Lexer(const std::string& source)
    : source(source), start(0), current(0), line(1), tokenLine(1) {
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        tokenLine = line;
        scanToken();
    }

    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}

char Lexer::advance() {
    return source[current++];
}

bool Lexer::isAtEnd() const {
    return current >= static_cast<int>(source.length());
}

char Lexer::peek() const {
    if (isAtEnd()) {
        return '\0';
    }

    return source[current];
}

void Lexer::addToken(TokenType type) {
    tokens.push_back({type, source.substr(start, current - start), tokenLine});
}

void Lexer::scanToken() {
    char character = advance();

    switch (character) {
        case ' ': 
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            break;
        case '+': addToken(TokenType::PLUS); break;
        case '-': addToken(TokenType::MINUS); break;
        case '*': addToken(TokenType::STAR); break;
        case '/': addToken(TokenType::SLASH); break;
        case '=': addToken(TokenType::EQUAL); break;
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '"': string(); break;
        default:
            if (std::isalpha(static_cast<unsigned char>(character)) || character == '_') {
                identifier();
            } else if (std::isdigit(static_cast<unsigned char>(character))) {
                number();
            } else {
                addToken(TokenType::UNKNOWN);
            }
            break;
    }
}

void Lexer::identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    std::string text = source.substr(start, current - start);

    if (text == "let") {
        addToken(TokenType::LET);
    } else if (text == "print") {
        addToken(TokenType::PRINT);
    } else if (text == "repeat") {
        addToken(TokenType::REPEAT);
    } else if (text == "end") {
        addToken(TokenType::END);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

void Lexer::number() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    addToken(TokenType::NUMBER);
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
        }
        advance();
    }

    if (isAtEnd()) {
        addToken(TokenType::UNKNOWN);
        return;
    }

    advance();
    addToken(TokenType::STRING);
}

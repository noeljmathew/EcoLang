#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    INT,
    MAIN,
    PRINTF,
    RETURN,
    IDENTIFIER,
    NUMBER,
    STRING,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    SEMICOLON,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

#endif

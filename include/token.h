#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    LET,
    PRINT,
    REPEAT,
    END,
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
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

#endif

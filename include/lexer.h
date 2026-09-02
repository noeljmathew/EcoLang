#ifndef LEXER_H
#define LEXER_H

#include <stdexcept>
#include <string>
#include <vector>

#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source);

    std::vector<Token> scanTokens();

private:
    char advance();
    bool isAtEnd() const;
    char peek() const;
    void addToken(TokenType type);
    void scanToken();
    void identifier();
    void number();
    void string();
    std::runtime_error error(int line, const std::string& message) const;

    std::string source;
    std::vector<Token> tokens;
    int start;
    int current;
    int line;
    int tokenLine;
};

#endif

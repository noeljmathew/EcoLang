#include "lexer.h"

#include <cctype>

using namespace std;

Lexer::Lexer(const std::string& source)
    : source(source), start(0), current(0), line(1), tokenLine(1) {
}

vector<Token> Lexer::scanTokens() {
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
        case '/':
            if (peek() == '/') {
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
            } else if (peek() == '*') {
                advance();

                while (!isAtEnd()) {
                    char commentCharacter = advance();
                    if (commentCharacter == '\n') {
                        line++;
                    } else if (commentCharacter == '*' && peek() == '/') {
                        advance();
                        break;
                    }
                }
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case '=':
            if (peek() == '=') {
                advance();
                addToken(TokenType::EQUAL_EQUAL);
            } else {
                addToken(TokenType::EQUAL);
            }
            break;
        case '!':
            if (peek() == '=') {
                advance();
                addToken(TokenType::NOT_EQUAL);
            } else {
                throw error(tokenLine, "invalid character '!'.");
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                addToken(TokenType::LESS_EQUAL);
            } else {
                addToken(TokenType::LESS);
            }
            break;
        case '>':
            if (peek() == '=') {
                advance();
                addToken(TokenType::GREATER_EQUAL);
            } else {
                addToken(TokenType::GREATER);
            }
            break;
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '"': string(); break;
        default:
            if (isalpha(static_cast<unsigned char>(character)) || character == '_') {
                identifier();
            } else if (isdigit(static_cast<unsigned char>(character))) {
                number();
            } else {
                throw error(tokenLine, "invalid character '" + std::string(1, character) + "'.");
            }
            break;
    }
}

void Lexer::identifier() {
    while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    std::string text = source.substr(start, current - start);

    if (text == "int") {
        addToken(TokenType::INT);
    } else if (text == "printf") {
        addToken(TokenType::PRINTF);
    } else if (text == "return") {
        addToken(TokenType::RETURN);
    } else if (text == "if") {
        addToken(TokenType::IF);
    } else if (text == "else") {
        addToken(TokenType::ELSE);
    } else if (text == "while") {
        addToken(TokenType::WHILE);
    } else if (text == "for") {
        addToken(TokenType::FOR);
    } else if (text == "void") {
        addToken(TokenType::VOID);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

void Lexer::number() {
    while (isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    // A digit run immediately followed by a letter (e.g. "123abc") is not a
    // valid number and not a separate identifier either - it's almost
    // certainly a typo, so we report it as one bad token instead of quietly
    // splitting it into "123" and "abc".
    if (isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
        while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
            advance();
        }
        throw error(tokenLine, "invalid number '" + source.substr(start, current - start) + "'.");
    }

    addToken(TokenType::NUMBER);
}

void Lexer::string() {
    int stringStartLine = tokenLine;

    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
        }
        advance();
    }

    if (isAtEnd()) {
        throw error(stringStartLine, "unterminated string.");
    }

    advance();
    addToken(TokenType::STRING);
}

std::runtime_error Lexer::error(int line, const std::string& message) const {
    return std::runtime_error("Lexical error on line " + std::to_string(line) + ": " + message);
}

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"

void printIndent(int level) {
    std::cout << std::string(level * 2, ' ');
}

void printExpression(const Expression* expression, int level) {
    if (const NumberExpression* number = dynamic_cast<const NumberExpression*>(expression)) {
        printIndent(level);
        std::cout << "NumberExpression: " << number->value << '\n';
    } else if (const StringExpression* string = dynamic_cast<const StringExpression*>(expression)) {
        printIndent(level);
        std::cout << "StringExpression: \"" << string->value << "\"\n";
    } else if (const VariableExpression* variable =
                   dynamic_cast<const VariableExpression*>(expression)) {
        printIndent(level);
        std::cout << "VariableExpression: " << variable->name << '\n';
    } else if (const BinaryExpression* binary =
                   dynamic_cast<const BinaryExpression*>(expression)) {
        printIndent(level);
        std::cout << "BinaryExpression: " << binary->operatorSymbol << '\n';
        printExpression(binary->left.get(), level + 1);
        printExpression(binary->right.get(), level + 1);
    }
}

void printStatement(const Statement* statement, int level) {
    if (const LetStatement* let = dynamic_cast<const LetStatement*>(statement)) {
        printIndent(level);
        std::cout << "LetStatement: " << let->name << '\n';
        printExpression(let->value.get(), level + 1);
    } else if (const PrintStatement* print = dynamic_cast<const PrintStatement*>(statement)) {
        printIndent(level);
        std::cout << "PrintStatement\n";
        printExpression(print->expression.get(), level + 1);
    } else if (const RepeatStatement* repeat = dynamic_cast<const RepeatStatement*>(statement)) {
        printIndent(level);
        std::cout << "RepeatStatement\n";
        printIndent(level + 1);
        std::cout << "Count:\n";
        printExpression(repeat->count.get(), level + 2);
        printIndent(level + 1);
        std::cout << "Body:\n";
        for (const std::unique_ptr<Statement>& bodyStatement : repeat->body) {
            printStatement(bodyStatement.get(), level + 2);
        }
    }
}

int main() {
    std::ifstream file("samples/hello.eco");
    if (!file) {
        std::cerr << "Could not open samples/hello.eco\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer lexer(buffer.str());
    std::vector<Token> tokens = lexer.scanTokens();

    try {
        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parse();

        std::cout << "Program\n";
        for (const std::unique_ptr<Statement>& statement : program->statements) {
            printStatement(statement.get(), 1);
        }
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

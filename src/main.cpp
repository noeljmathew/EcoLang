#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"

using namespace std;

void printIndent(int level) {
    cout << string(level * 2, ' ');
}

void printExpression(const Expression* expression, int level) {
    if (const NumberExpression* number = dynamic_cast<const NumberExpression*>(expression)) {
        printIndent(level);
        cout << "NumberExpression: " << number->value << '\n';
    } else if (const StringExpression* string = dynamic_cast<const StringExpression*>(expression)) {
        printIndent(level);
        cout << "StringExpression: \"" << string->value << "\"\n";
    } else if (const VariableExpression* variable =
                   dynamic_cast<const VariableExpression*>(expression)) {
        printIndent(level);
        cout << "VariableExpression: " << variable->name << '\n';
    } else if (const BinaryExpression* binary =
                   dynamic_cast<const BinaryExpression*>(expression)) {
        printIndent(level);
        cout << "BinaryExpression: " << binary->operatorSymbol << '\n';
        printExpression(binary->left.get(), level + 1);
        printExpression(binary->right.get(), level + 1);
    }
}

void printStatement(const Statement* statement, int level) {
    if (const LetStatement* let = dynamic_cast<const LetStatement*>(statement)) {
        printIndent(level);
        cout << "LetStatement: " << let->name << '\n';
        printExpression(let->value.get(), level + 1);
    } else if (const PrintStatement* print = dynamic_cast<const PrintStatement*>(statement)) {
        printIndent(level);
        cout << "PrintStatement\n";
        printExpression(print->expression.get(), level + 1);
    } else if (const RepeatStatement* repeat = dynamic_cast<const RepeatStatement*>(statement)) {
        printIndent(level);
        cout << "RepeatStatement\n";
        printIndent(level + 1);
        cout << "Count:\n";
        printExpression(repeat->count.get(), level + 2);
        printIndent(level + 1);
        cout << "Body:\n";
        for (const unique_ptr<Statement>& bodyStatement : repeat->body) {
            printStatement(bodyStatement.get(), level + 2);
        }
    }
}

int main() {
    ifstream file("samples/hello.eco");
    if (!file) {
        cerr << "Could not open samples/hello.eco\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    Lexer lexer(buffer.str());
    vector<Token> tokens = lexer.scanTokens();

    try {
        Parser parser(tokens);
        unique_ptr<Program> program = parser.parse();

        cout << "Program\n";
        for (const unique_ptr<Statement>& statement : program->statements) {
            printStatement(statement.get(), 1);
        }
    } catch (const runtime_error& error) {
        cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

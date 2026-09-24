#include "dump.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

// TokenType-to-string helper - nothing else in the codebase needed one
// before this (parser.cpp only ever compares TokenType values, never
// prints their names).
std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::INT:
            return "INT";
        case TokenType::PRINTF:
            return "PRINTF";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::IF:
            return "IF";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::FOR:
            return "FOR";
        case TokenType::VOID:
            return "VOID";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::STRING:
            return "STRING";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::STAR:
            return "STAR";
        case TokenType::SLASH:
            return "SLASH";
        case TokenType::EQUAL:
            return "EQUAL";
        case TokenType::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case TokenType::NOT_EQUAL:
            return "NOT_EQUAL";
        case TokenType::LESS:
            return "LESS";
        case TokenType::LESS_EQUAL:
            return "LESS_EQUAL";
        case TokenType::GREATER:
            return "GREATER";
        case TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case TokenType::LEFT_PAREN:
            return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:
            return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::END_OF_FILE:
            return "END_OF_FILE";
        case TokenType::UNKNOWN:
            return "UNKNOWN";
        default:
            throw std::runtime_error("Token dump error: unrecognized token type.");
    }
}

// Same style already used for ComparisonOp -> mnemonic in
// CodeGenerator::comparisonMnemonicFor (codegen.cpp) - here mapping back
// to the source-level symbol instead of an assembly mnemonic, since the
// AST JSON schema (see ast_parser.py's _comparison_operator) stores
// comparisons as their original "==", "!=", "<", "<=", ">", ">=" text.
std::string comparisonOpSymbol(ComparisonOp op) {
    switch (op) {
        case ComparisonOp::Equal:
            return "==";
        case ComparisonOp::NotEqual:
            return "!=";
        case ComparisonOp::Less:
            return "<";
        case ComparisonOp::LessEqual:
            return "<=";
        case ComparisonOp::Greater:
            return ">";
        case ComparisonOp::GreaterEqual:
            return ">=";
        default:
            throw std::runtime_error("AST dump error: unrecognized comparison operator.");
    }
}

std::string returnTypeName(ReturnType type) {
    switch (type) {
        case ReturnType::Int:
            return "int";
        case ReturnType::Void:
            return "void";
        default:
            throw std::runtime_error("AST dump error: unrecognized return type.");
    }
}

// Standard JSON string escaping. Identifiers and numbers never need it,
// but a printf format string can contain arbitrary characters, including
// '"' or '\' (if the source escapes into the format string oddly) or raw
// control characters (this lexer's string() allows a literal newline
// inside a string literal) - so this is written as a general-purpose
// escaper rather than special-cased to only what this language's own
// test corpus happens to use.
std::string jsonEscape(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (unsigned char character : text) {
        switch (character) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (character < 0x20) {
                    std::ostringstream escaped;
                    escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                            << static_cast<int>(character);
                    result += escaped.str();
                } else {
                    result += static_cast<char>(character);
                }
        }
    }

    return result;
}

std::string jsonString(const std::string& text) {
    return "\"" + jsonEscape(text) + "\"";
}

std::string dumpExpression(const Expression* expression);
std::string dumpStatement(const Statement* statement);

std::string dumpBlock(const BlockStatement* block) {
    std::string statements = "[";
    for (size_t i = 0; i < block->statements.size(); ++i) {
        if (i > 0) {
            statements += ",";
        }
        statements += dumpStatement(block->statements[i].get());
    }
    statements += "]";

    return "{\"type\":\"Block\",\"line\":" + std::to_string(block->line) +
           ",\"statements\":" + statements + "}";
}

// Dispatch chain matching the exact style already used by
// SemanticAnalyzer::typeOfExpression (semantic.cpp) and
// IRGenerator::generateExpression (ir.cpp): dynamic_cast down the known
// concrete Expression subclasses in ast.h.
//
// StringExpression is deliberately not one of the cases here: the real
// grammar's expression()/term()/factor() chain (parser.cpp) never
// produces a StringExpression as a general sub-expression - it only ever
// exists wrapped directly in PrintfStatement::format, which dumpStatement
// below reads as a plain string field (printfStatement->format->value),
// never routing it through this function. The toy parser's grammar has
// the identical restriction (its _factor() has no STRING case either),
// so this is not a shape gap between the two - it is why the frontend's
// unreachable "String" case in expressionToDisplay was always dead code
// for both parsers, not something this change introduces.
std::string dumpExpression(const Expression* expression) {
    if (const NumberExpression* number = dynamic_cast<const NumberExpression*>(expression)) {
        return "{\"type\":\"Number\",\"line\":" + std::to_string(number->line) +
               ",\"value\":" + std::to_string(number->value) + "}";
    }

    if (const VariableExpression* variable = dynamic_cast<const VariableExpression*>(expression)) {
        return "{\"type\":\"Variable\",\"line\":" + std::to_string(variable->line) +
               ",\"name\":" + jsonString(variable->name) + "}";
    }

    if (const CallExpression* call = dynamic_cast<const CallExpression*>(expression)) {
        std::string args = "[";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            if (i > 0) {
                args += ",";
            }
            args += dumpExpression(call->arguments[i].get());
        }
        args += "]";

        return "{\"type\":\"Call\",\"line\":" + std::to_string(call->line) +
               ",\"callee\":" + jsonString(call->callee) + ",\"args\":" + args + "}";
    }

    if (const BinaryExpression* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        return "{\"type\":\"Binary\",\"line\":" + std::to_string(binary->line) +
               ",\"operator\":" + jsonString(std::string(1, binary->operatorSymbol)) +
               ",\"left\":" + dumpExpression(binary->left.get()) +
               ",\"right\":" + dumpExpression(binary->right.get()) + "}";
    }

    if (const ComparisonExpression* comparison =
            dynamic_cast<const ComparisonExpression*>(expression)) {
        return "{\"type\":\"Comparison\",\"line\":" + std::to_string(comparison->line) +
               ",\"operator\":" + jsonString(comparisonOpSymbol(comparison->op)) +
               ",\"left\":" + dumpExpression(comparison->left.get()) +
               ",\"right\":" + dumpExpression(comparison->right.get()) + "}";
    }

    throw std::runtime_error("AST dump error: unknown expression node type.");
}

// Dispatch chain matching the exact style already used by
// SemanticAnalyzer::analyzeStatement (semantic.cpp), same case order.
std::string dumpStatement(const Statement* statement) {
    if (const VariableDeclaration* declaration =
            dynamic_cast<const VariableDeclaration*>(statement)) {
        std::string initializer = declaration->initializer != nullptr
                                       ? dumpExpression(declaration->initializer.get())
                                       : "null";
        return "{\"type\":\"VariableDeclaration\",\"line\":" + std::to_string(declaration->line) +
               ",\"name\":" + jsonString(declaration->name) +
               ",\"initializer\":" + initializer + "}";
    }

    if (const AssignmentStatement* assignment =
            dynamic_cast<const AssignmentStatement*>(statement)) {
        return "{\"type\":\"Assignment\",\"line\":" + std::to_string(assignment->line) +
               ",\"name\":" + jsonString(assignment->name) +
               ",\"value\":" + dumpExpression(assignment->value.get()) + "}";
    }

    if (const PrintfStatement* printfStatement =
            dynamic_cast<const PrintfStatement*>(statement)) {
        std::string args = "[";
        for (size_t i = 0; i < printfStatement->arguments.size(); ++i) {
            if (i > 0) {
                args += ",";
            }
            args += dumpExpression(printfStatement->arguments[i].get());
        }
        args += "]";

        return "{\"type\":\"Printf\",\"line\":" + std::to_string(printfStatement->line) +
               ",\"format\":" + jsonString(printfStatement->format->value) +
               ",\"args\":" + args + "}";
    }

    if (const ReturnStatement* returnStatement =
            dynamic_cast<const ReturnStatement*>(statement)) {
        std::string value = returnStatement->value != nullptr
                                 ? dumpExpression(returnStatement->value.get())
                                 : "null";
        return "{\"type\":\"Return\",\"line\":" + std::to_string(returnStatement->line) +
               ",\"value\":" + value + "}";
    }

    if (const IfStatement* ifStatement = dynamic_cast<const IfStatement*>(statement)) {
        std::string elseBranch = ifStatement->elseBranch != nullptr
                                      ? dumpStatement(ifStatement->elseBranch.get())
                                      : "null";
        return "{\"type\":\"If\",\"line\":" + std::to_string(ifStatement->line) +
               ",\"condition\":" + dumpExpression(ifStatement->condition.get()) +
               ",\"then\":" + dumpBlock(ifStatement->thenBranch.get()) +
               ",\"else\":" + elseBranch + "}";
    }

    if (const WhileStatement* whileStatement = dynamic_cast<const WhileStatement*>(statement)) {
        return "{\"type\":\"While\",\"line\":" + std::to_string(whileStatement->line) +
               ",\"condition\":" + dumpExpression(whileStatement->condition.get()) +
               ",\"body\":" + dumpBlock(whileStatement->body.get()) + "}";
    }

    if (const ForStatement* forStatement = dynamic_cast<const ForStatement*>(statement)) {
        return "{\"type\":\"For\",\"line\":" + std::to_string(forStatement->line) +
               ",\"init\":" + dumpStatement(forStatement->init.get()) +
               ",\"condition\":" + dumpExpression(forStatement->condition.get()) +
               ",\"update\":" + dumpStatement(forStatement->update.get()) +
               ",\"body\":" + dumpBlock(forStatement->body.get()) + "}";
    }

    if (const CallStatement* callStatement = dynamic_cast<const CallStatement*>(statement)) {
        return "{\"type\":\"CallStatement\",\"line\":" + std::to_string(callStatement->line) +
               ",\"call\":" + dumpExpression(callStatement->call.get()) + "}";
    }

    if (const BlockStatement* block = dynamic_cast<const BlockStatement*>(statement)) {
        return dumpBlock(block);
    }

    throw std::runtime_error("AST dump error: unknown statement node type.");
}

std::string dumpFunction(const FunctionDeclaration* function) {
    std::string params = "[";
    for (size_t i = 0; i < function->parameters.size(); ++i) {
        if (i > 0) {
            params += ",";
        }
        params += jsonString(function->parameters[i]);
    }
    params += "]";

    return "{\"type\":\"FunctionDeclaration\",\"line\":" + std::to_string(function->line) +
           ",\"name\":" + jsonString(function->name) +
           ",\"returnType\":" + jsonString(returnTypeName(function->returnType)) +
           ",\"params\":" + params + ",\"body\":" + dumpBlock(function->body.get()) + "}";
}

}  // namespace

std::string dumpTokens(const std::vector<Token>& tokens) {
    std::ostringstream out;
    for (const Token& token : tokens) {
        out << token.line << ' ' << tokenTypeName(token.type);
        if (!token.lexeme.empty()) {
            out << ' ' << token.lexeme;
        }
        out << '\n';
    }
    return out.str();
}

// One genuine, unavoidable schema difference from ast_parser.py's
// Program node: neither side has a "line" field here (the real Program
// class in ast.h has no line member of its own - a program's own line
// number was never a meaningful concept, only its functions have one -
// and parse_program() in ast_parser.py builds the Program dict by hand
// without going through its _node() helper for exactly the same reason,
// so it omits "line" too). This is not a mismatch to flag; the two
// schemas already agree on it, and the frontend's programToDisplay never
// reads program.line regardless.
std::string dumpAst(const Program& program) {
    std::string functions = "[";
    for (size_t i = 0; i < program.functions.size(); ++i) {
        if (i > 0) {
            functions += ",";
        }
        functions += dumpFunction(program.functions[i].get());
    }
    functions += "]";

    return "{\"type\":\"Program\",\"functions\":" + functions + "}";
}

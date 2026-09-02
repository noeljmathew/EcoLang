#ifndef IR_H
#define IR_H

#include <string>
#include <vector>

#include "ast.h"

enum class IROpcode {
    STORE,
    BINARY_OP,
    PRINT,
    LOOP_START,
    LOOP_END
};

enum class IROperandKind {
    IntConstant,
    StringConstant,
    Name
};

struct IROperand {
    IROperandKind kind;
    int intValue = 0;
    std::string text;
};

// A single flat instruction. Not every field is used by every opcode
// (e.g. PRINT only uses operandA) - kept as one plain struct rather than
// a class hierarchy so later stages (optimizer, energy analysis) can
// freely copy/insert/erase instructions with plain std::vector operations.
struct IRInstruction {
    IROpcode opcode;
    std::string destination;
    char operatorSymbol = '\0';
    IROperand operandA;
    IROperand operandB;
    int line;
};

using IRProgram = std::vector<IRInstruction>;

// Lowers an already semantically-checked AST into a flat IRProgram.
// Assumes SemanticAnalyzer::analyze() has already succeeded on the same
// Program - it does not re-check declarations, types, or repeat counts.
class IRGenerator {
public:
    IRProgram generate(const Program& program);

private:
    void generateStatement(const Statement* statement, IRProgram& output);
    void generateLet(const LetStatement* statement, IRProgram& output);
    void generatePrint(const PrintStatement* statement, IRProgram& output);
    void generateRepeat(const RepeatStatement* statement, IRProgram& output);
    IROperand generateExpression(const Expression* expression, IRProgram& output);

    // Generates a fresh compiler-internal name ($t1, $t2, ...). The '$'
    // prefix can never collide with a user identifier, since EcoLang
    // identifiers must start with a letter or underscore.
    std::string newTemp();

    int tempCounter = 0;
};

#endif

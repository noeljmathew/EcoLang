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
    LOOP_END,
    RETURN,
    COMPARE,
    IF_START,
    ELSE,
    IF_END,
    FUNC_START,
    FUNC_END,
    CALL
};

// No static loop-bound analysis in this phase: every while/for loop
// compiles to LOOP_START(kAssumedLoopIterations)/LOOP_END regardless of
// its real condition, exactly like a fixed-count repeat. Illustrative
// placeholder, not a real bound analysis - styled like
// kJoulesPerEnergyUnit in energy.h.
constexpr int kAssumedLoopIterations = 10;

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

    // Used only by RETURN: true if operandA holds the returned
    // expression's value ("return expr;"), false for a bare "return;".
    // Every other opcode that reads operandA always has a real value
    // there and can ignore this field.
    bool hasOperand = false;

    // Used only by PRINT (printf): the literal format string, and the
    // argument operands in order. All other opcodes continue using
    // operandA/operandB exactly as before; PRINT does not use them.
    std::string formatString;
    std::vector<IROperand> operandList;

    // Used only by COMPARE: which of the six comparisons operandA and
    // operandB are being combined with (destination = operandA op
    // operandB, a 0/1 value) - operatorSymbol is a single char and
    // cannot represent <=, >=, ==, or !=.
    ComparisonOp comparisonOp = ComparisonOp::Equal;

    // Used by FUNC_START (the function being defined) and CALL (the
    // function being invoked). FUNC_START's parameter names reuse
    // operandList (one Name operand per parameter), the same way PRINT
    // already reuses it for a variable-length list of things. CALL's
    // arguments also live in operandList; its return value's temp uses
    // the existing destination field, same pattern as BINARY_OP/COMPARE.
    std::string functionName;
};

using IRProgram = std::vector<IRInstruction>;

// Lowers an already semantically-checked AST into a flat IRProgram.
// Assumes SemanticAnalyzer::analyze() has already succeeded on the same
// Program - it does not re-check declarations or types.
class IRGenerator {
public:
    IRProgram generate(const Program& program);

private:
    void generateStatement(const Statement* statement, IRProgram& output);
    void generateBlock(const BlockStatement* statement, IRProgram& output);
    void generateDeclaration(const VariableDeclaration* statement, IRProgram& output);
    void generateAssignment(const AssignmentStatement* statement, IRProgram& output);
    void generatePrintf(const PrintfStatement* statement, IRProgram& output);
    void generateReturn(const ReturnStatement* statement, IRProgram& output);
    void generateIf(const IfStatement* statement, IRProgram& output);
    void generateWhile(const WhileStatement* statement, IRProgram& output);
    void generateFor(const ForStatement* statement, IRProgram& output);
    void generateCallStatement(const CallStatement* statement, IRProgram& output);
    void generateFunction(const FunctionDeclaration* function, IRProgram& output);
    IROperand generateExpression(const Expression* expression, IRProgram& output);
    IROperand generateComparison(const ComparisonExpression* condition, IRProgram& output);
    IROperand generateCall(const CallExpression* call, IRProgram& output);

    // Generates a fresh compiler-internal name ($t1, $t2, ...). The '$'
    // prefix can never collide with a user identifier, since identifiers
    // must start with a letter or underscore.
    std::string newTemp();
    IROperand makeAssumedIterationCount() const;

    int tempCounter = 0;
};

#endif

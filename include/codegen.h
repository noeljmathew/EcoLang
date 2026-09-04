#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>

#include "ir.h"

// Translates an IRProgram (normally the optimizer's output) into a
// simple, readable pseudo-assembly / three-address-style text listing -
// one line per instruction, in the same order. This is NOT real machine
// code, real assembly, or LLVM IR; it exists purely as a beginner-legible
// target representation suitable for a project demonstration.
class CodeGenerator {
public:
    // Throws std::runtime_error (with a line number) if the program
    // contains an IR opcode or a BINARY_OP operator this generator does
    // not recognize, instead of silently emitting incorrect output.
    std::string generate(const IRProgram& program);

private:
    std::string generateInstruction(const IRInstruction& instruction) const;
    std::string operandToText(const IROperand& operand) const;
    std::string mnemonicFor(char operatorSymbol, int line) const;
    std::string comparisonMnemonicFor(ComparisonOp op) const;
};

#endif

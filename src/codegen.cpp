#include "codegen.h"

#include <sstream>
#include <stdexcept>

std::string CodeGenerator::generate(const IRProgram& program) {
    std::ostringstream output;

    for (const IRInstruction& instruction : program) {
        output << generateInstruction(instruction) << '\n';
    }

    return output.str();
}

// IR opcode -> pseudo-assembly line:
//   STORE      -> MOV  destination, value
//   BINARY_OP  -> ADD/SUB/MUL/DIV destination, left, right (see mnemonicFor)
//   PRINT      -> PRINT value
//   LOOP_START -> LOOP_START iterationCount
//   LOOP_END   -> LOOP_END
std::string CodeGenerator::generateInstruction(const IRInstruction& instruction) const {
    switch (instruction.opcode) {
        case IROpcode::STORE:
            return "MOV " + instruction.destination + ", " + operandToText(instruction.operandA);

        case IROpcode::BINARY_OP: {
            std::string mnemonic = mnemonicFor(instruction.operatorSymbol, instruction.line);
            return mnemonic + " " + instruction.destination + ", " +
                   operandToText(instruction.operandA) + ", " +
                   operandToText(instruction.operandB);
        }

        case IROpcode::PRINT:
            return "PRINT " + operandToText(instruction.operandA);

        case IROpcode::LOOP_START:
            return "LOOP_START " + operandToText(instruction.operandA);

        case IROpcode::LOOP_END:
            return "LOOP_END";

        default:
            throw std::runtime_error("Code generation error on line " +
                                     std::to_string(instruction.line) + ": unsupported IR opcode.");
    }
}

std::string CodeGenerator::operandToText(const IROperand& operand) const {
    switch (operand.kind) {
        case IROperandKind::IntConstant:
            return std::to_string(operand.intValue);

        case IROperandKind::StringConstant:
            return "\"" + operand.text + "\"";

        case IROperandKind::Name:
            return operand.text;

        default:
            throw std::runtime_error("Code generation error: unsupported IR operand kind.");
    }
}

std::string CodeGenerator::mnemonicFor(char operatorSymbol, int line) const {
    switch (operatorSymbol) {
        case '+':
            return "ADD";
        case '-':
            return "SUB";
        case '*':
            return "MUL";
        case '/':
            return "DIV";
        default:
            throw std::runtime_error("Code generation error on line " + std::to_string(line) +
                                     ": unsupported operator '" + operatorSymbol + "'.");
    }
}

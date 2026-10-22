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

        case IROpcode::PRINT: {
            std::string line = "PRINT \"" + instruction.formatString + "\"";
            for (const IROperand& argument : instruction.operandList) {
                line += ", " + operandToText(argument);
            }
            return line;
        }

        case IROpcode::LOOP_START:
            return "LOOP_START " + operandToText(instruction.operandA);

        case IROpcode::LOOP_END:
            return "LOOP_END";

        case IROpcode::RETURN:
            if (instruction.hasOperand) {
                return "RETURN " + operandToText(instruction.operandA);
            }
            return "RETURN";

        case IROpcode::COMPARE: {
            std::string mnemonic = comparisonMnemonicFor(instruction.comparisonOp);
            return mnemonic + " " + instruction.destination + ", " +
                   operandToText(instruction.operandA) + ", " +
                   operandToText(instruction.operandB);
        }

        case IROpcode::IF_START:
            return "IF_START " + operandToText(instruction.operandA);

        case IROpcode::ELSE:
            return "ELSE";

        case IROpcode::IF_END:
            return "IF_END";

        case IROpcode::FUNC_START: {
            std::string parameters;
            for (size_t i = 0; i < instruction.operandList.size(); i++) {
                if (i > 0) {
                    parameters += ", ";
                }
                parameters += operandToText(instruction.operandList[i]);
            }
            return "FUNC_START " + instruction.functionName + "(" + parameters + ")";
        }

        case IROpcode::FUNC_END:
            return "FUNC_END " + instruction.functionName;

        case IROpcode::CALL: {
            // No destination (e.g. the synthetic entry-point CALL main):
            // print "CALL f" rather than "CALL , f".
            std::string line = instruction.destination.empty()
                                   ? "CALL " + instruction.functionName
                                   : "CALL " + instruction.destination + ", " + instruction.functionName;
            for (const IROperand& argument : instruction.operandList) {
                line += ", " + operandToText(argument);
            }
            return line;
        }

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

std::string CodeGenerator::comparisonMnemonicFor(ComparisonOp op) const {
    switch (op) {
        case ComparisonOp::Equal:
            return "CMPEQ";
        case ComparisonOp::NotEqual:
            return "CMPNE";
        case ComparisonOp::Less:
            return "CMPLT";
        case ComparisonOp::LessEqual:
            return "CMPLE";
        case ComparisonOp::Greater:
            return "CMPGT";
        case ComparisonOp::GreaterEqual:
            return "CMPGE";
        default:
            throw std::runtime_error("Code generation error: unrecognized comparison operator.");
    }
}

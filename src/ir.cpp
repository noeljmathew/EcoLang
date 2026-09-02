#include "ir.h"

#include <stdexcept>
#include <string>

IRProgram IRGenerator::generate(const Program& program) {
    IRProgram output;

    for (const std::unique_ptr<Statement>& statement : program.statements) {
        generateStatement(statement.get(), output);
    }

    return output;
}

void IRGenerator::generateStatement(const Statement* statement, IRProgram& output) {
    if (const LetStatement* let = dynamic_cast<const LetStatement*>(statement)) {
        generateLet(let, output);
    } else if (const PrintStatement* print = dynamic_cast<const PrintStatement*>(statement)) {
        generatePrint(print, output);
    } else if (const RepeatStatement* repeat = dynamic_cast<const RepeatStatement*>(statement)) {
        generateRepeat(repeat, output);
    }
}

void IRGenerator::generateLet(const LetStatement* statement, IRProgram& output) {
    IROperand value = generateExpression(statement->value.get(), output);

    IRInstruction instruction{};
    instruction.opcode = IROpcode::STORE;
    instruction.destination = statement->name;
    instruction.operandA = value;
    instruction.line = statement->line;
    output.push_back(instruction);
}

void IRGenerator::generatePrint(const PrintStatement* statement, IRProgram& output) {
    IROperand value = generateExpression(statement->expression.get(), output);

    IRInstruction instruction{};
    instruction.opcode = IROpcode::PRINT;
    instruction.operandA = value;
    instruction.line = statement->line;
    output.push_back(instruction);
}

void IRGenerator::generateRepeat(const RepeatStatement* statement, IRProgram& output) {
    // The semantic analyzer already guarantees this is a constant integer
    // expression, so generating it here can never emit anything other than
    // an IntConstant operand (no BINARY_OP is ever needed for the count).
    IROperand count = generateExpression(statement->count.get(), output);

    IRInstruction loopStart{};
    loopStart.opcode = IROpcode::LOOP_START;
    loopStart.operandA = count;
    loopStart.line = statement->line;
    output.push_back(loopStart);

    for (const std::unique_ptr<Statement>& bodyStatement : statement->body) {
        generateStatement(bodyStatement.get(), output);
    }

    IRInstruction loopEnd{};
    loopEnd.opcode = IROpcode::LOOP_END;
    loopEnd.line = statement->line;
    output.push_back(loopEnd);
}

IROperand IRGenerator::generateExpression(const Expression* expression, IRProgram& output) {
    if (const NumberExpression* number = dynamic_cast<const NumberExpression*>(expression)) {
        IROperand operand{};
        operand.kind = IROperandKind::IntConstant;
        operand.intValue = number->value;
        return operand;
    }

    if (const StringExpression* text = dynamic_cast<const StringExpression*>(expression)) {
        IROperand operand{};
        operand.kind = IROperandKind::StringConstant;
        operand.text = text->value;
        return operand;
    }

    if (const VariableExpression* variable = dynamic_cast<const VariableExpression*>(expression)) {
        IROperand operand{};
        operand.kind = IROperandKind::Name;
        operand.text = variable->name;
        return operand;
    }

    if (const BinaryExpression* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        IROperand left = generateExpression(binary->left.get(), output);
        IROperand right = generateExpression(binary->right.get(), output);

        std::string temp = newTemp();

        IRInstruction instruction{};
        instruction.opcode = IROpcode::BINARY_OP;
        instruction.destination = temp;
        instruction.operatorSymbol = binary->operatorSymbol;
        instruction.operandA = left;
        instruction.operandB = right;
        instruction.line = binary->line;
        output.push_back(instruction);

        IROperand result{};
        result.kind = IROperandKind::Name;
        result.text = temp;
        return result;
    }

    // Every concrete Expression subtype is handled above; this only exists
    // so the function has a well-defined return on every path.
    throw std::runtime_error("Internal error: unrecognized expression node.");
}

std::string IRGenerator::newTemp() {
    ++tempCounter;
    return "$t" + std::to_string(tempCounter);
}

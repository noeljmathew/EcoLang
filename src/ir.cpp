#include "ir.h"

#include <stdexcept>
#include <string>

IRProgram IRGenerator::generate(const Program& program) {
    IRProgram output;
    generateBlock(program.mainFunction->body.get(), output);
    return output;
}

void IRGenerator::generateStatement(const Statement* statement, IRProgram& output) {
    if (const VariableDeclaration* declaration =
            dynamic_cast<const VariableDeclaration*>(statement)) {
        generateDeclaration(declaration, output);
    } else if (const AssignmentStatement* assignment =
                   dynamic_cast<const AssignmentStatement*>(statement)) {
        generateAssignment(assignment, output);
    } else if (const PrintfStatement* printfStatement =
                   dynamic_cast<const PrintfStatement*>(statement)) {
        generatePrintf(printfStatement, output);
    } else if (const ReturnStatement* returnStatement =
                   dynamic_cast<const ReturnStatement*>(statement)) {
        generateReturn(returnStatement, output);
    } else if (const BlockStatement* block = dynamic_cast<const BlockStatement*>(statement)) {
        generateBlock(block, output);
    }
}

void IRGenerator::generateBlock(const BlockStatement* statement, IRProgram& output) {
    for (const std::unique_ptr<Statement>& inner : statement->statements) {
        generateStatement(inner.get(), output);
    }
}

void IRGenerator::generateDeclaration(const VariableDeclaration* statement, IRProgram& output) {
    if (statement->initializer == nullptr) {
        return;  // declaration only - nothing to compute yet
    }

    IROperand value = generateExpression(statement->initializer.get(), output);

    IRInstruction instruction{};
    instruction.opcode = IROpcode::STORE;
    instruction.destination = statement->name;
    instruction.operandA = value;
    instruction.line = statement->line;
    output.push_back(instruction);
}

void IRGenerator::generateAssignment(const AssignmentStatement* statement, IRProgram& output) {
    IROperand value = generateExpression(statement->value.get(), output);

    IRInstruction instruction{};
    instruction.opcode = IROpcode::STORE;
    instruction.destination = statement->name;
    instruction.operandA = value;
    instruction.line = statement->line;
    output.push_back(instruction);
}

void IRGenerator::generatePrintf(const PrintfStatement* statement, IRProgram& output) {
    std::vector<IROperand> arguments;
    for (const std::unique_ptr<Expression>& argument : statement->arguments) {
        arguments.push_back(generateExpression(argument.get(), output));
    }

    IRInstruction instruction{};
    instruction.opcode = IROpcode::PRINT;
    instruction.formatString = statement->format->value;
    instruction.operandList = std::move(arguments);
    instruction.line = statement->line;
    output.push_back(instruction);
}

void IRGenerator::generateReturn(const ReturnStatement* statement, IRProgram& output) {
    IRInstruction instruction{};
    instruction.opcode = IROpcode::RETURN;
    instruction.line = statement->line;

    if (statement->value != nullptr) {
        instruction.operandA = generateExpression(statement->value.get(), output);
        instruction.hasOperand = true;
    }

    output.push_back(instruction);
}

IROperand IRGenerator::generateExpression(const Expression* expression, IRProgram& output) {
    if (const NumberExpression* number = dynamic_cast<const NumberExpression*>(expression)) {
        IROperand operand{};
        operand.kind = IROperandKind::IntConstant;
        operand.intValue = number->value;
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

    // Every concrete Expression subtype reachable here (NumberExpression,
    // VariableExpression, BinaryExpression) is handled above -
    // StringExpression never reaches this function, since printf's format
    // string is read directly from the AST in generatePrintf instead.
    throw std::runtime_error("Internal error: unrecognized expression node.");
}

std::string IRGenerator::newTemp() {
    ++tempCounter;
    return "$t" + std::to_string(tempCounter);
}

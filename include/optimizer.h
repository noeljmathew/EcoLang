#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <optional>
#include <string>
#include <unordered_map>

#include "ir.h"

// Runs a small set of safe, local optimizations over an IRProgram and
// returns a new, optimized IRProgram:
//   - constant folding         (BINARY_OP on two int constants)
//   - constant propagation     (known constant values substituted into
//                                later operands, including inside
//                                LOOP_START's own count operand)
//   - algebraic simplification (x+0, 0+x, x-0, x*1, 1*x, x*0, 0*x)
//   - dead temporary elimination (unused "$t..." instructions removed)
//
// LOOP_START/LOOP_END act as barriers for propagated variable knowledge:
// anything learned about a variable's value while inside a loop body is
// forgotten again once the matching LOOP_END is reached (the loop body
// might run zero or many times, so knowledge learned inside it must not
// leak to code after the loop). Folding pure arithmetic and simplifying
// algebraic identities inside a loop body is still done, since those are
// local and safe regardless of how many times the loop runs.
//
// Never reorders PRINT / LOOP_START / LOOP_END instructions, never
// removes a user-named STORE, and only folds integer arithmetic (never
// string concatenation).

// Simple counts describing what the most recent optimize() call actually
// did, computed directly from real transformations the optimizer already
// performs (nothing here is estimated or hard-coded):
//   - originalInstructionCount / optimizedInstructionCount / instructionsRemoved
//     come straight from the input and output IRProgram sizes.
//   - constantFoldingCount is incremented exactly once per BINARY_OP that
//     gets replaced by a computed constant STORE.
//   - constantPropagationCount is incremented exactly once per operand
//     that gets substituted with a previously-known constant value (a
//     Name resolved against the current constant map), regardless of
//     which instruction it happens in.
//   - algebraicSimplificationCount is incremented exactly once per
//     BINARY_OP rewritten via one of the seven identities, i.e. only when
//     one side is a known identity constant (0 or 1) and the other side
//     is NOT already a compile-time constant (if both sides are already
//     constants, that BINARY_OP is fully folded instead, and is counted
//     under constantFoldingCount, not here).
//   - deadTemporaryEliminationCount is incremented exactly once per
//     compiler-generated "$t..." instruction dropped as unused.
// A single instruction can contribute to more than one count (e.g. a
// BINARY_OP whose operands both get propagated in and then the whole
// thing folds counts as two propagations and one folding).
struct OptimizationStats {
    int originalInstructionCount = 0;
    int optimizedInstructionCount = 0;
    int instructionsRemoved = 0;

    int constantFoldingCount = 0;
    int constantPropagationCount = 0;
    int algebraicSimplificationCount = 0;
    int deadTemporaryEliminationCount = 0;
};

class Optimizer {
public:
    IRProgram optimize(const IRProgram& input);

    // Statistics describing what the most recent optimize() call did.
    OptimizationStats getStats() const;

private:
    using ConstantMap = std::unordered_map<std::string, IROperand>;

    IRProgram foldAndPropagate(const IRProgram& input);
    IRProgram eliminateDeadTemporaries(const IRProgram& input);

    // Returns the operand's known constant value if it is a Name that
    // currently maps to one, otherwise returns the operand unchanged.
    // Counts a constant-propagation event whenever a substitution happens.
    IROperand resolveOperand(const IROperand& operand, const ConstantMap& knownConstants);

    // Remembers "destination now holds this constant value", or forgets
    // any previous value for it if the new value is not a constant.
    void recordIfConstant(const std::string& destination, const IROperand& value,
                           ConstantMap& knownConstants) const;

    // Integer arithmetic only. Returns std::nullopt for division by zero.
    std::optional<int> foldArithmetic(char operatorSymbol, int left, int right) const;

    // Applies the seven allowed algebraic identities. Returns the
    // surviving operand (or a 0 constant for the multiply-by-zero cases)
    // when one applies, otherwise std::nullopt.
    std::optional<IROperand> simplifyIdentity(char operatorSymbol, const IROperand& left,
                                              const IROperand& right) const;

    OptimizationStats stats;
};

#endif

# EcoLang

EcoLang is a small, hand-written, energy-aware compiler built in C++17 as a
Compiler Design course project. It compiles a tiny imperative language
(`let`, `print`, `repeat ... end`) all the way from source text through
lexing, parsing, semantic analysis, IR generation, optimization, and code
generation, and finishes by estimating how much "energy" the compiled
program costs to run.

## Project objective

The goal isn't to build a production language - it's to demonstrate every
classic compiler stage end to end, on a language simple enough to reason
about by hand, while adding one twist beyond a standard course compiler: an
**energy cost model**. Every operation in the optimized program is given an
estimated energy cost, so the compiler can report not just "does this
program run correctly" but "roughly how expensive is it," and suggest that
optimizing the program (constant folding, dead-code removal, ...) also
reduces its estimated energy cost.

## Language features

EcoLang supports exactly this, no more:

- `let <identifier> = <expression>` - declare a variable and give it a value
- `print <expression>` - print a value
- `repeat <constant integer expression> ... end` - repeat a block a fixed
  number of times, including nested `repeat` blocks
- Two value types: **integers** and **strings**
- Arithmetic operators `+ - * /` on integers
- String concatenation with `+` (`"foo" + "bar"`)
- Parentheses for grouping
- `//` line comments and `/* ... */` block comments

There is no `if`, no boolean type, no functions, no arrays, no unary minus
(so no negative number literals), and no way to reassign a variable after
it has been declared. See **Assumptions and limitations** below.

## Grammar

```
program    -> statement*
statement  -> letStatement | printStatement | repeatStatement
letStatement    -> "let" IDENTIFIER "=" expression
printStatement  -> "print" expression
repeatStatement -> "repeat" expression statement* "end"

expression -> term (("+" | "-") term)*
term       -> factor (("*" | "/") factor)*
factor     -> NUMBER | STRING | IDENTIFIER | "(" expression ")"
```

Operator precedence is the usual `* /` before `+ -`, both left-associative;
parentheses override precedence as normal. Statements are not separated by
semicolons or required to be on their own line - the grammar is entirely
token-driven.

## Token types

Defined in [include/token.h](include/token.h):

```
LET, PRINT, REPEAT, END,
IDENTIFIER, NUMBER, STRING,
PLUS, MINUS, STAR, SLASH, EQUAL,
LEFT_PAREN, RIGHT_PAREN,
END_OF_FILE, UNKNOWN
```

Each `Token` carries its `type`, its raw source text (`lexeme`), and the
source `line` it came from, so every later stage can report line-numbered
errors. The lexer ([src/lexer.cpp](src/lexer.cpp)) throws a
`Lexical error on line N: ...` for an invalid character, an unterminated
string, or a number immediately followed by a letter (e.g. `123abc`).

## AST structure

Defined in [include/ast.h](include/ast.h). Every node - `Expression` and
`Statement` alike - carries the source `line` it came from. Concrete nodes:

- `NumberExpression`, `StringExpression`, `VariableExpression`
- `BinaryExpression` (left, operator character, right)
- `LetStatement`, `PrintStatement`
- `RepeatStatement` (count expression + a body of statements)
- `Program` (the root: a list of top-level statements)

The parser ([src/parser.cpp](src/parser.cpp)) is a straightforward
recursive-descent parser matching the grammar above, building this tree
directly with `std::unique_ptr` ownership (no manual memory management,
no raw-pointer ownership ambiguity).

## Semantic analysis

[include/semantic.h](include/semantic.h) / [src/semantic.cpp](src/semantic.cpp)

`SemanticAnalyzer` walks the AST once and checks:

- **Declared before use** - using a variable that was never `let`
- **No redeclaration** - `let x` twice anywhere in the program is an error
- **Type validity** - see the table below
- **Repeat count validity** - must be an `Int`, and must be a compile-time
  constant expression (see *Assumptions*)

Only two types exist: `Int` and `String`. Valid binary operations:

| Operation | Valid? |
|---|---|
| `int + int`, `int - int`, `int * int`, `int / int` | yes, produces `Int` |
| `string + string` | yes, produces `String` (concatenation) |
| any other mix (`int + string`, `string - string`, `string * int`, ...) | no - semantic error |

Errors are reported as `Semantic error on line N: ...` and stop compilation
immediately (no partial IR/optimization/energy/codegen/report runs after a
semantic error).

## Symbol table

A single flat `std::unordered_map<std::string, Symbol>`
([include/semantic.h](include/semantic.h)), where each `Symbol` stores the
variable's `name`, `type`, and the line it was declared on. There is
deliberately **no scope stack** - EcoLang has one global namespace for the
whole program, including variables declared inside a `repeat` block (see
*Assumptions*).

## IR structure

[include/ir.h](include/ir.h) / [src/ir.cpp](src/ir.cpp)

The AST is lowered into a flat `std::vector<IRInstruction>` (`IRProgram`) -
classic three-address code, not a tree. Five opcodes:

```
STORE       dest = operandA
BINARY_OP   dest = operandA <op> operandB
PRINT       operandA
LOOP_START  operandA   (iteration count)
LOOP_END
```

An operand (`IROperand`) is tagged as an `IntConstant`, a `StringConstant`,
or a `Name` (a variable or a compiler-generated temporary). Multi-step
arithmetic is flattened using temporaries named `$t1, $t2, ...` - the `$`
prefix can never collide with a user identifier, since EcoLang identifiers
must start with a letter or underscore. Every instruction keeps the source
`line` of the AST node that produced it. Nested `repeat` blocks become
nested `LOOP_START`/`LOOP_END` pairs in the same flat vector - there is no
separate tree structure to track nesting.

Example: `let x = 2 + 3 * 4` lowers to:

```
BINARY_OP $t1 = 3 * 4
BINARY_OP $t2 = 2 + $t1
STORE     x   = $t2
```

## Optimization techniques

[include/optimizer.h](include/optimizer.h) / [src/optimizer.cpp](src/optimizer.cpp)

`Optimizer::optimize` runs two passes over the IR and returns a new,
optimized `IRProgram`:

1. **Constant folding** - a `BINARY_OP` on two integer constants is
   computed at compile time and replaced with a `STORE` of the result
   (never for division by zero, which is left untouched).
2. **Constant propagation** - a small map remembers which variables/temps
   currently hold a known constant, and substitutes that constant into
   every later operand that references them by name (including a
   `repeat` loop's own count operand).
3. **Algebraic simplification** - exactly seven safe integer identities:
   `x+0`, `0+x`, `x-0`, `x*1`, `1*x`, `x*0`, `0*x`.
4. **Dead temporary elimination** - a compiler-generated `$t...` temporary
   whose value is never read after the above passes is dropped entirely.
   User-named variables are never removed, even if seemingly unused.

`LOOP_START`/`LOOP_END` act as a barrier: any constant knowledge learned
about a variable *inside* a loop body is forgotten again once the loop
closes (the loop might run zero or many times), so it never incorrectly
leaks to code after the loop. Folding pure arithmetic inside a loop body is
still done freely, since that's safe regardless of iteration count.

`Optimizer::getStats()` returns an `OptimizationStats` describing exactly
what the most recent `optimize()` call did (instruction counts before/after,
plus how many times each of the four techniques above actually fired) - the
compiler prints this as the `[Optimization]` section of its normal output.
These counts are computed directly from the real transformations above, not
estimated. One subtlety: if a `BINARY_OP`'s *other* operand also happens to
already be a known constant (e.g. `c + 0` where `c` was itself propagated
in as a literal), that instruction is fully folded rather than identity-
simplified, since the "both operands are constants" check runs first - so
it's counted under Constant Folding, not Algebraic Simplification. The
identity counter only increments when one side is a known identity
constant (`0` or `1`) and the *other* side genuinely isn't foldable.

## Energy cost model

[include/energy.h](include/energy.h) / [src/energy.cpp](src/energy.cpp)

`analyzeEnergy` walks the **optimized** IR once, keeping a running loop
multiplier (a stack, multiplied by each `LOOP_START`'s count and restored
on the matching `LOOP_END`) to know how many times each instruction will
actually execute. Cost table:

| Operation | Cost |
|---|---|
| Assignment (`STORE`) | 1 unit |
| Arithmetic (`BINARY_OP`) | 1 unit |
| Comparison | 1 unit (always 0 occurrences today - EcoLang has no comparison operators yet) |
| Print (`PRINT`) | 5 units |
| Loop overhead | 1 unit **per iteration** |

The result (`EnergyReport`) contains a full breakdown (assignment /
arithmetic / comparison / loop-iteration / print counts), `totalOperations`
(unweighted), and an energy breakdown - `assignmentEnergy`, `arithmeticEnergy`,
`comparisonEnergy`, `loopEnergy`, `printEnergy` - which is each count above
multiplied by its cost from the table. `totalEnergyUnits` is simply the sum
of those five (never computed a second, different way), and `estimatedJoules`
is `totalEnergyUnits * kJoulesPerEnergyUnit`. All of this is computed once,
in `analyzeEnergy` itself; `report.cpp` only formats the numbers it's given.
See *Assumptions* for the two intentional deviations from the requirements
document baked into this model, and why.

## Code generation

[include/codegen.h](include/codegen.h) / [src/codegen.cpp](src/codegen.cpp)

`CodeGenerator::generate` takes the optimized IR and emits a simple,
readable pseudo-assembly text listing - **not** real machine code, real
assembly, or LLVM IR, just a one-line-per-instruction translation meant to
be easy to read in a demo:

```
STORE x = 5              ->  MOV x, 5
BINARY_OP $t1 = 2 + 3    ->  ADD $t1, 2, 3   (SUB/MUL/DIV likewise)
PRINT x                  ->  PRINT x
LOOP_START 5              -> LOOP_START 5
LOOP_END                  -> LOOP_END
```

Instruction order and loop structure are always preserved exactly. An
unrecognized IR opcode or `BINARY_OP` operator raises a clear,
line-numbered `Code generation error on line N: ...` instead of silently
emitting something wrong.

## Final pipeline

[src/main.cpp](src/main.cpp) wires every stage together:

```
source file
  -> Lexer            (source -> tokens)
  -> Parser            (tokens -> AST)
  -> SemanticAnalyzer  (checks the AST; stops here on error)
  -> IRGenerator       (AST -> raw IR)
  -> Optimizer         (raw IR -> optimized IR)
  -> analyzeEnergy     (optimized IR -> EnergyReport)
  -> CodeGenerator     (optimized IR -> pseudo-assembly text)
  -> ReportGenerator   (EnergyReport -> formatted report text)
```

Energy analysis and code generation are both run on the **same** optimized
IR value (energy analysis is never run twice, and never run on the raw
IR). Any error at any stage is caught once, printed to stderr, and the
program exits with status 1 - no later stage ever runs after an earlier
one has failed.

## How to compile

With `g++` directly (no external dependencies):

```sh
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude \
    src/main.cpp src/lexer.cpp src/parser.cpp src/semantic.cpp \
    src/ir.cpp src/energy.cpp src/optimizer.cpp src/codegen.cpp \
    src/report.cpp -o build/ecolang
```

Or with CMake, if you have it installed:

```sh
cmake -S . -B build
cmake --build build
```

Both build the same set of source files and should produce identical
behavior; `CMakeLists.txt` is kept in sync with the `g++` command above.

## How to run it

```sh
./build/ecolang <path-to-file.eco>
```

Passing no file prints a usage message and exits 1. Passing a file that
can't be opened, or a program that fails to lex/parse/type-check, prints a
single clear error message and exits 1.

## Example EcoLang program

```
let x = 2 + 3 * 4
print x

repeat 10
    repeat 20
        print "Hi"
    end
end
```

## Example output

```
========================================
           EcoLang Compiler
========================================

[Optimization]
-----------------
Original IR Instructions   : 9
Optimized IR Instructions  : 7
Instructions Removed       : 2

Constant Folding           : 2
Constant Propagation       : 3
Algebraic Simplification   : 0
Dead Temporary Elimination : 2

[Code Generation]
-----------------
MOV x, 14
PRINT 14
LOOP_START 10
LOOP_START 20
PRINT "Hi"
LOOP_END
LOOP_END

[Energy Report]
----------------
========================================
        EcoLang Energy Report
========================================

Operation Counts
----------------
Assignments         : 1
Arithmetic Ops      : 0
Comparisons         : 0
Loop Iterations     : 210
Print Statements    : 201
Total Operations    : 412

Energy Breakdown
----------------
Assignment Energy   : 1
Arithmetic Energy   : 0
Comparison Energy   : 0
Loop Energy         : 210
Print Energy        : 1005

Total Energy Units  : 1216
Estimated Energy    : 0.0001216 J

Complexity
----------
Time Complexity     : O(n^2)
```

Note that `Arithmetic Ops` is `0`: the optimizer folds `2 + 3 * 4` down to
the constant `14` before energy analysis or code generation ever run, so
there is no arithmetic left to count or execute.

## Assumptions and limitations

These are deliberate, documented decisions made while building EcoLang -
not requirements handed down by the project brief, except where noted:

- EcoLang currently supports **Int and String** values only - no boolean,
  no float.
- The symbol table is **global/flat**; there are no nested/block scopes. A
  variable declared inside a `repeat` block is visible for the rest of the
  program after the block ends.
- **Variables cannot be reassigned** after declaration - there is no bare
  `x = value` statement, only `let x = value`, and declaring the same name
  twice is a semantic error. This means a variable's value is fixed for
  the rest of the program once declared.
- **Repeat counts must be compile-time constant expressions** (a literal,
  or arithmetic built only from literals) - not an arbitrary variable.
  This is a project assumption, not an explicit requirement in the source
  document, but it's necessary: without it, neither the energy analyzer
  nor the code generator's `LOOP_START` would have a concrete iteration
  count to work with.
- **Loop overhead is modeled as 1 energy unit per iteration.** The
  requirements document's own cost table lists "Loop Overhead = 2-3", but
  its worked example (`repeat 100` -> "Loop overhead = 100") only makes
  sense at 1 unit per iteration. This implementation follows the worked
  example; the constant is named (`kLoopOverheadUnitsPerIteration` in
  `energy.h`) so it can be changed to 2 or 3 in one place if needed.
- **1 energy unit = `1e-7` J is an illustrative scaling factor**, not a
  hardware measurement - the requirements document doesn't specify a real
  conversion, and this exists purely to give the report a human-readable
  Joules figure.
- **Time complexity is a structural loop-nesting heuristic**, not a full
  asymptotic analysis: 0 loop levels -> `O(1)`, 1 -> `O(n)`, 2 -> `O(n^2)`,
  and so on, based purely on maximum loop nesting depth in the IR.
- **Energy analysis runs on optimized IR**, never on the raw IR - the
  pipeline always calls the optimizer first, and both energy analysis and
  code generation consume that same optimized result.
- There is no unary minus, so negative number literals (`-5`) are not
  supported.
- Error recovery stops at the first error in each stage (lexer, parser, or
  semantic analyzer) rather than collecting and reporting multiple errors
  at once.

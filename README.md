# EcoLang

EcoLang is a small, hand-written, energy-aware compiler for a **restricted
subset of C**, built in C++17 as a Compiler Design course project. It is
not a full C compiler - it supports `int`/`void` functions, parameters,
declarations, assignment, arithmetic, `printf`, `return`, `if`/`else`,
`while`, `for`, comparisons, and function calls (including recursion), and
nothing beyond that. It compiles source text all the way through lexing,
parsing, semantic analysis, IR generation, optimization, and pseudo-code
generation, and finishes by estimating how much "energy" the compiled
program costs to run.

## Project purpose

The goal isn't to build a production language or a real C compiler - it's
to demonstrate every classic compiler stage end to end, on a language
simple enough to reason about by hand, while adding one twist beyond a
standard course compiler: an **energy cost model**. Every operation in the
optimized program is given an estimated energy cost, so the compiler can
report not just "does this program run correctly" but "roughly how
expensive is it," and show that optimizing a program (constant folding,
dead-code removal, ...) also reduces its estimated energy cost.

EcoLang started as an even smaller toy language (`let` / `print` /
`repeat ... end`) and was migrated in three phases to the C-like subset
described below - first the basic statement forms, then control flow and
comparisons, then functions and recursion. The migration is complete; the
language described here is the current, final one.

## Supported C subset

- `int x;` and `int x = expr;` - integer declarations, with or without an initializer
- `x = expr;` - assignment to an already-declared variable
- Arithmetic: `+ - * /`, with the usual precedence and parentheses for grouping
- Comparisons: `== != < <= > >=` - exactly one comparison per condition (no `&&`/`||`)
- `if (condition) { ... }` with an optional `else { ... }` (including `else if`)
- `while (condition) { ... }`
- `for (init; condition; update) { ... }` - `init` is a declaration or assignment, `update` is an assignment
- `printf("format", arg1, arg2, ...);` - any number of comma-separated integer arguments
- `return;` and `return expr;`
- Function declarations: `int name(int a, int b) { ... }` or `void name(int a) { ... }` - only `int` and `void` return types, only `int` parameters
- Function calls, as an expression (`x = add(a, b) + 1;`) or as a standalone statement (`printResult(x);`, discarding the result)
- Recursion, including mutual recursion (two functions calling each other)
- `//` line comments and `/* ... */` block comments
- `{ }` blocks and `;` statement terminators

**Not supported** (deliberately, to keep the project scoped): arrays,
pointers, structs, floats, booleans, strings as a variable type (a string
can only ever appear as a `printf` format literal), `&&`/`||`, `break`/
`continue`, unary minus, function prototypes/forward declarations, nested
function definitions, and block-level scoping (see *Assumptions and
limitations*).

## Compiler pipeline

```
source file (.c)
  -> Lexer            (source -> tokens)
  -> Parser            (tokens -> AST)
  -> SemanticAnalyzer  (checks the AST; stops here on error)
  -> IRGenerator       (AST -> raw IR)
  -> Optimizer         (raw IR -> optimized IR)
  -> analyzeEnergy     (optimized IR -> EnergyReport)
  -> CodeGenerator     (optimized IR -> pseudo-assembly text)
  -> ReportGenerator   (EnergyReport -> formatted report text)
```

[src/main.cpp](src/main.cpp) wires every stage together. Energy analysis
and code generation both run on the **same** optimized IR (energy
analysis is never run twice, and never run on the raw IR). Any error at
any stage is caught once, printed to stderr, and the program exits with
status 1 - no later stage ever runs after an earlier one has failed.

### Lexer and tokens

[include/lexer.h](include/lexer.h) / [src/lexer.cpp](src/lexer.cpp),
tokens defined in [include/token.h](include/token.h). Hand-written,
character-at-a-time scanning (no regex). Every `Token` carries its `type`,
raw source text, and source `line`, so every later stage can report
line-numbered errors. The lexer throws a `Lexical error on line N: ...`
for an invalid character, an unterminated string, or a number immediately
followed by a letter (e.g. `123abc`). Two-character operators (`==`, `!=`,
`<=`, `>=`) are recognized with one character of lookahead, the same
technique already used to tell `/`, `//`, and `/*` apart.

### AST and parser

[include/ast.h](include/ast.h) - every node carries the source `line` it
came from. A `Program` is a list of `FunctionDeclaration`s (one of which
must be named `main`, taking no parameters and returning `int` - checked
by semantic analysis, not the parser). The parser
([src/parser.cpp](src/parser.cpp)) is a straightforward recursive-descent
parser matching the grammar below, building the tree with `std::unique_ptr`
ownership throughout.

```
program            -> functionDeclaration*
functionDeclaration -> ("int" | "void") IDENTIFIER "(" parameters? ")" block
parameters         -> "int" IDENTIFIER ("," "int" IDENTIFIER)*
block              -> "{" statement* "}"

statement -> declaration | assignment | printfStatement | returnStatement
           | ifStatement | whileStatement | forStatement | callStatement
           | block

declaration     -> "int" IDENTIFIER ("=" expression)? ";"
assignment      -> IDENTIFIER "=" expression ";"
printfStatement -> "printf" "(" STRING ("," expression)* ")" ";"
returnStatement -> "return" expression? ";"
ifStatement     -> "if" "(" condition ")" block ("else" (ifStatement | block))?
whileStatement  -> "while" "(" condition ")" block
forStatement    -> "for" "(" (declaration | assignment) condition ";" update ")" block
update          -> IDENTIFIER "=" expression   (an assignment, but with no ";" of its own)
callStatement   -> IDENTIFIER "(" arguments? ")" ";"

condition  -> expression ("==" | "!=" | "<" | "<=" | ">" | ">=") expression
expression -> term (("+" | "-") term)*
term       -> factor (("*" | "/") factor)*
factor     -> NUMBER | IDENTIFIER | call | "(" expression ")"
call       -> IDENTIFIER "(" arguments? ")"
arguments  -> expression ("," expression)*
```

### Semantic analysis

[include/semantic.h](include/semantic.h) /
[src/semantic.cpp](src/semantic.cpp). Function signatures are collected
in a first pass, before any function body is checked, so functions can
call each other regardless of declaration order - this is what makes
mutual recursion work without needing forward declarations. Then each
function's body is checked using its own fresh, flat symbol table, seeded
with its parameters. Checks performed:

- Declared before use, and no redeclaration (variables and functions alike)
- Every call's argument count matches the callee's declared parameter count, and each argument expression is itself valid
- A `void` function's result cannot be used as a value (only as a discarded, standalone call statement)
- A `void` function cannot `return` a value
- Every `if`/`while`/`for` condition is a valid comparison
- The program defines exactly one `main`, taking no parameters and returning `int`

Errors are reported as `Semantic error on line N: ...` (or, for a missing
`main`, a plain `Semantic error: ...` with no single line to point to) and
stop compilation immediately.

### IR

[include/ir.h](include/ir.h) / [src/ir.cpp](src/ir.cpp). The AST is
lowered into a flat `std::vector<IRInstruction>` - three-address code, not
a tree. Opcodes: `STORE`, `BINARY_OP`, `COMPARE`, `PRINT`, `LOOP_START`/
`LOOP_END`, `IF_START`/`ELSE`/`IF_END`, `RETURN`, `FUNC_START`/`FUNC_END`,
`CALL`. Every function is emitted once, as `FUNC_START name(params) ...
FUNC_END name`; the program's actual entry point is a synthetic `CALL
main` appended after every function definition. A recursive call is just
an ordinary `CALL` whose target happens to be the function currently being
defined - no special-casing needed anywhere in the pipeline.

## Optimization

[include/optimizer.h](include/optimizer.h) /
[src/optimizer.cpp](src/optimizer.cpp). `Optimizer::optimize` runs constant
folding (arithmetic and comparisons), constant propagation, seven safe
algebraic identities (`x+0`, `0+x`, `x-0`, `x*1`, `1*x`, `x*0`, `0*x`), and
dead-temporary elimination (compiler-generated `$t...` temporaries only -
user-named variables and `CALL` instructions are never removed, since a
call may have side effects regardless of whether its result is used).

`LOOP_START`/`LOOP_END` and `IF_START`/`ELSE`/`IF_END` act as barriers:
after a loop or `if`/`else`, a variable's value is only still trusted if it
is *unchanged* from before the region - anything reassigned inside it, or
newly introduced inside it, is forgotten, since we don't know at compile
time how many times the loop ran or which branch was taken.
`FUNC_START`/`FUNC_END` use a stronger, full reset instead: a function's
parameters come fresh from whatever the caller passed, so nothing about
the caller's known values applies inside it. There is no interprocedural
constant folding or inlining across a `CALL` - each function is optimized
using only what's known about its own body.

## Energy analysis

[include/energy.h](include/energy.h) / [src/energy.cpp](src/energy.cpp).
Cost table (estimated computational cost, not a real hardware
measurement):

| Operation | Cost |
|---|---|
| Assignment (`STORE`) | 1 unit |
| Arithmetic (`BINARY_OP`) | 1 unit |
| Comparison (`COMPARE`) | 1 unit |
| Print (`PRINT`) | 5 units |
| Loop overhead | 1 unit per iteration |

`analyzeEnergy` computes each function's own energy cost separately
(splitting the IR at each `FUNC_START`/`FUNC_END` pair), starting from
`main` and following `CALL`s on demand, memoizing each function's cost so
it's never recomputed. A function's cost is only ever computed if `main`
can actually reach it - an unreachable function contributes nothing. When
a call is encountered inside a loop or another function, the callee's
already-computed cost is scaled by whatever loop multiplier is active at
that call site. Recursion (direct or mutual) is detected - when a
function's cost calculation would call back into a function still being
computed, that function's own non-recursive tally is instead multiplied
by `kAssumedRecursionDepth` (a named, illustrative placeholder, exactly
like `kAssumedLoopIterations` for loops - see *Assumptions*). `if`/`else`
counts both branches as if they always execute (a documented
over-approximation, not real control-flow simulation), and `Time
Complexity` reflects only `main`'s own loop nesting, treating every call
as opaque - consistent with there being no other interprocedural analysis
anywhere in the compiler.

## Code generation

[include/codegen.h](include/codegen.h) /
[src/codegen.cpp](src/codegen.cpp). `CodeGenerator::generate` takes the
optimized IR and emits a simple, readable pseudo-assembly text listing -
**not** real machine code, real assembly, or LLVM IR:

```
STORE x = 5                    -> MOV x, 5
BINARY_OP $t1 = 2 + 3          -> ADD $t1, 2, 3   (SUB/MUL/DIV likewise)
COMPARE $t1 = a < b            -> CMPLT $t1, a, b
PRINT "x = %d", x              -> PRINT "x = %d", x
LOOP_START 5 / LOOP_END        -> LOOP_START 5 / LOOP_END
IF_START $t1 / ELSE / IF_END   -> IF_START $t1 / ELSE / IF_END
FUNC_START f(a, b) / FUNC_END f -> FUNC_START f(a, b) / FUNC_END f
CALL $t1 = f, 2, 3             -> CALL $t1, f, 2, 3
RETURN x                       -> RETURN x
```

Instruction order and structure are always preserved exactly. An
unrecognized IR opcode or operator raises a clear, line-numbered `Code
generation error on line N: ...` instead of silently emitting something
wrong.

## Project structure

```
EcoLang/
  include/    header files (one per module)
  src/        implementation files, plus main.cpp (the driver)
  samples/    example .c programs (hello.c, loops.c, nested.c)
  tests/      small, single-purpose .c test programs (see tests/README.md)
  build/      compiled output (gitignored)
  CMakeLists.txt
  README.md
```

## How to build

With `g++` directly (no external dependencies):

```sh
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude \
    src/main.cpp src/lexer.cpp src/parser.cpp src/semantic.cpp \
    src/ir.cpp src/energy.cpp src/optimizer.cpp src/codegen.cpp \
    src/report.cpp src/dump.cpp -o build/ecolang
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
./build/ecolang <path-to-file.c>
```

Passing no file prints a usage message and exits 1. Passing a file that
can't be opened, or a program that fails to lex/parse/type-check, prints a
single clear error message and exits 1.

## Sample programs

- [samples/hello.c](samples/hello.c) - declarations, arithmetic, a
  function with parameters and a return value, and `printf`
- [samples/loops.c](samples/loops.c) - a `while` loop accumulating a
  running total
- [samples/nested.c](samples/nested.c) - nested `while` loops (a small
  multiplication table)

## Testing

[tests/](tests/) contains 28 small, single-purpose `.c` programs - 18
valid programs, each demonstrating one language feature, and 10 invalid
programs, each triggering exactly one error category (see
[tests/README.md](tests/README.md) for the full list). Every valid
program should compile successfully (exit 0); every invalid program
should fail with a single clear error message and exit 1, without any
later pipeline stage running.

## Example EcoLang program

```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    printf("%d\n", result);
    return 0;
}
```

## Assumptions and limitations

These are deliberate, documented decisions made while building EcoLang -
not requirements handed down by an external spec, except where noted:

- **No block-level scoping.** Each function has one flat local scope,
  seeded with its parameters. A variable declared inside an `if`/`while`/
  `for` is visible for the rest of that *function* - the same
  simplification the earlier phases used for the whole program, now
  scoped per function instead.
- **Variables cannot be reassigned in the same statement they're
  declared in** - only via a separate `x = value;` assignment - and
  declaring the same name twice in one function is a semantic error.
- **`while`/`for` loops always compile to a fixed, assumed iteration
  count** (`kAssumedLoopIterations`, currently 10), regardless of their
  real runtime condition - there is no static loop-bound analysis. The
  condition is still fully checked for validity in semantic analysis; it
  just isn't part of the generated IR, since the energy/codegen model
  intentionally abstracts every loop as "runs N times."
- **Recursion depth is similarly assumed**, via `kAssumedRecursionDepth`
  (currently 10) - there is no attempt to analyze how deep a recursive
  call chain actually goes.
- **No reachability/return-path analysis.** A non-`int`-void function is
  not verified to return a value on every code path - a bare `return;`
  inside an `int` function is accepted without further checking, matching
  how real compilers historically handled this before more advanced flow
  analysis existed.
- **`if`/`else` energy accounting counts both branches as if they always
  ran** - a worst-case over-approximation, not real control-flow
  simulation, which keeps the energy analyzer a single flat pass with no
  control-flow graph.
- **No interprocedural optimization** - constant folding/propagation
  never crosses a `CALL`; each function is optimized using only its own
  body.
- **Time complexity is a structural loop-nesting heuristic**, based only
  on `main`'s own maximum loop nesting depth, not a full asymptotic
  analysis - it does not attempt to trace complexity through function
  calls.
- **1 energy unit = `1e-7` J is an illustrative scaling factor**, not a
  hardware measurement, purely to give the report a human-readable
  Joules figure.
- **Loop overhead is modeled as 1 energy unit per iteration** (named
  `kLoopOverheadUnitsPerIteration`, changeable in one place).
- There is no unary minus, so negative number literals (`-5`) are not
  supported.
- Error recovery stops at the first error in each stage rather than
  collecting and reporting multiple errors at once.

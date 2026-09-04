# EcoLang test programs

Small, single-purpose `.c` programs exercising the C-subset compiler. Each
one is run directly with `./build/ecolang tests/<file>.c`.

## Valid programs (expect exit code 0)

| File | Demonstrates |
|---|---|
| `01_declaration.c` | a variable declaration with no initializer |
| `02_initialized_declaration.c` | a variable declaration with an initializer |
| `03_assignment.c` | assigning a new value to an existing variable |
| `04_arithmetic.c` | `+ - * /`, operator precedence, and parentheses |
| `05_printf.c` | `printf` with a format argument and with a plain string |
| `06_return.c` | `return` with a value |
| `07_if.c` | a single `if` |
| `08_if_else.c` | `if` / `else` |
| `09_while.c` | a `while` loop |
| `10_for.c` | a `for` loop |
| `11_comparisons.c` | all six comparison operators: `== != < <= > >=` |
| `12_function_declaration.c` | declaring and calling a function with no parameters |
| `13_function_parameters.c` | a function with parameters |
| `14_function_call.c` | a `void` function called as a standalone statement |
| `15_recursion.c` | a function calling itself (factorial) |
| `16_mutual_recursion.c` | two functions calling each other (`isOdd`/`isEven`) |
| `17_nested_control_flow.c` | an `if`/`else` nested inside a `while` |
| `18_comments.c` | `//` line comments and `/* */` block comments |

## Invalid programs (expect exit code 1)

| File | Expected error stage |
|---|---|
| `19_missing_semicolon.c` | syntax error - missing `;` |
| `20_undeclared_variable.c` | semantic error - variable used before declaration |
| `21_duplicate_variable.c` | semantic error - the same variable declared twice |
| `22_undeclared_function.c` | semantic error - calling a function that doesn't exist |
| `23_wrong_arg_count.c` | semantic error - call with the wrong number of arguments |
| `24_duplicate_function.c` | semantic error - the same function declared twice |
| `25_missing_main.c` | semantic error - no `main` function in the program |
| `26_void_as_value.c` | semantic error - using a `void` call's (nonexistent) result as a value |
| `27_malformed_condition.c` | syntax error - a condition that isn't a comparison |
| `28_invalid_character.c` | lexical error - a character the language doesn't recognize |

Every "invalid" case should print a single, clear, line-numbered (where
applicable) error message to stderr and exit with status 1 - none of them
should crash the compiler or produce partial output from later stages.

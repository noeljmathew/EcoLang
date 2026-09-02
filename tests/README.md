# EcoLang test programs

Small, single-purpose `.eco` programs for exercising the compiler. Each one
is run directly with `./build/ecolang tests/<file>.eco`.

| File | Expected result |
|---|---|
| `01_let.eco` | succeeds - basic variable declaration |
| `02_print.eco` | succeeds - declare then print |
| `03_precedence.eco` | succeeds - `2 + 3 * 4` must evaluate as `2 + (3 * 4)` |
| `04_parentheses.eco` | succeeds - `(2 + 3) * 4` must evaluate as grouped |
| `05_string_concat.eco` | succeeds - `string + string` concatenation |
| `06_repeat.eco` | succeeds - a single `repeat ... end` loop |
| `07_nested_repeat.eco` | succeeds - a `repeat` nested inside another `repeat` |
| `08_undeclared_variable.eco` | **fails** - semantic error, variable used before declaration |
| `09_redeclaration.eco` | **fails** - semantic error, `x` declared twice |
| `10_type_mismatch.eco` | **fails** - semantic error, `int + string` is not a valid operation |
| `11_invalid_character.eco` | **fails** - lexical error, `@` is not a valid EcoLang character |
| `12_missing_end.eco` | **fails** - syntax error, `repeat` block never closed with `end` |

Every "fails" case should print a single, line-numbered error message to
stderr and exit with a non-zero status - none of them should crash the
compiler.

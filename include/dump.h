#ifndef DUMP_H
#define DUMP_H

#include <string>
#include <vector>

#include "ast.h"
#include "token.h"

// Renders the lexer's already-produced token stream as one line per
// token: "<line> <TYPE> <lexeme>" (the trailing lexeme is omitted -
// not even a trailing space - for END_OF_FILE, whose lexeme is always
// empty). Line first and type second, both single "words" with no
// internal whitespace, so a consumer can safely split each line on
// whitespace with a max of 2 splits and take the remainder verbatim as
// the lexeme, even for a STRING token whose lexeme itself contains
// spaces (e.g. a printf format string).
std::string dumpTokens(const std::vector<Token>& tokens);

// Renders the parser's already-produced AST as JSON, matching the
// schema webapp/backend/ast_parser.py (the independent, toy
// visualization parser this replaces) used to produce, field name for
// field name, so the existing frontend tree renderer needs no changes -
// only the data source changes. See dump.cpp for the one genuine shape
// difference this real AST cannot avoid (Program has no source line -
// neither did the toy parser's Program node, so this is actually a
// match, not a mismatch - see the comment above dumpAst there for the
// full reasoning).
std::string dumpAst(const Program& program);

#endif

"""Flask backend for the EcoLang webapp.

This is a thin wrapper around the existing, already-built EcoLang compiler
executable (build/ecolang). It does not reimplement any compiler behavior:
it writes the submitted source to a temporary .c file, runs the compiler
executable against it, and turns the compiler's own stdout/stderr into JSON.
"""

import json
import os
import re
import subprocess
import tempfile

from flask import Flask, jsonify, request
from flask_cors import CORS

BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, "..", ".."))
COMPILER_PATH = os.path.join(REPO_ROOT, "build", "ecolang")

app = Flask(__name__)
CORS(app)


DECIMAL_VALUE_PATTERN = re.compile(r"-?\d+\.\d+")


def parse_key_value_lines(block):
    """Parse every "Key : Value" line in a block of compiler output into a
    dict, converting pure-integer values to int and pure-decimal values
    (e.g. the energy model's nJ figures, like "378.942808") to float.
    Lines without a colon (banners, dashed rules, subsection headings) are
    skipped. A value with any other trailing text (e.g. "0.0000004 J")
    does not match either pattern and is kept as the original string, so
    no precision is ever rounded, truncated, or dropped in the process."""
    result = {}
    for line in block.splitlines():
        if ":" not in line:
            continue
        key, _, value = line.partition(":")
        key = key.strip()
        value = value.strip()
        if not key or not value:
            continue
        if value.lstrip("-").isdigit():
            result[key] = int(value)
        elif DECIMAL_VALUE_PATTERN.fullmatch(value):
            result[key] = float(value)
        else:
            result[key] = value
    return result


def strip_section_underline(block):
    """Drop the "-----------------" underline the compiler prints right
    below a section heading, and surrounding blank lines, from a raw text
    block such as the code-generation output."""
    block = block.strip("\n")
    first_line, _, rest = block.partition("\n")
    if first_line and set(first_line) == {"-"}:
        block = rest
    return block.strip("\n")


def parse_tokens_block(block):
    """Parse the [Tokens] section's "<line> <TYPE> <lexeme>" lines (one
    per token - see dumpTokens in src/dump.cpp) into a list of
    {"type", "lexeme", "line"} dicts. Split with maxsplit=2 so a lexeme
    containing internal spaces (e.g. a printf format string, quotes and
    all) survives intact; END_OF_FILE's line has no trailing lexeme at
    all, which naturally yields an empty string for it."""
    tokens = []
    for line in strip_section_underline(block).splitlines():
        if not line.strip():
            continue
        parts = line.split(" ", 2)
        tokens.append({
            "line": int(parts[0]),
            "type": parts[1],
            "lexeme": parts[2] if len(parts) > 2 else "",
        })
    return tokens


def parse_ast_block(block):
    """Parse the [AST] section's raw JSON text (dumpAst in src/dump.cpp)
    into a Python object, confirming it is actually valid JSON rather than
    blindly string-forwarding it - json.loads raises (and this propagates
    out of compile_source uncaught, a 500) if the compiler ever emits
    something malformed here, since that would be a compiler bug, not
    something this parsing layer should silently paper over."""
    return json.loads(strip_section_underline(block))


def parse_compiler_stdout(stdout):
    """Split the compiler's stdout into the [Tokens], [AST],
    [Optimization], [IR Before Optimization], [Code Generation], and
    [Energy Report] sections it actually prints, in that order, and parse
    each one. Every section's block runs up to whichever later marker
    appears next in stdout, skipping over any marker that isn't present -
    the same "missing sections don't block later ones" behavior this
    function always had, now generalized instead of hand-chained per pair
    of markers now that there are six sections instead of four."""
    markers = [
        "[Tokens]",
        "[AST]",
        "[Optimization]",
        "[IR Before Optimization]",
        "[Code Generation]",
        "[Energy Report]",
    ]
    positions = [stdout.find(marker) for marker in markers]

    blocks = {}
    for i, marker in enumerate(markers):
        start = positions[i]
        if start == -1:
            blocks[marker] = ""
            continue
        start += len(marker)
        end = next((p for p in positions[i + 1:] if p != -1), len(stdout))
        blocks[marker] = stdout[start:end]

    return {
        "tokens": parse_tokens_block(blocks["[Tokens]"]),
        "ast": parse_ast_block(blocks["[AST]"]),
        "optimization": parse_key_value_lines(blocks["[Optimization]"]),
        "irBefore": strip_section_underline(blocks["[IR Before Optimization]"]),
        "codegen": strip_section_underline(blocks["[Code Generation]"]),
        "energy": parse_key_value_lines(blocks["[Energy Report]"]),
    }


@app.route("/compile", methods=["POST"])
def compile_source():
    body = request.get_json(silent=True) or {}
    source = body.get("source")

    if not isinstance(source, str):
        return jsonify({"success": False, "error": "Request body must include a 'source' string."}), 400

    if not os.path.isfile(COMPILER_PATH):
        return jsonify({
            "success": False,
            "error": f"EcoLang compiler executable not found at {COMPILER_PATH}. Build it first.",
        }), 500

    tmp_path = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as tmp_file:
            tmp_file.write(source)
            tmp_path = tmp_file.name

        result = subprocess.run(
            [COMPILER_PATH, tmp_path],
            capture_output=True,
            text=True,
            timeout=10,
        )

        if result.returncode != 0:
            return jsonify({"success": False, "error": result.stderr})

        parsed = parse_compiler_stdout(result.stdout)
        response = {
            "success": True,
            "tokens": parsed["tokens"],
            "ast": parsed["ast"],
            "optimization": parsed["optimization"],
            "energy": parsed["energy"],
            "codegen": parsed["codegen"],
            "irBefore": parsed["irBefore"],
        }
        return jsonify(response)
    except subprocess.TimeoutExpired:
        return jsonify({"success": False, "error": "Compilation timed out."}), 500
    finally:
        if tmp_path and os.path.exists(tmp_path):
            os.remove(tmp_path)


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5001, debug=True)

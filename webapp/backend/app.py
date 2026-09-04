"""Flask backend for the EcoLang webapp.

This is a thin wrapper around the existing, already-built EcoLang compiler
executable (build/ecolang). It does not reimplement any compiler behavior:
it writes the submitted source to a temporary .c file, runs the compiler
executable against it, and turns the compiler's own stdout/stderr into JSON.
"""

import os
import subprocess
import tempfile

from flask import Flask, jsonify, request
from flask_cors import CORS

BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(BACKEND_DIR, "..", ".."))
COMPILER_PATH = os.path.join(REPO_ROOT, "build", "ecolang")

app = Flask(__name__)
CORS(app)


def parse_key_value_lines(block):
    """Parse every "Key : Value" line in a block of compiler output into a
    dict, converting pure-integer values to int. Lines without a colon
    (banners, dashed rules, subsection headings) are skipped."""
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
        else:
            result[key] = value
    return result


def parse_compiler_stdout(stdout):
    """Split the compiler's stdout into the [Optimization], [Code Generation],
    and [Energy Report] sections it actually prints, and parse each one."""
    opt_marker = "[Optimization]"
    codegen_marker = "[Code Generation]"
    energy_marker = "[Energy Report]"

    opt_start = stdout.find(opt_marker)
    codegen_start = stdout.find(codegen_marker)
    energy_start = stdout.find(energy_marker)

    optimization_block = ""
    codegen_block = ""
    energy_block = ""

    if opt_start != -1 and codegen_start != -1:
        optimization_block = stdout[opt_start + len(opt_marker):codegen_start]
    if codegen_start != -1 and energy_start != -1:
        codegen_block = stdout[codegen_start + len(codegen_marker):energy_start]
    if energy_start != -1:
        energy_block = stdout[energy_start + len(energy_marker):]

    return {
        "optimization": parse_key_value_lines(optimization_block),
        "codegen": strip_section_underline(codegen_block),
        "energy": parse_key_value_lines(energy_block),
    }


def strip_section_underline(block):
    """Drop the "-----------------" underline the compiler prints right
    below a section heading, and surrounding blank lines, from a raw text
    block such as the code-generation output."""
    block = block.strip("\n")
    first_line, _, rest = block.partition("\n")
    if first_line and set(first_line) == {"-"}:
        block = rest
    return block.strip("\n")


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
        return jsonify({
            "success": True,
            "optimization": parsed["optimization"],
            "energy": parsed["energy"],
            "codegen": parsed["codegen"],
        })
    except subprocess.TimeoutExpired:
        return jsonify({"success": False, "error": "Compilation timed out."}), 500
    finally:
        if tmp_path and os.path.exists(tmp_path):
            os.remove(tmp_path)


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5001, debug=True)

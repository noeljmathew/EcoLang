// Talks to the Flask backend in webapp/backend/app.py.
export const BACKEND_URL = "http://localhost:5001/compile";

// POSTs `source` to /compile and returns the parsed JSON response
// (the shape compile_source() in webapp/backend/app.py returns: success,
// error, optimization, energy, codegen, irBefore, ast, tokens). Throws
// on a network-level failure (backend unreachable) - callers decide how
// to surface that; a non-2xx HTTP status still resolves normally here
// since /compile always returns JSON with its own success/error fields
// regardless of status code.
export async function compileSource(source) {
  const response = await fetch(BACKEND_URL, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ source }),
  });
  return response.json();
}

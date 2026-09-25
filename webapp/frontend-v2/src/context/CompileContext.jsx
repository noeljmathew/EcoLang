import { createContext, useCallback, useContext, useState } from "react";
import { BACKEND_URL, compileSource } from "../api";

const STORAGE_KEY = "ecolang:lastCompile";

const CompileContext = createContext(null);

// Loaded once, synchronously, as useState's initializer - so the very
// first render already has whatever was persisted from a previous
// session, with no loading flicker for stage pages.
function loadPersisted() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    return raw ? JSON.parse(raw) : null;
  } catch {
    // Corrupt JSON, or localStorage unavailable (private mode, quota) -
    // either way, fall back to "nothing compiled yet" rather than throw.
    return null;
  }
}

export function CompileProvider({ children }) {
  // The full last *successful* /compile response (success/optimization/
  // energy/codegen/irBefore/ast/tokens) - every stage page reads this.
  // Deliberately separate from the compile page's own in-flight/error
  // feedback below: a failed compile attempt must not blank out the
  // last good result other pages are still showing.
  const [lastResult, setLastResult] = useState(loadPersisted);

  // Feedback about the most recent compile *attempt*, which may be a
  // failure even while lastResult still holds an earlier success.
  const [status, setStatus] = useState("idle"); // idle | loading | success | error
  const [errorMessage, setErrorMessage] = useState(null);

  const runCompile = useCallback(async (source) => {
    setStatus("loading");
    setErrorMessage(null);

    let data;
    try {
      data = await compileSource(source);
    } catch (err) {
      // Network-level failure (backend unreachable, timed out, etc.) -
      // no response to read data.success from at all.
      setStatus("error");
      setErrorMessage("Could not reach the backend at " + BACKEND_URL + "\n\n" + err);
      throw err;
    }

    if (data.success) {
      setLastResult(data);
      try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(data));
      } catch {
        // Persistence failed (private mode, quota) - the in-memory
        // lastResult is still set for this session, only survival
        // across a reload is lost.
      }
      setStatus("success");
    } else {
      setStatus("error");
      setErrorMessage(data.error || "Compilation failed.");
    }

    return data;
  }, []);

  const value = { lastResult, status, errorMessage, runCompile };
  return <CompileContext.Provider value={value}>{children}</CompileContext.Provider>;
}

export function useCompile() {
  const ctx = useContext(CompileContext);
  if (!ctx) {
    throw new Error("useCompile must be used within a CompileProvider");
  }
  return ctx;
}

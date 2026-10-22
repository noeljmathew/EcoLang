import { createContext, useCallback, useContext, useState } from "react";
import { useCompile } from "./CompileContext";
import { DEFAULT_FILE, SAMPLES } from "../samples";

const SourceContext = createContext(null);

// Holds the editor's source text and the current file name, so the
// top bar's Compile button and Cmd/Ctrl+Enter work from any page and
// edits survive navigation. Compiling still goes through CompileContext.
export function SourceProvider({ children }) {
  const [fileName, setFileName] = useState(DEFAULT_FILE);
  const [source, setSource] = useState(SAMPLES[DEFAULT_FILE]);
  const { status, runCompile } = useCompile();

  const loadExample = useCallback((name) => {
    if (SAMPLES[name] === undefined) return;
    setFileName(name);
    setSource(SAMPLES[name]);
  }, []);

  const compile = useCallback(async () => {
    if (status === "loading") return;
    try {
      await runCompile(source);
    } catch {
      // runCompile already recorded the failure in CompileContext
      // (status and errorMessage), which the UI reads.
    }
  }, [runCompile, source, status]);

  // True once the text differs from the example it was loaded from.
  const edited = source !== SAMPLES[fileName];

  const value = { fileName, edited, source, setSource, loadExample, compile };
  return <SourceContext.Provider value={value}>{children}</SourceContext.Provider>;
}

export function useSource() {
  const ctx = useContext(SourceContext);
  if (!ctx) {
    throw new Error("useSource must be used within a SourceProvider");
  }
  return ctx;
}

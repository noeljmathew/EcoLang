// Monaco setup, run once before any editor mounts.
//
// Offline by design: @monaco-editor/react normally downloads Monaco from
// a CDN, but loader.config({ monaco }) hands it the copy bundled from
// node_modules instead. Only the core editor API, the C language and the
// one generic editor worker are imported, not Monaco's full language set.
import { loader } from "@monaco-editor/react";
import * as monaco from "monaco-editor/editor/editor.api";
import "monaco-editor/languages/definitions/cpp/register";
// Three editor features from Monaco's full build: find (Cmd/Ctrl+F),
// bracket matching, and word-based suggestions. Suggestions are computed
// in the editor worker below.
import "monaco-editor/editor/contrib/find/browser/findController";
import "monaco-editor/editor/contrib/bracketMatching/browser/bracketMatching";
import "monaco-editor/editor/contrib/suggest/browser/suggestController";
import EditorWorker from "monaco-editor/editor/editor.worker?worker";

// Vite's "?worker" import bundles the worker as a local file, so this
// never fetches anything from the network.
self.MonacoEnvironment = {
  getWorker() {
    return new EditorWorker();
  },
};

// The pseudo-assembly printed by src/codegen.cpp (used on the IR and
// Code Generation pages): opcodes, $t temporaries, numbers and strings.
monaco.languages.register({ id: "ecoasm" });
monaco.languages.setMonarchTokensProvider("ecoasm", {
  tokenizer: {
    root: [
      [/\b(FUNC_START|FUNC_END|LOOP_START|LOOP_END|IF_START|IF_END|ELSE|CALL|RETURN)\b/, "keyword.control"],
      [/\b(MOV|ADD|SUB|MUL|DIV|CMPEQ|CMPNE|CMPLT|CMPLE|CMPGT|CMPGE|PRINT)\b/, "keyword"],
      [/\$t\d+/, "variable.temp"],
      [/-?\d+/, "number"],
      [/"([^"\\]|\\.)*"/, "string"],
      [/[A-Za-z_]\w*/, "identifier"],
    ],
  },
});

// One dark theme for every editor, matching the tokens in index.css.
monaco.editor.defineTheme("ecolang-dark", {
  base: "vs-dark",
  inherit: true,
  rules: [
    { token: "", foreground: "e6e6e8" },
    { token: "comment", foreground: "6b6c72" },
    { token: "keyword", foreground: "c4b5fd" },
    { token: "keyword.control", foreground: "93c5fd" },
    { token: "variable.temp", foreground: "fda4af" },
    { token: "number", foreground: "fcd34d" },
    { token: "string", foreground: "86efac" },
    { token: "type", foreground: "93c5fd" },
    { token: "identifier", foreground: "e6e6e8" },
    { token: "delimiter", foreground: "9a9ba1" },
  ],
  colors: {
    "editor.background": "#17181b",
    "editor.foreground": "#e6e6e8",
    "editorLineNumber.foreground": "#4b4c52",
    "editorLineNumber.activeForeground": "#9a9ba1",
    "editor.lineHighlightBackground": "#1d1e22",
    "editor.lineHighlightBorder": "#1d1e22",
    "editor.selectionBackground": "#3a3b42",
    "editorCursor.foreground": "#e6e6e8",
    "editorIndentGuide.background1": "#232428",
    "editorWidget.background": "#1d1e22",
    "editorWidget.border": "#2a2b30",
    "scrollbarSlider.background": "#ffffff20",
    "scrollbarSlider.hoverBackground": "#ffffff30",
  },
});

// Monaco measures character widths once. If Geist Mono finishes loading
// after that, re-measure so the cursor and selections line up.
document.fonts?.addEventListener("loadingdone", () => monaco.editor.remeasureFonts());

loader.config({ monaco });

export { monaco };

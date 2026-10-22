import Editor from "@monaco-editor/react";
import { EDITOR_OPTIONS } from "../lib/editorOptions";

// Read-only Monaco view of compiler text, by default the pseudo-assembly
// language ("ecoasm") registered in lib/monaco.js. `onMount` gives
// IrPage access to the editor for its removed-line highlights.
export default function CodeView({ value, language = "ecoasm", onMount }) {
  return (
    <Editor
      value={value}
      language={language}
      theme="ecolang-dark"
      onMount={onMount}
      loading={<div className="p-4 text-[12px] text-faint">Loading editor</div>}
      options={{ ...EDITOR_OPTIONS, readOnly: true, domReadOnly: true, contextmenu: false }}
    />
  );
}

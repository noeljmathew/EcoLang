import { useEffect, useRef } from "react";
import Editor from "@monaco-editor/react";
import { useSource } from "../../context/SourceContext";
import { monaco } from "../../lib/monaco";
import { EDITOR_OPTIONS } from "../../lib/editorOptions";

// The editable C source. Monaco handles Cmd/Ctrl+Enter itself while
// focused, so the shortcut is bound here as well as in TopBar. The ref
// keeps the binding pointed at the latest compile() without rebinding.
export default function SourceEditor() {
  const { source, setSource, compile } = useSource();
  const compileRef = useRef(compile);

  useEffect(() => {
    compileRef.current = compile;
  }, [compile]);

  function handleMount(editor) {
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => compileRef.current());
  }

  return (
    <Editor
      value={source}
      onChange={(value) => setSource(value ?? "")}
      language="c"
      theme="ecolang-dark"
      onMount={handleMount}
      loading={<div className="p-4 text-[12px] text-faint">Loading editor</div>}
      options={{ ...EDITOR_OPTIONS, tabSize: 4 }}
    />
  );
}

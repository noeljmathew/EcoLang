import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import CodeView from "../components/CodeView";
import { PanelHeader } from "../components/Panel";

// The optimized program's pseudo-assembly, shown in a read-only editor.
export default function CodegenPage() {
  const { lastResult } = useCompile();
  const codegen = lastResult?.codegen;

  return (
    <section className="flex h-full min-h-[480px] flex-col">
      <PageHeading
        title="Code generation"
        description="The optimized IR rendered as pseudo-assembly. This is not real machine code."
      />
      {codegen === undefined ? (
        <EmptyState />
      ) : (
        <div className="flex min-h-0 flex-1 flex-col">
          <PanelHeader label="Pseudo-assembly" className="bg-panel">
            <span className="font-mono">{codegen.split("\n").length} lines</span>
          </PanelHeader>
          <div className="min-h-0 flex-1">
            <CodeView value={codegen} />
          </div>
        </div>
      )}
    </section>
  );
}

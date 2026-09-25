import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";

// Ported as-is from the old index.html's Code Generation tab: the
// optimized program's pseudo-assembly, shown plainly.
export default function CodegenPage() {
  const { lastResult } = useCompile();
  const codegen = lastResult?.codegen;

  if (codegen === undefined) {
    return (
      <section>
        <h1>Code Generation</h1>
        <EmptyState />
      </section>
    );
  }

  return (
    <section>
      <h1>Code Generation</h1>
      <pre>{codegen}</pre>
    </section>
  );
}

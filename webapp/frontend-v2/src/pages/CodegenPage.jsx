import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";

// The optimized program's pseudo-assembly, shown plainly.
export default function CodegenPage() {
  const { lastResult } = useCompile();
  const codegen = lastResult?.codegen;

  return (
    <section>
      <PageHeading title="Code Generation" description="The optimized IR, rendered as readable pseudo-assembly." />
      {codegen === undefined ? (
        <EmptyState />
      ) : (
        <pre className="overflow-x-auto rounded-xl border border-accent-soft-border bg-paper-raised p-5 font-mono text-[13px] leading-6 whitespace-pre text-ink shadow-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark dark:shadow-soft-dark">
          {codegen}
        </pre>
      )}
    </section>
  );
}

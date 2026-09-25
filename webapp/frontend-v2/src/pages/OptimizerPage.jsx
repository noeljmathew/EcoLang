import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";

// Ported from the old index.html's Optimization tab: a headline
// "before -> after" instruction-count figure, plus the individual pass
// counts. Reads the same data.optimization fields the old table did.
const PASS_LABELS = [
  "Constant Folding",
  "Constant Propagation",
  "Algebraic Simplification",
  "Dead Temporary Elimination",
];

export default function OptimizerPage() {
  const { lastResult } = useCompile();
  const optimization = lastResult?.optimization;

  if (!optimization) {
    return (
      <section>
        <PageHeading title="Optimizer" description="Constant folding, propagation, and dead-code elimination stats." />
        <EmptyState />
      </section>
    );
  }

  const original = optimization["Original IR Instructions"];
  const optimized = optimization["Optimized IR Instructions"];
  const removed = optimization["Instructions Removed"];
  const hasHeadline = typeof original === "number" && typeof optimized === "number";
  const pct = hasHeadline && original > 0 ? (removed / original) * 100 : 0;
  const removedWord = removed === 1 ? "instruction" : "instructions";

  return (
    <section>
      <PageHeading title="Optimizer" description="Constant folding, propagation, and dead-code elimination stats." />

      {hasHeadline && (
        <div className="mb-8 rounded-xl border border-accent-soft-border bg-paper-raised p-6 shadow-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:shadow-soft-dark">
          <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
            IR instructions
          </div>
          <div className="font-display mt-1 text-3xl font-semibold text-ink dark:text-ink-dark">
            {original} <span className="text-stage-blue dark:text-stage-blue-dark">→</span> {optimized}
          </div>
          <div className="mt-2 text-sm text-ink-soft dark:text-ink-dark-soft">
            {removed} {removedWord} removed · {pct.toFixed(1)}% reduction
          </div>
        </div>
      )}

      <div className="divide-y divide-accent-soft-border rounded-xl border border-accent-soft-border shadow-soft dark:divide-accent-dark-soft dark:border-accent-dark-soft dark:shadow-soft-dark">
        {PASS_LABELS.filter((key) => key in optimization).map((key) => (
          <div key={key} className="flex items-center justify-between px-5 py-3">
            <span className="text-ink-soft dark:text-ink-dark-soft">{key}</span>
            <span className="font-mono text-ink dark:text-ink-dark">{optimization[key]}</span>
          </div>
        ))}
      </div>
    </section>
  );
}

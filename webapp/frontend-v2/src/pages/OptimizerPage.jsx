import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";

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
        <h1>Optimizer</h1>
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
      <h1>Optimizer</h1>

      {hasHeadline && (
        <div className="opt-headline">
          <div className="opt-headline-label">IR instructions</div>
          <div className="opt-headline-figure">{original} → {optimized}</div>
          <div className="opt-headline-detail">
            {removed} {removedWord} removed · {pct.toFixed(1)}% reduction
          </div>
        </div>
      )}

      <div className="opt-passes">
        {PASS_LABELS.filter((key) => key in optimization).map((key) => (
          <div className="opt-pass-row" key={key}>
            <span>{key}</span>
            <span>{optimization[key]}</span>
          </div>
        ))}
      </div>
    </section>
  );
}

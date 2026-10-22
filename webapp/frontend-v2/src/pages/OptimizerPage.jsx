import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import { PageBody, Panel, PanelHeader, Stat } from "../components/Panel";

// A headline "before -> after" instruction-count figure, plus the
// individual pass counts, from data.optimization.
const PASS_LABELS = [
  "Constant Folding",
  "Constant Propagation",
  "Algebraic Simplification",
  "Dead Temporary Elimination",
];

const DESCRIPTION = "Constant folding, constant propagation, algebraic simplification and dead-temporary elimination.";

export default function OptimizerPage() {
  const { lastResult } = useCompile();
  const optimization = lastResult?.optimization;

  if (!optimization) {
    return (
      <section>
        <PageHeading title="Optimizer" description={DESCRIPTION} />
        <EmptyState />
      </section>
    );
  }

  const original = optimization["Original IR Instructions"];
  const optimized = optimization["Optimized IR Instructions"];
  const removed = optimization["Instructions Removed"];
  const hasHeadline = typeof original === "number" && typeof optimized === "number";
  const pct = hasHeadline && original > 0 ? (removed / original) * 100 : 0;

  return (
    <section>
      <PageHeading title="Optimizer" description={DESCRIPTION} />
      <PageBody>
        {hasHeadline && (
          <Panel className="grid grid-cols-1 gap-px bg-line sm:grid-cols-3">
            <Stat label="IR instructions" value={original + " → " + optimized} />
            <Stat label="Instructions removed" value={removed} />
            <Stat label="Reduction" value={pct.toFixed(1) + "%"} />
          </Panel>
        )}
        <Panel>
          <PanelHeader label="Pass counts" />
          <table className="w-full border-collapse text-[13px]">
            <tbody>
              {PASS_LABELS.filter((key) => key in optimization).map((key) => (
                <tr key={key} className="border-b border-line last:border-0">
                  <td className="px-3 py-2 text-muted">{key}</td>
                  <td className="px-3 py-2 text-right font-mono text-fg">{optimization[key]}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </Panel>
      </PageBody>
    </section>
  );
}

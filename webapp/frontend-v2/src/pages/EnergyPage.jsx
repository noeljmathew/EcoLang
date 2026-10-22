import { motion } from "framer-motion";
import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import WaterTeaser from "../components/WaterTeaser";

// Energy Report page: headline stats,
// the five per-category operation-count bars (scaled to the largest of
// the five for this specific compile, not a fixed global max), and the
// full key/value table.
const ENERGY_BAR_FIELDS = ["Assignments", "Arithmetic Ops", "Comparisons", "Loop Iterations", "Print Statements"];

export default function EnergyPage() {
  const { lastResult } = useCompile();
  const energy = lastResult?.energy;
  const energyBefore = lastResult?.energyBefore;

  if (!energy) {
    return (
      <section>
        <PageHeading title="Energy Report" description="Estimated energy cost, from the real Georgiou et al. Cortex-M0 hardware model." />
        <EmptyState />
      </section>
    );
  }

  const barValues = ENERGY_BAR_FIELDS.map((key) => Number(energy[key]) || 0);
  const barMax = Math.max(...barValues, 1);
  const tableEntries = Object.entries(energy).filter(
    ([key]) => key !== "Estimated Energy" && key !== "Time Complexity"
  );

  return (
    <section>
      <PageHeading title="Energy Report" description="Estimated energy cost, from the real Georgiou et al. Cortex-M0 hardware model." />

      <div className="mb-8 flex flex-wrap gap-10">
        <div>
          <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
            Estimated Energy
          </div>
          <div className="font-display mt-1 text-3xl font-semibold text-stage-emerald dark:text-stage-emerald-dark">
            {energy["Estimated Energy"] ?? "-"}
          </div>
        </div>
        <div>
          <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
            Time Complexity
          </div>
          <div className="font-display mt-1 text-3xl font-semibold text-ink dark:text-ink-dark">
            {energy["Time Complexity"] ?? "-"}
          </div>
        </div>
      </div>

      <div className="mb-8 flex flex-col gap-3">
        {ENERGY_BAR_FIELDS.map((key, i) => (
          <div key={key} className="grid grid-cols-[140px_1fr_64px] items-center gap-3 text-sm">
            <span className="text-ink-soft dark:text-ink-dark-soft">{key}</span>
            <div className="h-2 overflow-hidden rounded-full bg-stage-emerald/12 dark:bg-stage-emerald-dark/15">
              <motion.div
                className="h-full rounded-full bg-stage-emerald dark:bg-stage-emerald-dark"
                initial={{ width: 0 }}
                animate={{ width: (barValues[i] / barMax) * 100 + "%" }}
                transition={{ duration: 0.6, delay: i * 0.08, ease: "easeOut" }}
              />
            </div>
            <span className="text-right font-mono text-ink dark:text-ink-dark">{barValues[i]}</span>
          </div>
        ))}
      </div>

      <div className="divide-y divide-accent-soft-border rounded-xl border border-accent-soft-border shadow-soft dark:divide-accent-dark-soft dark:border-accent-dark-soft dark:shadow-soft-dark">
        {tableEntries.map(([key, value]) => (
          <div key={key} className="flex items-center justify-between px-5 py-2.5 text-sm">
            <span className="text-ink-soft dark:text-ink-dark-soft">{key}</span>
            <span className="font-mono text-ink dark:text-ink-dark">{value}</span>
          </div>
        ))}
      </div>

      <WaterTeaser energy={energy} energyBefore={energyBefore} />
    </section>
  );
}

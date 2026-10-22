import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import WaterTeaser from "../components/WaterTeaser";
import { PageBody, Panel, PanelHeader, Stat } from "../components/Panel";

// The five per-category energy fields printed by the compiler's energy
// report (src/report.cpp), in nJ.
const ENERGY_CATEGORIES = [
  ["Assignment Energy", "Assignments"],
  ["Arithmetic Energy", "Arithmetic"],
  ["Comparison Energy", "Comparisons"],
  ["Loop Energy", "Loops"],
  ["Print Energy", "Print"],
];

const DESCRIPTION = "Estimated energy cost in nanojoules, from the Georgiou et al. (2021) Cortex-M0 model.";

function formatNj(value) {
  return typeof value === "number" ? value.toFixed(6) : "-";
}

export default function EnergyPage() {
  const { lastResult } = useCompile();
  const energy = lastResult?.energy;
  const energyBefore = lastResult?.energyBefore;

  if (!energy) {
    return (
      <section>
        <PageHeading title="Energy report" description={DESCRIPTION} />
        <EmptyState />
      </section>
    );
  }

  // The before report is missing only for results persisted before that
  // feature existed. Hide everything comparative in that case.
  const hasBefore = !!energyBefore && typeof energyBefore["Total Energy Units"] === "number";
  const totalAfter = energy["Total Energy Units"];
  const totalBefore = hasBefore ? energyBefore["Total Energy Units"] : null;
  const savedNj = hasBefore ? totalBefore - totalAfter : null;
  const savedPct = hasBefore && totalBefore > 0 ? (savedNj / totalBefore) * 100 : null;

  // Bars share one scale: the largest value across both reports.
  const barMax = Math.max(
    1,
    ...ENERGY_CATEGORIES.flatMap(([key]) => [Number(energy[key]) || 0, hasBefore ? Number(energyBefore[key]) || 0 : 0])
  );

  return (
    <section>
      <PageHeading title="Energy report" description={DESCRIPTION} />
      <PageBody>
        {/* gap-px over a bg-line background draws 1px dividers between cells. */}
        <Panel className="grid grid-cols-2 gap-px bg-line lg:grid-cols-5">
          <Stat label="After optimization" value={formatNj(totalAfter) + " nJ"} valueClassName="text-stage-emerald" />
          <Stat label="Before optimization" value={hasBefore ? formatNj(totalBefore) + " nJ" : "-"} />
          <Stat label="Saving" value={hasBefore ? formatNj(savedNj) + " nJ" : "-"} />
          <Stat label="Saving (%)" value={savedPct !== null ? savedPct.toFixed(2) + "%" : "-"} />
          <Stat
            className="col-span-2 lg:col-span-1"
            label="Estimated energy" value={energy["Estimated Energy"] ?? "-"} note={"Time complexity " + (energy["Time Complexity"] ?? "-")} />
        </Panel>

        <Panel>
          <PanelHeader label="Energy by category (nJ)">
            <span className="flex items-center gap-3">
              <Legend className="bg-white/25" label="Before" />
              <Legend className="bg-stage-emerald" label="After" />
            </span>
          </PanelHeader>
          <div className="flex flex-col gap-3 p-4">
            {ENERGY_CATEGORIES.map(([key, label]) => (
              <CategoryBars
                key={key}
                label={label}
                before={hasBefore ? Number(energyBefore[key]) || 0 : null}
                after={Number(energy[key]) || 0}
                max={barMax}
              />
            ))}
          </div>
        </Panel>

        <Panel>
          <PanelHeader label="Full report" />
          <div className="overflow-x-auto">
            <table className="w-full border-collapse text-[13px]">
              <thead className="text-left text-[11px] tracking-wide text-faint uppercase">
                <tr className="border-b border-line">
                  <th className="px-3 py-2 font-medium">Field</th>
                  {hasBefore && <th className="px-3 py-2 text-right font-medium">Before</th>}
                  <th className="px-3 py-2 text-right font-medium">After</th>
                </tr>
              </thead>
              <tbody>
                {Object.entries(energy).map(([key, value]) => (
                  <tr key={key} className="border-b border-line last:border-0">
                    <td className="px-3 py-1.5 text-muted">{key}</td>
                    {hasBefore && <td className="px-3 py-1.5 text-right font-mono text-muted">{String(energyBefore[key] ?? "-")}</td>}
                    <td className="px-3 py-1.5 text-right font-mono text-fg">{String(value)}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </Panel>

        <WaterTeaser energy={energy} energyBefore={energyBefore} />
      </PageBody>
    </section>
  );
}

function Legend({ className, label }) {
  return (
    <span className="flex items-center gap-1.5 text-[11px] text-muted">
      <span className={"h-1.5 w-3 rounded-sm " + className} />
      {label}
    </span>
  );
}

// Two thin bars per category: before (neutral) above after (emerald).
function CategoryBars({ label, before, after, max }) {
  return (
    <div className="grid grid-cols-[88px_1fr_96px] items-center gap-x-3 gap-y-1 text-[12px] sm:grid-cols-[120px_1fr_120px]">
      {before !== null && (
        <>
          <span className="row-span-2 text-muted">{label}</span>
          <Bar value={before} max={max} className="bg-white/25" />
          <span className="text-right font-mono text-muted">{before.toFixed(6)}</span>
        </>
      )}
      {before === null && <span className="text-muted">{label}</span>}
      <Bar value={after} max={max} className="bg-stage-emerald" />
      <span className="text-right font-mono text-fg">{after.toFixed(6)}</span>
    </div>
  );
}

function Bar({ value, max, className }) {
  return (
    <div className="h-1.5 overflow-hidden rounded-sm bg-white/5">
      <div className={"h-full rounded-sm " + className} style={{ width: (value / max) * 100 + "%" }} />
    </div>
  );
}

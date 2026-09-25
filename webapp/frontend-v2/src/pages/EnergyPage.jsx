import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";

// Ported from the old index.html's Energy Report tab: headline stats,
// the five per-category operation-count bars (scaled to the largest of
// the five for this specific compile, not a fixed global max), and the
// full key/value table.
const ENERGY_BAR_FIELDS = ["Assignments", "Arithmetic Ops", "Comparisons", "Loop Iterations", "Print Statements"];

export default function EnergyPage() {
  const { lastResult } = useCompile();
  const energy = lastResult?.energy;

  if (!energy) {
    return (
      <section>
        <h1>Energy</h1>
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
      <h1>Energy</h1>

      <div className="headline-stats">
        <div className="stat stat--energy">
          <div className="label">Estimated Energy</div>
          <div className="value">{energy["Estimated Energy"] ?? "-"}</div>
        </div>
        <div className="stat">
          <div className="label">Time Complexity</div>
          <div className="value">{energy["Time Complexity"] ?? "-"}</div>
        </div>
      </div>

      <div className="energy-bars">
        {ENERGY_BAR_FIELDS.map((key, i) => (
          <div className="energy-bar-row" key={key}>
            <span className="energy-bar-label">{key}</span>
            <div className="energy-bar-track">
              <div className="energy-bar-fill" style={{ width: (barValues[i] / barMax) * 100 + "%" }} />
            </div>
            <span className="energy-bar-value">{barValues[i]}</span>
          </div>
        ))}
      </div>

      <table>
        <tbody>
          {tableEntries.map(([key, value]) => (
            <tr key={key}>
              <th>{key}</th>
              <td>{value}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}

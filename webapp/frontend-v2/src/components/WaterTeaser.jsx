import { Link } from "react-router-dom";
import { STAGE_COLORS } from "../stageColors";
import { computeWaterImpact, formatLiters } from "../water";

const ENERGY_STAGE = STAGE_COLORS[5];

// Compact summary card for the bottom of the Energy page - just enough
// to discover the water-impact story exists, without duplicating the
// full citation/illustrative breakdown that now lives at /water. Uses
// the exact same computeWaterImpact() as the full page, so the headline
// number here can never drift from what /water shows for the same
// compile.
export default function WaterTeaser({ energy, energyBefore }) {
  const impact = computeWaterImpact(energy, energyBefore);
  if (!impact) return null;

  const headline = impact.hasComparison
    ? impact.percentSaved.toFixed(2) + "%"
    : formatLiters(impact.litersAfter);
  const headlineLabel = impact.hasComparison
    ? "less site cooling water after optimization"
    : "this run's site cooling water";

  return (
    <Link
      to="/water"
      className={
        "mt-10 flex items-center justify-between rounded-xl border p-6 shadow-soft transition-all duration-200 hover:-translate-y-0.5 hover:shadow-soft-lg dark:shadow-soft-dark dark:hover:shadow-soft-dark-lg " +
        ENERGY_STAGE.iconChip
      }
    >
      <div>
        <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
          Water impact
        </div>
        <div className={"font-display mt-1 text-3xl font-semibold " + ENERGY_STAGE.iconText}>{headline}</div>
        <div className="mt-1 text-xs text-ink-soft dark:text-ink-dark-soft">{headlineLabel}</div>
      </div>
      <span className={"text-sm font-medium whitespace-nowrap " + ENERGY_STAGE.iconText}>
        See full water impact →
      </span>
    </Link>
  );
}

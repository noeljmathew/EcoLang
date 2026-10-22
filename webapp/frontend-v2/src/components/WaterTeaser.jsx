import { Link } from "react-router-dom";
import { ArrowRight, Droplets } from "lucide-react";
import { computeWaterImpact, formatLiters } from "../water";

// Compact link to /water at the bottom of the Energy page. Uses the same
// computeWaterImpact() as the full page, so the figure here always
// matches what /water shows for the same compile.
export default function WaterTeaser({ energy, energyBefore }) {
  const impact = computeWaterImpact(energy, energyBefore);
  if (!impact) return null;

  const headline = impact.hasComparison ? impact.percentSaved.toFixed(2) + "%" : formatLiters(impact.litersAfter);
  const headlineLabel = impact.hasComparison
    ? "less site cooling water after optimization"
    : "this run's site cooling water";

  return (
    <Link
      to="/water"
      className="group flex items-center gap-3 rounded-md border border-line bg-panel px-4 py-3 transition-colors duration-150 hover:bg-hover"
    >
      <Droplets size={16} strokeWidth={1.75} className="text-stage-emerald" />
      <span className="font-mono text-[14px] text-fg">{headline}</span>
      <span className="text-[13px] text-muted">{headlineLabel}</span>
      <span className="ml-auto flex items-center gap-1 text-[12px] text-muted group-hover:text-fg">
        Water impact
        <ArrowRight size={14} strokeWidth={1.75} />
      </span>
    </Link>
  );
}

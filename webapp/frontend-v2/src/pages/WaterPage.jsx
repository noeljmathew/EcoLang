import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import { PageBody, Panel, PanelHeader, Stat } from "../components/Panel";
import { Badge } from "../components/ui/badge";
import { ILLUSTRATIVE_RUN_COUNT, SITE_WUE_LITERS_PER_KWH, WUE_CITATION, computeWaterImpact, formatLiters } from "../water";

// Every figure comes from computeWaterImpact() in water.js. Water is
// derived from Energy's numbers, not a pipeline stage, so the header
// shows a "Beyond the pipeline" badge instead of a stage badge. The
// real figures and the illustrative at-scale figures are kept visually
// separate (the illustrative block has a dashed border and a label).
const DESCRIPTION =
  "What this program's hardware-weighted energy cost means for site cooling water: one real number, one real percentage, and one clearly labeled illustration at scale.";

const PERCENT_EXPLANATION =
  "A real percentage: both sides are this program's actual hardware-weighted energy total (Georgiou et al. Cortex-M0 model), before and after the optimizer. It is not an instruction-count proxy. Water scales directly with energy, so this is also the site cooling water reduction.";

export default function WaterPage() {
  const { lastResult } = useCompile();
  const impact = computeWaterImpact(lastResult?.energy, lastResult?.energyBefore);
  const badge = <Badge>Beyond the pipeline</Badge>;

  return (
    <section>
      <PageHeading title="Water impact" description={DESCRIPTION} badge={badge} />
      {!impact ? (
        <EmptyState />
      ) : (
        <PageBody>
          <Panel className="grid grid-cols-1 gap-px bg-line sm:grid-cols-2">
            <Stat
              label="Energy reduction from optimization"
              value={impact.hasComparison ? impact.percentSaved.toFixed(2) + "%" : "-"}
              valueClassName="text-[24px] text-stage-emerald"
              note={
                impact.hasComparison
                  ? PERCENT_EXPLANATION
                  : "Recompile to see this. Older persisted results do not include a before-optimization energy figure to compare against."
              }
            />
            <Stat
              label="This run's site cooling water"
              value={formatLiters(impact.litersAfter)}
              valueClassName="text-[24px]"
              note={
                "Vanishingly small, as expected for a single compile: real energy (" +
                impact.joulesAfter.toExponential(3) +
                " J) converted through the cited WUE below, nothing more."
              }
            />
          </Panel>

          <Panel>
            <PanelHeader label="Source" />
            <div className="flex flex-col gap-2 p-4 text-[13px] leading-5 text-muted">
              <p>{WUE_CITATION}</p>
              <p>
                This figure is <span className="font-medium text-fg">site cooling water only</span>: the water evaporated
                on site to cool the computer. It excludes the water used upstream to generate the electricity itself,
                which is separately larger, so this is not a total water footprint.
              </p>
            </div>
          </Panel>

          {impact.hasComparison && (
            <div className="rounded-md border border-dashed border-line-strong">
              <div className="flex flex-wrap items-center gap-2 border-b border-dashed border-line-strong px-4 py-2.5">
                <Badge>Illustrative</Badge>
                <span className="text-[12px] text-muted">Not a real system. A demonstration multiplier only.</span>
              </div>
              <div className="p-4">
                <p className="text-[13px] text-fg">
                  If this exact program ran <span className="font-mono">{ILLUSTRATIVE_RUN_COUNT.toLocaleString()}</span>{" "}
                  times:
                </p>
                <div className="mt-3 grid grid-cols-1 gap-4 sm:grid-cols-3">
                  <ScaleFigure label="Before optimization" value={formatLiters(impact.litersBeforeAtScale)} />
                  <ScaleFigure label="After optimization" value={formatLiters(impact.litersAfterAtScale)} />
                  <ScaleFigure
                    label="Difference"
                    value={formatLiters(impact.litersSavedAtScale)}
                    className="text-stage-emerald"
                  />
                </div>
                <p className="mt-4 text-[12px] leading-5 text-muted">
                  One billion runs is chosen only to make an otherwise invisible per-run quantity visible. It is not a
                  claim about any real deployment. The principle it illustrates is real and cited (less computation
                  means less site cooling water, at WUE = {SITE_WUE_LITERS_PER_KWH} L/kWh); the run count is not.
                </p>
              </div>
            </div>
          )}
        </PageBody>
      )}
    </section>
  );
}

function ScaleFigure({ label, value, className = "text-fg" }) {
  return (
    <div>
      <div className="text-[11px] font-medium tracking-wide text-faint uppercase">{label}</div>
      <div className={"mt-1 font-mono text-[18px] " + className}>{value}</div>
    </div>
  );
}

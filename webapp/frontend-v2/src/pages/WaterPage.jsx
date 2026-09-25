import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import { STAGE_COLORS } from "../stageColors";
import {
  ILLUSTRATIVE_RUN_COUNT,
  SITE_WUE_LITERS_PER_KWH,
  WUE_CITATION,
  computeWaterImpact,
  formatLiters,
} from "../water";

// Energy is stage 5 - emerald (see stageColors.js). Water is derived
// from Energy's own numbers, not a compiler stage of its own (see
// NavBar.jsx's "beyond the pipeline" grouping), so it borrows Energy's
// color rather than getting a new one from the 7-stage spectrum.
const ENERGY_STAGE = STAGE_COLORS[5];

// The dedicated /water page - the full WaterImpactPanel's content,
// elevated: the headline stat leads at real display size instead of
// being one card among several at the bottom of Energy. Every string
// below is unchanged from the original panel - citation, the "site
// cooling water only" exclusion framing, the real-vs-illustrative
// distinction, the honesty tone. Only the visual weight and ordering
// changed. A teaching illustration of a real, cited principle (less
// computation -> less site cooling water, at scale) - not a claim that
// any single compile of this program measurably moved a water meter.
export default function WaterPage() {
  const { lastResult } = useCompile();
  const impact = computeWaterImpact(lastResult?.energy, lastResult?.energyBefore);

  return (
    <section>
      <div className="mb-6">
        <span
          className={
            "mb-2 inline-flex items-center gap-1.5 rounded-full border px-3 py-1 text-xs font-medium tracking-wide uppercase " +
            ENERGY_STAGE.badge
          }
        >
          Derived from energy · not a pipeline stage
        </span>
        <h1 className="font-display text-4xl font-semibold tracking-tight text-ink dark:text-ink-dark">
          Water Impact
        </h1>
        <p className="mt-2 max-w-2xl text-base text-ink-soft dark:text-ink-dark-soft">
          What this program's real, hardware-weighted energy cost means for site cooling water -
          one real number, one real percentage, and one clearly-labeled illustration at scale.
        </p>
      </div>

      {!impact ? (
        <EmptyState />
      ) : (
        <>
          {/* Headline: leads the page, at real display size - this is
              the thing the dedicated page exists to show prominently. */}
          {impact.hasComparison ? (
            <div className="mb-6">
              <div className="text-sm font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
                Energy reduction from optimization
              </div>
              <div
                className={
                  "font-display mt-1 text-5xl leading-none font-semibold sm:text-6xl " + ENERGY_STAGE.iconText
                }
              >
                {impact.percentSaved.toFixed(2)}%
              </div>
              <p className="mt-3 max-w-xl text-ink-soft dark:text-ink-dark-soft">
                A real percentage: both sides are this program's actual hardware-weighted energy
                total (Georgiou et al. Cortex-M0 model), before vs. after the optimizer - not an
                instruction-count proxy. Water scales directly with energy, so this is also the
                site-cooling-water reduction.
              </p>
            </div>
          ) : (
            <div className="mb-6">
              <div className="text-sm font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
                This run's site cooling water
              </div>
              <div
                className={
                  "font-display mt-1 text-5xl leading-none font-semibold sm:text-6xl " + ENERGY_STAGE.iconText
                }
              >
                {formatLiters(impact.litersAfter)}
              </div>
              <p className="mt-3 max-w-xl text-ink-soft dark:text-ink-dark-soft">
                Vanishingly small, as expected for a single compile. Recompile to also see the
                before/after percentage - older persisted results don't include a
                before-optimization energy figure to compare against.
              </p>
            </div>
          )}

          {/* Citation, stated plainly - not buried in a code comment or
              a tiny footnote. */}
          <div className="mb-6 max-w-2xl border-t border-accent-soft-border pt-4 dark:border-accent-dark-soft">
            <p className="text-xs text-ink-soft dark:text-ink-dark-soft">{WUE_CITATION}</p>
            <p className="mt-1 text-xs text-ink-soft dark:text-ink-dark-soft">
              This figure is <span className="font-medium">site cooling water only</span> - the
              water evaporated on-site to cool the computer. It excludes the water used upstream
              to generate the electricity itself, which is separately larger - this is not a
              total water footprint.
            </p>
          </div>

          {/* Real figures, from this compile's own numbers. */}
          <div className="mb-6 grid grid-cols-1 gap-4 sm:grid-cols-2">
            <div className={"rounded-xl border p-5 shadow-soft dark:shadow-soft-dark " + ENERGY_STAGE.iconChip}>
              <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
                This run's site cooling water
              </div>
              <div className={"font-display mt-1 text-2xl font-semibold " + ENERGY_STAGE.iconText}>
                {formatLiters(impact.litersAfter)}
              </div>
              <div className="mt-1 text-xs text-ink-soft dark:text-ink-dark-soft">
                Vanishingly small, as expected for a single compile - real energy (
                {impact.joulesAfter.toExponential(3)} J) converted through the cited WUE above,
                nothing more.
              </div>
            </div>

            <div className={"rounded-xl border p-5 shadow-soft dark:shadow-soft-dark " + ENERGY_STAGE.iconChip}>
              <div className="text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
                Energy reduction from optimization
              </div>
              {impact.hasComparison ? (
                <>
                  <div className={"font-display mt-1 text-2xl font-semibold " + ENERGY_STAGE.iconText}>
                    {impact.percentSaved.toFixed(2)}%
                  </div>
                  <div className="mt-1 text-xs text-ink-soft dark:text-ink-dark-soft">
                    A real percentage: both sides are this program's actual hardware-weighted
                    energy total (Georgiou et al. Cortex-M0 model), before vs. after the
                    optimizer - not an instruction-count proxy. Water scales directly with
                    energy, so this is also the site-cooling-water reduction.
                  </div>
                </>
              ) : (
                <div className="mt-1 text-sm text-ink-soft dark:text-ink-dark-soft">
                  Recompile to see this - older persisted results don't include a
                  before-optimization energy figure to compare against.
                </div>
              )}
            </div>
          </div>

          {/* Illustrative at-scale comparison - visually distinct
              (dashed border, explicit label) from the real figures
              above so the two are never mistaken for each other. */}
          {impact.hasComparison && (
            <div className="rounded-xl border-2 border-dashed border-ink-soft/30 p-5 dark:border-ink-dark-soft/30">
              <div className="flex items-center gap-2">
                <span className="rounded-full border border-ink-soft/30 px-2 py-0.5 text-[10px] font-semibold tracking-wide text-ink-soft uppercase dark:border-ink-dark-soft/30 dark:text-ink-dark-soft">
                  Illustrative
                </span>
                <span className="text-xs text-ink-soft dark:text-ink-dark-soft">
                  not a real system - a demonstration multiplier only
                </span>
              </div>
              <p className="mt-3 text-base text-ink dark:text-ink-dark">
                If this exact program ran{" "}
                <span className="font-mono">{ILLUSTRATIVE_RUN_COUNT.toLocaleString()}</span> times:
              </p>
              <div className="mt-4 grid grid-cols-1 gap-4 sm:grid-cols-3">
                <div>
                  <div className="text-xs text-ink-soft dark:text-ink-dark-soft">Before optimization</div>
                  <div className="font-display mt-1 text-3xl text-ink dark:text-ink-dark">
                    {formatLiters(impact.litersBeforeAtScale)}
                  </div>
                </div>
                <div>
                  <div className="text-xs text-ink-soft dark:text-ink-dark-soft">After optimization</div>
                  <div className="font-display mt-1 text-3xl text-ink dark:text-ink-dark">
                    {formatLiters(impact.litersAfterAtScale)}
                  </div>
                </div>
                <div>
                  <div className="text-xs text-ink-soft dark:text-ink-dark-soft">Difference</div>
                  <div className={"font-display mt-1 text-3xl font-medium " + ENERGY_STAGE.iconText}>
                    {formatLiters(impact.litersSavedAtScale)}
                  </div>
                </div>
              </div>
              <p className="mt-4 text-xs text-ink-soft dark:text-ink-dark-soft">
                One billion runs is chosen only to make an otherwise-invisible per-run quantity
                visible - it is not a claim about any real deployment. The principle it
                illustrates is real and cited (less computation → less site cooling water, at
                WUE = {SITE_WUE_LITERS_PER_KWH} L/kWh); the run count is not.
              </p>
            </div>
          )}
        </>
      )}
    </section>
  );
}

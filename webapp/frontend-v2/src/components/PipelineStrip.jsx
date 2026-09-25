import { useCompile } from "../context/CompileContext";
import { PIPELINE_STAGES, determineFailedStageIndex, stageState } from "../pipeline";
import { STAGE_COLORS } from "../stageColors";

// Persistent pipeline status strip, ported from the old index.html.
// failedIndex mirrors exactly what the old page's compile() passed to
// renderPipeline() at each point: -1 while idle/loading or on a
// network failure (no stage-level info yet), null on success (all
// done), or determineFailedStageIndex(errorMessage) on a compile
// failure.
//
// Color rule, deliberately: a "done" stage's dot uses ITS OWN spectrum
// color (STAGE_COLORS[i].dot) - a full successful compile lights up all
// seven hues at once, the whole point of having a spectrum at all. A
// "failed" stage never touches STAGE_COLORS - it's always the same
// fixed red, regardless of which stage failed or what that stage's own
// color is, so failure reads as unambiguously "wrong" rather than
// competing with the stage's identity color. "unreached" stays a fixed
// neutral grey for the same reason - only "done" gets stage color.
export default function PipelineStrip() {
  const { status, errorMessage } = useCompile();

  let failedIndex;
  if (status === "success") {
    failedIndex = null;
  } else if (status === "error") {
    failedIndex = determineFailedStageIndex(errorMessage);
  } else {
    // idle or loading - same as the old page's initial markup (every
    // stage starts "unreached") and its renderPipeline(-1) call at the
    // start of every compile attempt.
    failedIndex = -1;
  }

  return (
    <div className="mb-8 flex flex-wrap items-center gap-x-1 gap-y-2 rounded-xl border border-accent-soft-border bg-paper-raised px-4 py-3 text-xs shadow-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:shadow-soft-dark">
      {PIPELINE_STAGES.map((label, i) => {
        const state = stageState(i, failedIndex);
        return (
          <span key={label} className="flex items-center gap-1">
            {i > 0 && <span className="mx-1 text-ink-soft/50 dark:text-ink-dark-soft/50">→</span>}
            <span
              className={
                "flex items-center gap-1.5 transition-opacity duration-300 " +
                (state === "unreached" ? "opacity-40" : "opacity-100")
              }
            >
              <span
                className={
                  "h-2 w-2 rounded-full border transition-colors duration-300 " +
                  (state === "done"
                    ? STAGE_COLORS[i].dot
                    : state === "failed"
                      ? "border-red-600 bg-red-600 dark:border-red-500 dark:bg-red-500"
                      : "border-ink-soft/50 bg-transparent dark:border-ink-dark-soft/50")
                }
              />
              <span
                className={
                  "tracking-wide uppercase " +
                  (state === "failed"
                    ? "font-semibold text-red-600 dark:text-red-400"
                    : state === "done"
                      ? "font-medium text-ink dark:text-ink-dark"
                      : "text-ink-soft dark:text-ink-dark-soft")
                }
              >
                {label}
              </span>
            </span>
          </span>
        );
      })}
    </div>
  );
}

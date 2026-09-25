import { useCompile } from "../context/CompileContext";
import { PIPELINE_STAGES, determineFailedStageIndex, stageState } from "../pipeline";

// Persistent pipeline status strip, ported from the old index.html.
// failedIndex mirrors exactly what the old page's compile() passed to
// renderPipeline() at each point: -1 while idle/loading or on a
// network failure (no stage-level info yet), null on success (all
// done), or determineFailedStageIndex(errorMessage) on a compile
// failure. Unstyled beyond a plain dot + label per stage - real visual
// design is a later pass.
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
    <div className="pipeline">
      {PIPELINE_STAGES.map((label, i) => (
        <span key={label} className="pipeline-stage-wrap">
          {i > 0 && <span className="pipeline-arrow">→</span>}
          <span className={"pipeline-stage pipeline-stage--" + stageState(i, failedIndex)}>
            <span className="pipeline-dot" />
            <span className="pipeline-label">{label}</span>
          </span>
        </span>
      ))}
    </div>
  );
}

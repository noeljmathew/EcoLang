import { useCompile } from "../../context/CompileContext";
import { PIPELINE_STAGES, determineFailedStageIndex, stageState } from "../../pipeline";
import { STAGE_COLORS } from "../../stageColors";
import { cn } from "../../lib/utils";

// Same rule the old pipeline strip used: failedIndex is null after a
// success (every stage done), -1 while idle/loading or after a network
// failure (no stage information), otherwise the index of the failed stage.
function currentFailedIndex(status, errorMessage) {
  if (status === "success") return null;
  if (status === "error") return determineFailedStageIndex(errorMessage);
  return -1;
}

// Backend reachability comes only from the last compile attempt: no
// attempt yet means "not checked", a network error means "unreachable",
// and any response at all (success or compile error) means "connected".
function backendState(status, errorMessage) {
  if (status === "idle") return { label: "not checked", dot: "bg-faint" };
  if (status === "loading") return { label: "checking", dot: "bg-faint" };
  if (status === "error" && String(errorMessage).startsWith("Could not reach the backend")) {
    return { label: "unreachable", dot: "bg-danger" };
  }
  return { label: "connected", dot: "bg-stage-emerald" };
}

export default function StatusBar() {
  const { status, errorMessage } = useCompile();
  const failedIndex = currentFailedIndex(status, errorMessage);
  const backend = backendState(status, errorMessage);

  return (
    <footer className="flex h-6 shrink-0 items-center gap-3 border-t border-line bg-panel px-3 font-mono text-[11px] text-faint">
      <ol aria-label="Pipeline status" className="flex min-w-0 items-center gap-2.5">
        {PIPELINE_STAGES.map((label, i) => {
          const state = stageState(i, failedIndex);
          return (
            <li key={label} className="flex items-center gap-1.5" title={label + ": " + state}>
              <span
                className={cn(
                  "h-1.5 w-1.5 rounded-full",
                  state === "done" && STAGE_COLORS[i].dot,
                  state === "failed" && "bg-danger",
                  state === "unreached" && "border border-faint"
                )}
              />
              <span
                className={cn(
                  "hidden tracking-wide lg:inline",
                  state === "done" && "text-muted",
                  state === "failed" && "font-medium text-danger"
                )}
              >
                {label}
              </span>
            </li>
          );
        })}
        {failedIndex !== null && failedIndex >= 0 && (
          <li className="font-medium text-danger lg:hidden">{PIPELINE_STAGES[failedIndex]} failed</li>
        )}
      </ol>
      <div className="ml-auto flex shrink-0 items-center gap-1.5">
        <span className={cn("h-1.5 w-1.5 rounded-full", backend.dot)} />
        <span>Backend: {backend.label}</span>
      </div>
    </footer>
  );
}

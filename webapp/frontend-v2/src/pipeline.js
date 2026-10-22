// Pipeline-strip logic, originally written for the first single-page
// frontend (since removed): the fixed stage order, and failed-stage
// detection from the compiler's error-message prefixes, falling back
// to -1 for anything else.

export const PIPELINE_STAGES = ["SOURCE", "LEXER", "AST", "IR", "OPTIMIZER", "ENERGY", "CODEGEN"];

// Returns the index (into PIPELINE_STAGES) of the stage that failed, or
// -1 if there's no stage-level information (e.g. a network failure -
// the message won't start with any of the known compiler error prefixes,
// so it falls through to -1).
export function determineFailedStageIndex(errorMessage) {
  const message = String(errorMessage || "");
  if (message.startsWith("Lexical error")) return 1; // LEXER
  if (message.startsWith("Syntax error")) return 2; // AST
  // No separate "semantic analysis" stage exists in this pipeline - it
  // runs after parsing (AST) and before IR, so a semantic error is
  // reported against AST rather than inventing a stage that isn't there.
  if (message.startsWith("Semantic error")) return 2; // AST
  return -1; // no stage-level information (e.g. a network failure)
}

// Ported from renderPipeline's per-stage state logic (the body of its
// pipelineStageEls.forEach), extracted into a pure function so it can
// be used as JSX className logic instead of imperative classList calls.
// `failedIndex` is null (everything succeeded), -1 (no stage-level
// info - e.g. nothing compiled yet, still loading, or a network
// failure), or the index of the stage that failed.
export function stageState(i, failedIndex) {
  if (failedIndex === null) return "done";
  if (failedIndex === -1) return "unreached";
  if (i < failedIndex) return "done";
  if (i === failedIndex) return "failed";
  return "unreached";
}

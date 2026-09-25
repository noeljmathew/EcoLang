// Per-page pipeline-stage metadata - purely presentational (which route
// maps to which stage badge). Reuses PIPELINE_STAGES' exact labels and
// order from pipeline.js (the pipeline strip's real data) so the two
// stay consistent; this file adds no new data-flow of its own, just a
// route -> stage-index lookup for the small badge near each heading.
import { PIPELINE_STAGES } from "./pipeline";
import { STAGE_COLORS } from "./stageColors";

export const PAGE_STAGE_INDEX = {
  "/compile": 0,
  "/tokens": 1,
  "/ast": 2,
  "/ir": 3,
  "/optimizer": 4,
  "/energy": 5,
  "/codegen": 6,
};

export function stageBadge(pathname) {
  const index = PAGE_STAGE_INDEX[pathname];
  if (index === undefined) return null;
  return { index, label: PIPELINE_STAGES[index], color: STAGE_COLORS[index] };
}

// Pages that aren't one of the 7 numbered pipeline stages (so
// stageBadge above deliberately does not map them - no "Stage N" badge)
// but still want a themed background wash (PageWash.jsx) tied to a
// stage's color. /water is derived from Energy's own numbers, so it
// borrows Energy's color without claiming to BE the Energy stage.
const EXTRA_PAGE_COLORS = {
  "/water": STAGE_COLORS[5],
};

export function pageAccentColor(pathname) {
  const badge = stageBadge(pathname);
  if (badge) return badge.color;
  return EXTRA_PAGE_COLORS[pathname] ?? null;
}

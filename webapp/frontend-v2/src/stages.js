// Navigation and per-page stage metadata, shared by the activity rail,
// the mobile menu and each page header. Stage labels come from
// PIPELINE_STAGES (pipeline.js) so every place uses the same names.
import { Braces, Code, Droplets, FileCode, Gauge, House, ListTree, Network, Zap } from "lucide-react";
import { PIPELINE_STAGES } from "./pipeline";
import { STAGE_COLORS } from "./stageColors";

export const HOME_ITEM = { to: "/", label: "Home", icon: House };

// The 7 pipeline stages, in order. `stage` indexes PIPELINE_STAGES.
export const STAGE_ITEMS = [
  { to: "/compile", label: "Compile", icon: FileCode, stage: 0 },
  { to: "/tokens", label: "Tokens", icon: Braces, stage: 1 },
  { to: "/ast", label: "AST", icon: Network, stage: 2 },
  { to: "/ir", label: "IR", icon: ListTree, stage: 3 },
  { to: "/optimizer", label: "Optimizer", icon: Gauge, stage: 4 },
  { to: "/energy", label: "Energy", icon: Zap, stage: 5 },
  { to: "/codegen", label: "Code Generation", icon: Code, stage: 6 },
];

// Not a pipeline stage: Water is derived from Energy's numbers, so it
// sits after a "Beyond the pipeline" separator.
export const WATER_ITEM = { to: "/water", label: "Water Impact", icon: Droplets };

export function stageBadge(pathname) {
  const item = STAGE_ITEMS.find((entry) => entry.to === pathname);
  if (!item) return null;
  return { index: item.stage, label: PIPELINE_STAGES[item.stage], color: STAGE_COLORS[item.stage] };
}

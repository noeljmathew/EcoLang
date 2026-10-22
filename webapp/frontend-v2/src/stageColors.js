// The 7-stage colors, indexed to match PIPELINE_STAGES (pipeline.js):
// STAGE_COLORS[i] is always the color for PIPELINE_STAGES[i]. Base hex
// values live in index.css (--color-stage-*). These are used only for
// small stage markers: dots, badges and the status bar.
//
// Class names are written out in full rather than built with string
// concatenation, because Tailwind finds classes by scanning the source
// text and would not see a name assembled at runtime.
export const STAGE_COLORS = [
  // 0 SOURCE
  { dot: "bg-stage-amber", text: "text-stage-amber", badge: "border-stage-amber/30 text-stage-amber" },
  // 1 LEXER
  { dot: "bg-stage-orange", text: "text-stage-orange", badge: "border-stage-orange/30 text-stage-orange" },
  // 2 AST
  { dot: "bg-stage-rose", text: "text-stage-rose", badge: "border-stage-rose/30 text-stage-rose" },
  // 3 IR
  { dot: "bg-stage-violet", text: "text-stage-violet", badge: "border-stage-violet/30 text-stage-violet" },
  // 4 OPTIMIZER
  { dot: "bg-stage-blue", text: "text-stage-blue", badge: "border-stage-blue/30 text-stage-blue" },
  // 5 ENERGY
  { dot: "bg-stage-emerald", text: "text-stage-emerald", badge: "border-stage-emerald/30 text-stage-emerald" },
  // 6 CODEGEN
  { dot: "bg-stage-teal", text: "text-stage-teal", badge: "border-stage-teal/30 text-stage-teal" },
];

// The 7-stage pipeline spectrum's Tailwind class bundles, indexed to
// match PIPELINE_STAGES (pipeline.js) exactly - STAGE_COLORS[i] is
// always the color for PIPELINE_STAGES[i]. Base hex values live in
// index.css's @theme (--color-stage-*); this file only maps each stage
// to the *full, literal* class strings that use them.
//
// Written out by hand rather than built via string concatenation
// (e.g. `"bg-stage-" + key`) on purpose: Tailwind's build-time scanner
// finds candidate classes by looking for complete class-name substrings
// in the source text, not by evaluating JS - a dynamically-assembled
// class name is invisible to it and would silently produce no CSS.
export const STAGE_COLORS = [
  {
    // 0 SOURCE
    key: "amber",
    dot: "border-stage-amber bg-stage-amber dark:border-stage-amber-dark dark:bg-stage-amber-dark",
    badge:
      "border-stage-amber/30 bg-stage-amber/10 text-stage-amber dark:border-stage-amber-dark/30 dark:bg-stage-amber-dark/15 dark:text-stage-amber-dark",
    iconText: "text-stage-amber dark:text-stage-amber-dark",
    iconChip:
      "border-stage-amber/25 bg-stage-amber/12 dark:border-stage-amber-dark/30 dark:bg-stage-amber-dark/18",
    washFrom: "from-stage-amber/14 dark:from-stage-amber-dark/10",
  },
  {
    // 1 LEXER
    key: "orange",
    dot: "border-stage-orange bg-stage-orange dark:border-stage-orange-dark dark:bg-stage-orange-dark",
    badge:
      "border-stage-orange/30 bg-stage-orange/10 text-stage-orange dark:border-stage-orange-dark/30 dark:bg-stage-orange-dark/15 dark:text-stage-orange-dark",
    iconText: "text-stage-orange dark:text-stage-orange-dark",
    iconChip:
      "border-stage-orange/25 bg-stage-orange/12 dark:border-stage-orange-dark/30 dark:bg-stage-orange-dark/18",
    washFrom: "from-stage-orange/14 dark:from-stage-orange-dark/10",
  },
  {
    // 2 AST
    key: "rose",
    dot: "border-stage-rose bg-stage-rose dark:border-stage-rose-dark dark:bg-stage-rose-dark",
    badge:
      "border-stage-rose/30 bg-stage-rose/10 text-stage-rose dark:border-stage-rose-dark/30 dark:bg-stage-rose-dark/15 dark:text-stage-rose-dark",
    iconText: "text-stage-rose dark:text-stage-rose-dark",
    iconChip:
      "border-stage-rose/25 bg-stage-rose/12 dark:border-stage-rose-dark/30 dark:bg-stage-rose-dark/18",
    washFrom: "from-stage-rose/14 dark:from-stage-rose-dark/10",
  },
  {
    // 3 IR
    key: "violet",
    dot: "border-stage-violet bg-stage-violet dark:border-stage-violet-dark dark:bg-stage-violet-dark",
    badge:
      "border-stage-violet/30 bg-stage-violet/10 text-stage-violet dark:border-stage-violet-dark/30 dark:bg-stage-violet-dark/15 dark:text-stage-violet-dark",
    iconText: "text-stage-violet dark:text-stage-violet-dark",
    iconChip:
      "border-stage-violet/25 bg-stage-violet/12 dark:border-stage-violet-dark/30 dark:bg-stage-violet-dark/18",
    washFrom: "from-stage-violet/14 dark:from-stage-violet-dark/10",
  },
  {
    // 4 OPTIMIZER
    key: "blue",
    dot: "border-stage-blue bg-stage-blue dark:border-stage-blue-dark dark:bg-stage-blue-dark",
    badge:
      "border-stage-blue/30 bg-stage-blue/10 text-stage-blue dark:border-stage-blue-dark/30 dark:bg-stage-blue-dark/15 dark:text-stage-blue-dark",
    iconText: "text-stage-blue dark:text-stage-blue-dark",
    iconChip:
      "border-stage-blue/25 bg-stage-blue/12 dark:border-stage-blue-dark/30 dark:bg-stage-blue-dark/18",
    washFrom: "from-stage-blue/14 dark:from-stage-blue-dark/10",
  },
  {
    // 5 ENERGY - kept green/emerald, consistent with every earlier pass
    key: "emerald",
    dot: "border-stage-emerald bg-stage-emerald dark:border-stage-emerald-dark dark:bg-stage-emerald-dark",
    badge:
      "border-stage-emerald/30 bg-stage-emerald/10 text-stage-emerald dark:border-stage-emerald-dark/30 dark:bg-stage-emerald-dark/15 dark:text-stage-emerald-dark",
    iconText: "text-stage-emerald dark:text-stage-emerald-dark",
    iconChip:
      "border-stage-emerald/25 bg-stage-emerald/12 dark:border-stage-emerald-dark/30 dark:bg-stage-emerald-dark/18",
    washFrom: "from-stage-emerald/14 dark:from-stage-emerald-dark/10",
  },
  {
    // 6 CODEGEN
    key: "teal",
    dot: "border-stage-teal bg-stage-teal dark:border-stage-teal-dark dark:bg-stage-teal-dark",
    badge:
      "border-stage-teal/30 bg-stage-teal/10 text-stage-teal dark:border-stage-teal-dark/30 dark:bg-stage-teal-dark/15 dark:text-stage-teal-dark",
    iconText: "text-stage-teal dark:text-stage-teal-dark",
    iconChip:
      "border-stage-teal/25 bg-stage-teal/12 dark:border-stage-teal-dark/30 dark:bg-stage-teal-dark/18",
    washFrom: "from-stage-teal/14 dark:from-stage-teal-dark/10",
  },
];

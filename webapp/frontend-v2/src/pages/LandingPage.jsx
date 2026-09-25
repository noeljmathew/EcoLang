import { Link } from "react-router-dom";
import { motion } from "framer-motion";
import { PIPELINE_STAGES } from "../pipeline";
import { STAGE_COLORS } from "../stageColors";
import StageIcon from "../components/StageIcon";

// The full 7-stage spectrum, swept left-to-right as one continuous
// line - independent of how the stage cards below happen to wrap at a
// given viewport width, so the "coherent family" reads clearly no
// matter the layout. Uses each stage's light-mode hex directly (not a
// CSS variable) since Tailwind's gradient utilities cap out at 3 color
// stops and this needs 7; a thin decorative accent like this doesn't
// need a separate dark-mode pass the way body text/backgrounds do.
const SPECTRUM_GRADIENT =
  "linear-gradient(to right, #b45309, #c2410c, #be123c, #6d28d9, #1d4ed8, #047857, #0f766e)";

// Content below is pulled from README.md's opening description, "Project
// purpose" section, and "Compiler pipeline" diagram - not invented copy.
// See README.md at the repo root for the source.
const STAGE_DESCRIPTIONS = {
  SOURCE: "Your .c file",
  LEXER: "Source → tokens",
  AST: "Tokens → syntax tree",
  IR: "Tree → three-address code",
  OPTIMIZER: "Constant folding, dead-temporary elimination",
  ENERGY: "Estimated cost, from real hardware data",
  CODEGEN: "Optimized IR → pseudo-assembly",
};

export default function LandingPage() {
  return (
    <section className="relative">
      {/* Bespoke hero wash: unlike every other page (one stage, one
          color, via PageWash), the landing page represents the whole
          pipeline, so it gets a soft multi-hue glow sweeping the same
          7-color spectrum instead of a single stage tint. */}
      <div
        aria-hidden="true"
        className="pointer-events-none absolute -top-16 -right-24 -z-10 h-96 w-96 rounded-full opacity-[0.16] blur-3xl dark:opacity-[0.14]"
        style={{ background: SPECTRUM_GRADIENT }}
      />

      <motion.div
        initial={{ opacity: 0, y: 10 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.4, ease: "easeOut" }}
      >
        <h1 className="font-display text-4xl leading-[1.1] font-semibold tracking-tight text-ink sm:text-5xl dark:text-ink-dark">
          A compiler that knows what your code costs.
        </h1>
        <p className="mt-4 max-w-xl text-lg text-ink-soft dark:text-ink-dark-soft">
          EcoLang is a small, hand-written, energy-aware compiler for a restricted
          subset of C. It compiles source text all the way through lexing, parsing,
          semantic analysis, IR generation, and optimization — then estimates how
          much energy the compiled program costs to run, using real, published
          Cortex-M0 hardware measurements.
        </p>
        <Link
          to="/compile"
          className="mt-6 inline-flex items-center gap-2 rounded-md bg-accent px-6 py-3 text-sm font-medium text-white shadow-soft transition-all duration-200 hover:-translate-y-0.5 hover:bg-accent-hover hover:shadow-soft-lg dark:bg-accent-dark dark:text-paper-dark dark:shadow-soft-dark dark:hover:shadow-soft-dark-lg dark:hover:brightness-110"
        >
          Try the compiler
          <span aria-hidden="true">→</span>
        </Link>
      </motion.div>

      <motion.div
        initial={{ opacity: 0, y: 10 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.4, delay: 0.15, ease: "easeOut" }}
        className="mt-12"
      >
        <h2 className="font-display text-sm font-semibold tracking-widest text-ink-soft uppercase dark:text-ink-dark-soft">
          The pipeline
        </h2>
        <p className="mt-2 max-w-2xl text-ink-soft dark:text-ink-dark-soft">
          Every EcoLang program passes through seven stages, in order. Any error at
          any stage is caught once and reported clearly — no later stage ever runs
          after an earlier one has failed.
        </p>

        {/* The spectrum swept as one continuous line behind the cards -
            reads as a coherent progression regardless of how the cards
            themselves wrap at a given width. */}
        <div className="relative mt-6">
          <div
            aria-hidden="true"
            className="absolute top-7 right-0 left-0 hidden h-px opacity-40 sm:block dark:opacity-50"
            style={{ background: SPECTRUM_GRADIENT }}
          />
          <div className="relative flex flex-wrap gap-3 lg:flex-nowrap">
            {PIPELINE_STAGES.map((stage, i) => {
              const color = STAGE_COLORS[i];
              return (
                <motion.div
                  key={stage}
                  initial={{ opacity: 0, y: 8 }}
                  animate={{ opacity: 1, y: 0 }}
                  transition={{ duration: 0.3, delay: 0.25 + i * 0.06, ease: "easeOut" }}
                  className="group w-36 rounded-xl border border-accent-soft-border bg-paper-raised p-3 lg:w-auto lg:min-w-0 lg:flex-1 shadow-soft transition-all duration-200 hover:-translate-y-1 hover:shadow-soft-lg dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:shadow-soft-dark dark:hover:shadow-soft-dark-lg"
                >
                  <div className={"flex h-8 w-8 items-center justify-center rounded-lg border " + color.iconChip}>
                    <StageIcon index={i} className={"h-4 w-4 " + color.iconText} />
                  </div>
                  <div className={"mt-2 text-[11px] font-medium " + color.iconText}>
                    {String(i).padStart(2, "0")}
                  </div>
                  <div className="font-display mt-0.5 text-base font-semibold text-ink dark:text-ink-dark">
                    {stage}
                  </div>
                  <div className="mt-1 text-xs text-ink-soft dark:text-ink-dark-soft">
                    {STAGE_DESCRIPTIONS[stage]}
                  </div>
                </motion.div>
              );
            })}
          </div>
        </div>
      </motion.div>

      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        transition={{ duration: 0.4, delay: 0.6 }}
        className="mt-10 max-w-2xl border-t border-accent-soft-border pt-6 text-sm text-ink-soft dark:border-accent-dark-soft dark:text-ink-dark-soft"
      >
        <p>
          The language supports <code className="font-mono text-ink dark:text-ink-dark">int</code>/
          <code className="font-mono text-ink dark:text-ink-dark">void</code> functions, parameters,
          declarations, arithmetic, <code className="font-mono text-ink dark:text-ink-dark">printf</code>,
          control flow, comparisons, and recursion (including mutual recursion) — deliberately not
          a full C compiler, but enough to demonstrate every classic compiler stage end to end.
        </p>
      </motion.div>
    </section>
  );
}

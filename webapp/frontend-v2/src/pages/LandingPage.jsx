import { Link } from "react-router-dom";
import { ArrowRight } from "lucide-react";
import { PIPELINE_STAGES } from "../pipeline";
import { STAGE_COLORS } from "../stageColors";
import { Button } from "../components/ui/button";
import { cn } from "../lib/utils";

// Short descriptions taken from README.md's "Compiler pipeline" section.
const STAGE_DESCRIPTIONS = {
  SOURCE: "Your .c file",
  LEXER: "Source to tokens",
  AST: "Tokens to syntax tree",
  IR: "Tree to three-address code",
  OPTIMIZER: "Folding, propagation, dead temporaries",
  ENERGY: "Estimated cost in nJ",
  CODEGEN: "Optimized IR to pseudo-assembly",
};

export default function LandingPage() {
  return (
    <section className="mx-auto flex max-w-4xl flex-col gap-8 px-4 py-10 md:px-6 md:py-16">
      <div className="max-w-2xl">
        <h1 className="text-[28px] leading-9 font-semibold tracking-tight text-fg">
          A compiler that estimates what your code costs to run.
        </h1>
        <p className="mt-3 text-[14px] leading-6 text-muted">
          EcoLang is a small, hand-written compiler for a restricted subset of C. It runs a program through lexing,
          parsing, semantic analysis, IR generation and optimization, then estimates the energy the compiled program
          uses, based on a published Cortex-M0 energy model (Georgiou et al., 2021).
        </p>
      </div>

      <ol className="grid grid-cols-2 gap-px overflow-hidden rounded-md border border-line bg-line sm:grid-cols-4 lg:grid-cols-7">
        {PIPELINE_STAGES.map((stage, i) => (
          <li key={stage} className="bg-panel px-3 py-3">
            <div className="flex items-center gap-2 font-mono text-[11px] text-faint">
              <span className={cn("h-1.5 w-1.5 rounded-full", STAGE_COLORS[i].dot)} />
              {String(i).padStart(2, "0")}
            </div>
            <div className="mt-1.5 text-[13px] font-medium text-fg">{stage}</div>
            <div className="mt-0.5 text-[12px] leading-4 text-muted">{STAGE_DESCRIPTIONS[stage]}</div>
          </li>
        ))}
      </ol>

      <div>
        <Button asChild variant="primary">
          <Link to="/compile">
            Open the compiler
            <ArrowRight size={14} strokeWidth={1.75} />
          </Link>
        </Button>
      </div>
    </section>
  );
}

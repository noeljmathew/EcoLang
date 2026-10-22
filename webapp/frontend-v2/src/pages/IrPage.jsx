import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";

// IR is stage 3 - violet (see stageColors.js). Used directly below for
// the "didn't survive optimization" line highlight, tying this page's
// one small accent detail to its own spectrum color instead of the
// neutral UI accent.

// "After" is just data.codegen
// shown plainly (the optimized IR's pseudo-assembly rendering); "Before"
// is checked line-by-line against the full set of "After" lines and
// flagged if it doesn't survive optimization - a presence check, not a
// real diff.
export default function IrPage() {
  const { lastResult } = useCompile();
  const irBefore = lastResult?.irBefore;
  const irAfter = lastResult?.codegen;
  const hasData = irBefore !== undefined && irAfter !== undefined;

  const afterLines = hasData ? new Set(irAfter.split("\n")) : null;
  const beforeLines = hasData ? irBefore.split("\n") : [];

  return (
    <section>
      <PageHeading
        title="Intermediate Representation"
        description="Before and after the optimizer - a highlighted line in “Before” didn't survive optimization."
      />
      {!hasData ? (
        <EmptyState />
      ) : (
        <div className="grid grid-cols-1 gap-5 md:grid-cols-2">
          <div>
            <div className="mb-2 text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
              Before optimization
            </div>
            <div className="overflow-x-auto rounded-xl border border-accent-soft-border bg-paper-raised p-4 font-mono text-[13px] leading-6 shadow-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:shadow-soft-dark">
              {beforeLines.map((line, index) => (
                <div
                  key={index}
                  className={
                    "whitespace-pre " +
                    (afterLines.has(line)
                      ? "text-ink dark:text-ink-dark"
                      : "-mx-4 border-l-2 border-stage-violet bg-stage-violet/10 px-4 text-ink dark:border-stage-violet-dark dark:bg-stage-violet-dark/15 dark:text-ink-dark")
                  }
                >
                  {line}
                </div>
              ))}
            </div>
          </div>
          <div>
            <div className="mb-2 text-xs font-medium tracking-wide text-ink-soft uppercase dark:text-ink-dark-soft">
              After optimization
            </div>
            <pre className="overflow-x-auto rounded-xl border border-accent-soft-border bg-paper-raised p-4 font-mono text-[13px] leading-6 whitespace-pre text-ink shadow-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark dark:shadow-soft-dark">
              {irAfter}
            </pre>
          </div>
        </div>
      )}
    </section>
  );
}

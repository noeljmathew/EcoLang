import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";

// Ported from the old index.html's IR tab. "After" is just data.codegen
// shown plainly (the optimized IR - codegen.cpp's output already *is*
// the optimized IR's pseudo-assembly rendering); "Before" is checked
// line-by-line against the full set of "After" lines and flagged if it
// doesn't survive optimization - a presence check, not a real diff.
export default function IrPage() {
  const { lastResult } = useCompile();
  const irBefore = lastResult?.irBefore;
  const irAfter = lastResult?.codegen;

  if (irBefore === undefined || irAfter === undefined) {
    return (
      <section>
        <h1>IR</h1>
        <EmptyState />
      </section>
    );
  }

  const afterLines = new Set(irAfter.split("\n"));
  const beforeLines = irBefore.split("\n");

  return (
    <section>
      <h1>IR</h1>
      <div className="ir-columns">
        <div className="ir-column">
          <div className="ir-column-label">Before optimization</div>
          <div className="ir-output">
            {beforeLines.map((line, index) => (
              <div
                key={index}
                className={"ir-line" + (afterLines.has(line) ? "" : " ir-line--changed")}
              >
                {line}
              </div>
            ))}
          </div>
        </div>
        <div className="ir-column">
          <div className="ir-column-label">After optimization</div>
          <pre className="ir-output">{irAfter}</pre>
        </div>
      </div>
    </section>
  );
}

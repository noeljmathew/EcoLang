import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import CodeView from "../components/CodeView";
import { PanelHeader } from "../components/Panel";
import { ResizableGroup, ResizableHandle, ResizablePanel } from "../components/ui/resizable";
import { useIsNarrow } from "../lib/useIsNarrow";

// "After" is data.codegen (the optimized IR's pseudo-assembly). Each
// "Before" line is checked against the set of "After" lines and
// highlighted if it does not appear there: a presence check, not a
// real diff.
export default function IrPage() {
  const { lastResult } = useCompile();
  const narrow = useIsNarrow();
  const irBefore = lastResult?.irBefore;
  const irAfter = lastResult?.codegen;
  const hasData = irBefore !== undefined && irAfter !== undefined;

  const afterLines = hasData ? new Set(irAfter.split("\n")) : new Set();
  const beforeLines = hasData ? irBefore.split("\n") : [];
  // 1-based line numbers, as Monaco counts them.
  const removedLineNumbers = beforeLines.flatMap((line, i) => (afterLines.has(line) ? [] : [i + 1]));

  function highlightRemoved(editor) {
    editor.createDecorationsCollection(
      removedLineNumbers.map((lineNumber) => ({
        range: { startLineNumber: lineNumber, startColumn: 1, endLineNumber: lineNumber, endColumn: 1 },
        options: { isWholeLine: true, className: "ir-removed-line", linesDecorationsClassName: "ir-removed-gutter" },
      }))
    );
  }

  return (
    <section className="flex h-full min-h-[480px] flex-col">
      <PageHeading
        title="Intermediate representation"
        description="Before and after the optimizer. Highlighted lines in Before did not survive optimization."
      />
      {!hasData ? (
        <EmptyState />
      ) : (
        <div className="min-h-0 flex-1">
          <ResizableGroup orientation={narrow ? "vertical" : "horizontal"} key={narrow ? "v" : "h"}>
            <ResizablePanel defaultSize="50" minSize="20" className="flex flex-col">
              <PanelHeader label="Before optimization" className="bg-panel">
                <span className="flex items-center gap-2 font-mono">
                  <span className="h-3 w-0.5 bg-stage-violet" />
                  {removedLineNumbers.length} {removedLineNumbers.length === 1 ? "line" : "lines"} removed
                </span>
              </PanelHeader>
              <div className="min-h-0 flex-1">
                <CodeView value={irBefore} onMount={highlightRemoved} />
              </div>
            </ResizablePanel>
            <ResizableHandle />
            <ResizablePanel defaultSize="50" minSize="20" className="flex flex-col">
              <PanelHeader label="After optimization" className="bg-panel">
                <span className="font-mono">{irAfter.split("\n").length} lines</span>
              </PanelHeader>
              <div className="min-h-0 flex-1">
                <CodeView value={irAfter} />
              </div>
            </ResizablePanel>
          </ResizableGroup>
        </div>
      )}
    </section>
  );
}

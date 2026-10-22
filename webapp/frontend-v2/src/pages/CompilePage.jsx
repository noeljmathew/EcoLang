import { useSource } from "../context/SourceContext";
import { SAMPLES } from "../samples";
import PageHeading from "../components/PageHeading";
import SourceEditor from "../components/compile/SourceEditor";
import OutputPanel from "../components/compile/OutputPanel";
import { ResizableGroup, ResizableHandle, ResizablePanel } from "../components/ui/resizable";
import { useIsNarrow } from "../lib/useIsNarrow";

// Editor on the left, Problems/Output on the right (stacked on narrow
// screens). The result itself is shown on each stage page, which reads
// the shared CompileContext.
export default function CompilePage() {
  const { fileName, loadExample } = useSource();
  const narrow = useIsNarrow();

  return (
    <section className="flex h-full min-h-[480px] flex-col">
      <PageHeading
        title="Compile"
        description="Write or load a program in the EcoLang C subset and run it through the full pipeline."
        actions={<ExamplePicker value={fileName} onChange={loadExample} />}
      />
      <div className="min-h-0 flex-1">
        <ResizableGroup orientation={narrow ? "vertical" : "horizontal"} key={narrow ? "v" : "h"}>
          <ResizablePanel defaultSize={narrow ? "60" : "62"} minSize="25">
            <SourceEditor />
          </ResizablePanel>
          <ResizableHandle />
          <ResizablePanel defaultSize={narrow ? "40" : "38"} minSize="20">
            <OutputPanel />
          </ResizablePanel>
        </ResizableGroup>
      </div>
    </section>
  );
}

function ExamplePicker({ value, onChange }) {
  return (
    <label className="flex items-center gap-2 text-[12px] text-muted">
      Load example
      <select
        value={value in SAMPLES ? value : ""}
        onChange={(event) => onChange(event.target.value)}
        className="h-7 rounded-sm border border-line-strong bg-raised px-2 font-mono text-[12px] text-fg focus-visible:ring-2 focus-visible:ring-white/40 focus-visible:outline-none"
      >
        {Object.keys(SAMPLES).map((name) => (
          <option key={name} value={name}>
            {name}
          </option>
        ))}
      </select>
    </label>
  );
}

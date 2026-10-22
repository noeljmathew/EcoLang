import { useState } from "react";
import { CircleAlert, CircleCheck } from "lucide-react";
import { useCompile } from "../../context/CompileContext";
import { PIPELINE_STAGES, determineFailedStageIndex } from "../../pipeline";
import { STAGE_COLORS } from "../../stageColors";
import { Badge } from "../ui/badge";
import { ScrollArea } from "../ui/scroll-area";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "../ui/tabs";
import { cn } from "../../lib/utils";

// Problems: the compiler error from the last attempt, with the stage it
// failed in. Output: a short summary of the last successful compile.
// The panel switches to whichever tab is relevant after each attempt.
export default function OutputPanel() {
  const { status, errorMessage, lastResult } = useCompile();
  const [tab, setTab] = useState(status === "error" ? "problems" : "output");
  const [seenStatus, setSeenStatus] = useState(status);

  // When a compile attempt finishes, jump to the relevant tab. Done
  // during render (React's pattern for adjusting state when an input
  // changes) rather than in an effect.
  if (status !== seenStatus) {
    setSeenStatus(status);
    if (status === "error") setTab("problems");
    if (status === "success") setTab("output");
  }

  const hasProblem = status === "error" && errorMessage;

  return (
    <Tabs value={tab} onValueChange={setTab} className="flex h-full flex-col bg-panel">
      <TabsList>
        <TabsTrigger value="problems">
          Problems
          <span
            className={cn(
              "rounded-sm px-1 font-mono text-[10px]",
              hasProblem ? "bg-danger/15 text-danger" : "bg-hover text-faint"
            )}
          >
            {hasProblem ? 1 : 0}
          </span>
        </TabsTrigger>
        <TabsTrigger value="output">Output</TabsTrigger>
      </TabsList>
      <TabsContent value="problems" className="min-h-0">
        <ScrollArea className="h-full">
          {hasProblem ? <Problem message={errorMessage} /> : <Quiet text="No problems from the last compile." />}
        </ScrollArea>
      </TabsContent>
      <TabsContent value="output" className="min-h-0">
        <ScrollArea className="h-full">
          {status === "loading" ? (
            <Quiet text="Compiling..." />
          ) : lastResult ? (
            <Summary result={lastResult} fresh={status === "success"} />
          ) : (
            <Quiet text="Press Compile (or Cmd/Ctrl+Enter) to run the program through the pipeline." />
          )}
        </ScrollArea>
      </TabsContent>
    </Tabs>
  );
}

function Quiet({ text }) {
  return <p className="p-4 text-[13px] text-faint">{text}</p>;
}

function Problem({ message }) {
  const failedIndex = determineFailedStageIndex(message);
  const stageKnown = failedIndex >= 0;

  return (
    <div className="p-4">
      <div className="mb-3 flex flex-wrap items-center gap-2">
        <CircleAlert size={16} strokeWidth={1.75} className="text-danger" />
        <span className="text-[13px] font-medium text-fg">Compilation failed</span>
        {stageKnown ? (
          <Badge className={STAGE_COLORS[failedIndex].badge}>
            <span className="h-1.5 w-1.5 rounded-full bg-danger" />
            Failed at {PIPELINE_STAGES[failedIndex]}
          </Badge>
        ) : (
          <Badge>No stage information</Badge>
        )}
      </div>
      <pre className="font-mono text-[12px] leading-5 whitespace-pre-wrap text-danger">{message}</pre>
    </div>
  );
}

// Every number here is read straight from the last /compile response.
function Summary({ result, fresh }) {
  const opt = result.optimization ?? {};
  const after = result.energy?.["Total Energy Units"];
  const before = result.energyBefore?.["Total Energy Units"];
  const rows = [
    ["Tokens", result.tokens?.length],
    ["IR instructions", opt["Original IR Instructions"] + " before, " + opt["Optimized IR Instructions"] + " after"],
    ["Energy before optimization", typeof before === "number" ? before.toFixed(6) + " nJ" : "-"],
    ["Energy after optimization", typeof after === "number" ? after.toFixed(6) + " nJ" : "-"],
    ["Estimated energy", result.energy?.["Estimated Energy"] ?? "-"],
  ];

  return (
    <div className="p-4">
      <div className="mb-3 flex items-center gap-2">
        <CircleCheck size={16} strokeWidth={1.75} className="text-stage-emerald" />
        <span className="text-[13px] font-medium text-fg">
          {fresh ? "Compiled successfully" : "Last successful compile"}
        </span>
      </div>
      <dl className="grid grid-cols-[auto_1fr] gap-x-6 gap-y-1.5 font-mono text-[12px]">
        {rows.map(([label, value]) => (
          <div key={label} className="contents">
            <dt className="text-muted">{label}</dt>
            <dd className="text-fg">{value}</dd>
          </div>
        ))}
      </dl>
    </div>
  );
}

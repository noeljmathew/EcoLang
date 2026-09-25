import { useState } from "react";
import { useCompile } from "../context/CompileContext";
import PipelineStrip from "../components/PipelineStrip";
import PageHeading from "../components/PageHeading";

// Ported as-is from the old index.html's SAMPLES constant and the
// textarea's own default content (hello.c in both places).
const SAMPLES = {
  hello: `// Computes the cost of a simple order, including tax, and prints a
// summary. Demonstrates declarations, arithmetic, a function with
// parameters and a return value, and printf.
int addTax(int amount, int taxPercent) {
    return amount + (amount * taxPercent) / 100;
}

int main() {
    int itemPrice = 40;
    int quantity = 3;
    int subtotal = itemPrice * quantity;

    int total = addTax(subtotal, 5);

    printf("Subtotal: %d\\n", subtotal);
    printf("Total with tax: %d\\n", total);

    return 0;
}
`,
  loops: `// Demonstrates a loop accumulating a running total.
int main() {
    int total = 0;
    int i = 1;

    while (i <= 5) {
        total = total + i;
        printf("Added %d, running total = %d\\n", i, total);
        i = i + 1;
    }

    printf("Final total: %d\\n", total);
    return 0;
}
`,
  nested: `// Demonstrates nested loops with a small multiplication table.
int main() {
    int outer = 1;
    while (outer <= 3) {
        int inner = 1;
        while (inner <= 3) {
            int product = outer * inner;
            printf("%d x %d = %d\\n", outer, inner, product);
            inner = inner + 1;
        }
        outer = outer + 1;
    }

    return 0;
}
`,
  recursion: `int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    printf("%d\\n", result);
    return 0;
}
`,
};

// The source editor, sample picker, and Compile action - the compile
// *result* itself is not shown here; each stage (/tokens, /ast, /ir,
// /optimizer, /energy, /codegen) reads the shared CompileContext and
// renders its own slice of it.
export default function CompilePage() {
  const [source, setSource] = useState(SAMPLES.hello);
  const [sample, setSample] = useState("hello");
  const { status, errorMessage, runCompile } = useCompile();

  function handleSampleChange(event) {
    const value = event.target.value;
    setSample(value);
    const code = SAMPLES[value];
    if (code !== undefined) {
      setSource(code);
    }
  }

  async function handleCompile() {
    try {
      await runCompile(source);
    } catch {
      // runCompile already recorded the failure in context (status/
      // errorMessage) - nothing further to do here.
    }
  }

  return (
    <section>
      <PageHeading title="Compile" description="Write or load an EcoLang program and run it through the full pipeline." />

      <PipelineStrip />

      <div className="mb-2 flex flex-wrap items-center justify-between gap-3">
        <label htmlFor="source" className="text-sm font-medium text-ink-soft dark:text-ink-dark-soft">
          Source (EcoLang C subset)
        </label>
        <div className="flex items-center gap-2">
          <label htmlFor="samplePicker" className="text-sm text-ink-soft dark:text-ink-dark-soft">
            Load example:
          </label>
          <select
            id="samplePicker"
            value={sample}
            onChange={handleSampleChange}
            className="rounded-md border border-accent-soft-border bg-paper-raised px-2 py-1.5 text-sm text-ink dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark"
          >
            <option value="hello">hello.c</option>
            <option value="loops">loops.c</option>
            <option value="nested">nested.c</option>
            <option value="recursion">recursion.c</option>
          </select>
        </div>
      </div>

      <textarea
        id="source"
        spellCheck={false}
        value={source}
        onChange={(event) => setSource(event.target.value)}
        className="h-80 w-full resize-y rounded-xl border border-accent-soft-border bg-paper-raised p-4 font-mono text-[13px] leading-6 text-ink shadow-soft focus:border-accent focus:outline-none dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark dark:shadow-soft-dark dark:focus:border-accent-dark"
      />

      <div className="mt-4 flex items-center gap-3">
        <button
          type="button"
          disabled={status === "loading"}
          onClick={handleCompile}
          className="rounded-md bg-accent px-5 py-2 text-sm font-medium text-white shadow-soft transition-all duration-200 hover:-translate-y-0.5 hover:bg-accent-hover hover:shadow-soft-lg disabled:translate-y-0 disabled:cursor-default disabled:bg-ink-soft/40 disabled:shadow-none dark:bg-accent-dark dark:text-paper-dark dark:shadow-soft-dark dark:hover:shadow-soft-dark-lg dark:hover:brightness-110 dark:disabled:bg-ink-dark-soft/30"
        >
          Compile
        </button>
        <span id="status" className="text-sm text-ink-soft dark:text-ink-dark-soft">
          {status === "loading" && "Compiling…"}
          {status === "success" && "Compilation succeeded."}
          {status === "error" && "Compilation failed."}
        </span>
      </div>

      {status === "error" && errorMessage && (
        <div className="mt-6 rounded-xl border-2 border-red-600/70 bg-red-50 p-5 shadow-soft dark:border-red-500/60 dark:bg-red-950/40 dark:shadow-soft-dark">
          <h2 className="mb-2 text-sm font-semibold tracking-wide text-red-700 uppercase dark:text-red-400">
            Compile Error
          </h2>
          <div className="font-mono text-sm whitespace-pre-wrap text-red-800 dark:text-red-300">{errorMessage}</div>
        </div>
      )}
    </section>
  );
}

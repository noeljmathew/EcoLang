import { useState } from "react";
import { useCompile } from "../context/CompileContext";
import PipelineStrip from "../components/PipelineStrip";

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
      <h1>Compile</h1>

      <PipelineStrip />

      <div className="source-header">
        <label htmlFor="source">Source (EcoLang C subset)</label>
        <div className="sample-picker">
          <label htmlFor="samplePicker">Load example:</label>
          <select id="samplePicker" value={sample} onChange={handleSampleChange}>
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
      />

      <div className="controls">
        <button type="button" disabled={status === "loading"} onClick={handleCompile}>
          Compile
        </button>
        <span id="status">
          {status === "loading" && "Compiling..."}
          {status === "success" && "Compilation succeeded."}
          {status === "error" && "Compilation failed."}
        </span>
      </div>

      {status === "error" && errorMessage && (
        <div className="panel error-box">
          <h2 className="section-title section-title--error">Compile Error</h2>
          <div>{errorMessage}</div>
        </div>
      )}
    </section>
  );
}

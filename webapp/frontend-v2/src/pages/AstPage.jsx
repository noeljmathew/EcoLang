import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import AstTree from "../components/AstTree";

// Ported as-is from the old index.html's AST panel (collapse/expand,
// box-drawing connectors - see components/AstTree.jsx). No disclaimer:
// data.ast is real output from the compiler's own parser (dumpAst() in
// src/dump.cpp), not the old independent Python parser.
export default function AstPage() {
  const { lastResult } = useCompile();
  const ast = lastResult?.ast;

  if (!ast) {
    return (
      <section>
        <h1>AST</h1>
        <EmptyState />
      </section>
    );
  }

  return (
    <section>
      <h1>AST</h1>
      <AstTree ast={ast} />
    </section>
  );
}

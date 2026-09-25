import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import AstTree from "../components/AstTree";
import PageHeading from "../components/PageHeading";

// Ported as-is from the old index.html's AST panel (collapse/expand,
// box-drawing connectors - see components/AstTree.jsx). No disclaimer:
// data.ast is real output from the compiler's own parser (dumpAst() in
// src/dump.cpp), not the old independent Python parser.
export default function AstPage() {
  const { lastResult } = useCompile();
  const ast = lastResult?.ast;

  return (
    <section>
      <PageHeading
        title="Abstract Syntax Tree"
        description="The parser's real output, straight from the compiler's own dumpAst() - click a node to collapse or expand it."
      />
      {ast ? <AstTree ast={ast} /> : <EmptyState />}
    </section>
  );
}

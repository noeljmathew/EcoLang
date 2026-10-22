import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import AstTree from "../components/AstTree";
import PageHeading from "../components/PageHeading";
import { PageBody, Panel } from "../components/Panel";

// data.ast is real output from the compiler's own parser (dumpAst() in
// src/dump.cpp), not the old independent Python parser.
export default function AstPage() {
  const { lastResult } = useCompile();
  const ast = lastResult?.ast;

  return (
    <section>
      <PageHeading
        title="Abstract syntax tree"
        description="The parser's real output from dumpAst(). Click a node to collapse or expand it."
      />
      {ast ? (
        <PageBody>
          <Panel>
            <AstTree ast={ast} />
          </Panel>
        </PageBody>
      ) : (
        <EmptyState />
      )}
    </section>
  );
}

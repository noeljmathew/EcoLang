import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";
import { PageBody, Panel, PanelHeader } from "../components/Panel";

// Displays data.tokens: an array of {type, lexeme, line} from the real
// compiler's lexer (dumpTokens() in src/dump.cpp, parsed into rows by
// app.py).
export default function TokensPage() {
  const { lastResult } = useCompile();
  const tokens = lastResult?.tokens;

  return (
    <section>
      <PageHeading title="Tokens" description="The lexer's raw output: every token dumpTokens() produced, in source order." />
      {!tokens ? (
        <EmptyState />
      ) : (
        <PageBody>
          <Panel>
            <PanelHeader label="Token stream">
              <span className="font-mono">{tokens.length} tokens</span>
            </PanelHeader>
            <div className="overflow-x-auto">
              <table className="w-full border-collapse text-[13px]">
                <thead className="text-left text-[11px] tracking-wide text-faint uppercase">
                  <tr className="border-b border-line">
                    <th className="w-14 px-3 py-2 font-medium">#</th>
                    <th className="w-16 px-3 py-2 font-medium">Line</th>
                    <th className="w-48 px-3 py-2 font-medium">Type</th>
                    <th className="px-3 py-2 font-medium">Lexeme</th>
                  </tr>
                </thead>
                <tbody className="font-mono text-[12px]">
                  {tokens.map((token, index) => (
                    <tr key={index} className="border-b border-line/60 last:border-0 hover:bg-hover">
                      <td className="px-3 py-1 text-faint">{index}</td>
                      <td className="px-3 py-1 text-muted">{token.line}</td>
                      <td className="px-3 py-1 text-stage-orange">{token.type}</td>
                      <td className="px-3 py-1 whitespace-pre text-fg">{token.lexeme}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </Panel>
        </PageBody>
      )}
    </section>
  );
}

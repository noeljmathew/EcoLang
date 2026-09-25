import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";
import PageHeading from "../components/PageHeading";

// Displays data.tokens - an array of {type, lexeme, line} from the real
// compiler's lexer (dumpTokens() in src/dump.cpp, parsed into
// structured rows by app.py).
export default function TokensPage() {
  const { lastResult } = useCompile();
  const tokens = lastResult?.tokens;

  return (
    <section>
      <PageHeading
        title="Tokens"
        description="The lexer's raw output - every token dumpTokens() produced, in source order."
      />
      {!tokens ? (
        <EmptyState />
      ) : (
        <div className="overflow-hidden rounded-xl border border-accent-soft-border shadow-soft dark:border-accent-dark-soft dark:shadow-soft-dark">
          <div className="border-b border-accent-soft-border bg-paper-raised px-4 py-2 text-sm text-ink-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark-soft">
            {tokens.length} tokens
          </div>
          <div className="max-h-[70vh] overflow-y-auto">
            <table className="w-full border-collapse text-sm">
              <thead className="sticky top-0 bg-paper-raised text-xs tracking-wide text-ink-soft uppercase dark:bg-paper-dark-raised dark:text-ink-dark-soft">
                <tr>
                  <th className="px-4 py-2 text-left font-medium">#</th>
                  <th className="px-4 py-2 text-left font-medium">Line</th>
                  <th className="px-4 py-2 text-left font-medium">Type</th>
                  <th className="px-4 py-2 text-left font-medium">Lexeme</th>
                </tr>
              </thead>
              <tbody className="font-mono">
                {tokens.map((token, index) => (
                  <tr
                    key={index}
                    className="border-t border-accent-soft-border/60 hover:bg-stage-orange/8 dark:border-accent-dark-soft/60 dark:hover:bg-stage-orange-dark/10"
                  >
                    <td className="px-4 py-1.5 text-ink-soft dark:text-ink-dark-soft">{index}</td>
                    <td className="px-4 py-1.5 text-ink-soft dark:text-ink-dark-soft">{token.line}</td>
                    <td className="px-4 py-1.5 text-stage-orange dark:text-stage-orange-dark">{token.type}</td>
                    <td className="px-4 py-1.5 text-ink dark:text-ink-dark">{token.lexeme}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      )}
    </section>
  );
}

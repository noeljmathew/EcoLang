import { useCompile } from "../context/CompileContext";
import EmptyState from "../components/EmptyState";

// New page (not previously built). Displays data.tokens - an array of
// {type, lexeme, line} from the real compiler's lexer (dumpTokens() in
// src/dump.cpp, parsed into structured rows by app.py). Plain table for
// now; styling comes later.
export default function TokensPage() {
  const { lastResult } = useCompile();
  const tokens = lastResult?.tokens;

  if (!tokens) {
    return (
      <section>
        <h1>Tokens</h1>
        <EmptyState />
      </section>
    );
  }

  return (
    <section>
      <h1>Tokens</h1>
      <p>{tokens.length} tokens</p>
      <table>
        <thead>
          <tr>
            <th>#</th>
            <th>Line</th>
            <th>Type</th>
            <th>Lexeme</th>
          </tr>
        </thead>
        <tbody>
          {tokens.map((token, index) => (
            <tr key={index}>
              <td>{index}</td>
              <td>{token.line}</td>
              <td>{token.type}</td>
              <td>{token.lexeme}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}

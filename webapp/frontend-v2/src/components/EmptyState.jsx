import { Link } from "react-router-dom";

// Shown by every stage page when there's no compiled result in context
// yet (nothing compiled this session, and nothing persisted from a
// previous one either).
export default function EmptyState() {
  return (
    <p className="rounded-lg border border-dashed border-accent-soft-border bg-paper-raised px-6 py-8 text-ink-soft dark:border-accent-dark-soft dark:bg-paper-dark-raised dark:text-ink-dark-soft">
      Nothing compiled yet — go to the{" "}
      <Link to="/compile" className="font-medium text-accent underline underline-offset-2 hover:text-accent-hover dark:text-accent-dark">
        compiler
      </Link>
      .
    </p>
  );
}

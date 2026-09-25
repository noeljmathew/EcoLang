import { useLocation } from "react-router-dom";
import { stageBadge } from "../stages";

// Shared heading block for every stage page: the subtle "STAGE N ·
// LABEL" tag the design brief asks for, plus the serif display title
// and an optional one-line description. Looks up its own badge from
// the current route so pages don't each have to pass stage props by
// hand and risk drifting from PIPELINE_STAGES.
export default function PageHeading({ title, description }) {
  const location = useLocation();
  const badge = stageBadge(location.pathname);

  return (
    <div className="mb-6">
      {badge && (
        <span className={"mb-2 inline-block rounded-full border px-3 py-1 text-xs font-medium tracking-wide uppercase " + badge.color.badge}>
          Stage {badge.index} · {badge.label}
        </span>
      )}
      <h1 className="font-display text-4xl font-semibold tracking-tight text-ink dark:text-ink-dark">
        {title}
      </h1>
      {description && (
        <p className="mt-2 max-w-2xl text-base text-ink-soft dark:text-ink-dark-soft">
          {description}
        </p>
      )}
    </div>
  );
}

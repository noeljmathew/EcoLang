import { useLocation } from "react-router-dom";
import { stageBadge } from "../stages";
import { Badge } from "./ui/badge";
import { cn } from "../lib/utils";

// Compact header shared by every page: an optional stage badge (looked
// up from the route), the title, and a one-line description. `badge`
// overrides the route lookup (used by the Water page). `actions` render
// on the right.
export default function PageHeading({ title, description, badge, actions }) {
  const location = useLocation();
  const stage = stageBadge(location.pathname);

  return (
    <div className="flex shrink-0 flex-wrap items-center gap-x-4 gap-y-2 border-b border-line px-4 py-3 md:px-6">
      <div className="min-w-0 flex-1">
        <div className="flex flex-wrap items-center gap-2.5">
          {stage && (
            <Badge className={stage.color.badge}>
              <span className={cn("h-1.5 w-1.5 rounded-full", stage.color.dot)} />
              Stage {stage.index} · {stage.label}
            </Badge>
          )}
          {badge}
          <h1 className="text-[20px] leading-7 font-semibold tracking-tight text-fg">{title}</h1>
        </div>
        {description && <p className="mt-0.5 text-[13px] text-muted">{description}</p>}
      </div>
      {actions && <div className="flex items-center gap-2">{actions}</div>}
    </div>
  );
}

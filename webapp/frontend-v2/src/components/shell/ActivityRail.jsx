import { NavLink, useMatch } from "react-router-dom";
import { HOME_ITEM, STAGE_ITEMS, WATER_ITEM } from "../../stages";
import { STAGE_COLORS } from "../../stageColors";
import { Separator } from "../ui/separator";
import { Tooltip } from "../ui/tooltip";
import { cn } from "../../lib/utils";

// VS Code-style icon rail (desktop only; MobileMenu replaces it on
// narrow screens). The active page gets a 2px accent bar on the left.
// The active state comes from useMatch and className is a plain string:
// the Tooltip wraps the link in Radix's Slot, which merges className as
// a string and would break NavLink's function form of className.
function RailLink({ item, stageIndex }) {
  const Icon = item.icon;
  const isActive = useMatch({ path: item.to, end: true }) !== null;
  return (
    <Tooltip content={item.label}>
      <NavLink
        to={item.to}
        end={item.to === "/"}
        aria-label={item.label}
        className={cn(
          "relative flex h-10 w-12 items-center justify-center text-faint transition-colors duration-150 hover:text-fg focus-visible:text-fg focus-visible:outline-none",
          isActive && "text-fg before:absolute before:inset-y-2 before:left-0 before:w-0.5 before:bg-accent"
        )}
      >
        <Icon size={18} strokeWidth={1.75} />
        {stageIndex !== undefined && (
          <span className={cn("absolute right-2.5 bottom-2 h-1 w-1 rounded-full", STAGE_COLORS[stageIndex].dot)} />
        )}
      </NavLink>
    </Tooltip>
  );
}

export default function ActivityRail() {
  return (
    <nav aria-label="Pages" className="hidden w-12 shrink-0 flex-col border-r border-line bg-panel md:flex">
      <RailLink item={HOME_ITEM} />
      <Separator className="mx-auto my-1 w-6" />
      {STAGE_ITEMS.map((item) => (
        <RailLink key={item.to} item={item} stageIndex={item.stage} />
      ))}
      {/* Water is derived from Energy, not an 8th compiler stage. */}
      <Separator className="mx-auto my-1 w-6" aria-label="Beyond the pipeline" />
      <RailLink item={{ ...WATER_ITEM, label: "Water Impact (beyond the pipeline)" }} />
    </nav>
  );
}

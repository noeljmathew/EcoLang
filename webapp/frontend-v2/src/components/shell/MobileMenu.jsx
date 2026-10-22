import { NavLink } from "react-router-dom";
import { HOME_ITEM, STAGE_ITEMS, WATER_ITEM } from "../../stages";
import { STAGE_COLORS } from "../../stageColors";
import { cn } from "../../lib/utils";

// The page list shown under the top bar on narrow screens, where the
// activity rail is hidden. `onNavigate` closes the menu.
function MenuLink({ item, stageIndex, onNavigate }) {
  const Icon = item.icon;
  return (
    <NavLink
      to={item.to}
      end={item.to === "/"}
      onClick={onNavigate}
      className={({ isActive }) =>
        cn(
          "flex h-10 items-center gap-3 rounded-md px-3 text-[13px] text-muted hover:bg-hover hover:text-fg",
          isActive && "bg-hover text-fg"
        )
      }
    >
      <Icon size={16} strokeWidth={1.75} />
      <span className="flex-1">{item.label}</span>
      {stageIndex !== undefined && <span className={cn("h-1.5 w-1.5 rounded-full", STAGE_COLORS[stageIndex].dot)} />}
    </NavLink>
  );
}

export default function MobileMenu({ onNavigate }) {
  return (
    <nav
      aria-label="Pages"
      className="absolute inset-x-0 top-10 z-40 border-b border-line bg-panel p-2 md:hidden"
    >
      <MenuLink item={HOME_ITEM} onNavigate={onNavigate} />
      {STAGE_ITEMS.map((item) => (
        <MenuLink key={item.to} item={item} stageIndex={item.stage} onNavigate={onNavigate} />
      ))}
      <div className="mt-2 border-t border-line px-3 pt-3 pb-1 text-[11px] font-medium tracking-wide text-faint uppercase">
        Beyond the pipeline
      </div>
      <MenuLink item={WATER_ITEM} onNavigate={onNavigate} />
    </nav>
  );
}

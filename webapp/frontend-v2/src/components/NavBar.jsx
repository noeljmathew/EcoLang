import { NavLink } from "react-router-dom";
import { useDarkMode } from "../useDarkMode";
import { STAGE_COLORS } from "../stageColors";

// The 7 real pipeline stages, in order - each one is an actual phase
// this compiler runs, one route per stage.
const PIPELINE_LINKS = [
  { to: "/compile", label: "Compile" },
  { to: "/tokens", label: "Tokens" },
  { to: "/ast", label: "AST" },
  { to: "/ir", label: "IR" },
  { to: "/optimizer", label: "Optimizer" },
  { to: "/energy", label: "Energy" },
  { to: "/codegen", label: "Code Generation" },
];

// Energy is stage 5 - emerald. Water is derived from Energy's own
// numbers, not a compiler phase of its own, so its nav entry borrows
// Energy's color rather than getting a new one from the 7-stage
// spectrum, and lives in its own group below a divider (see the
// "Beyond the pipeline" label below) so the nav never implies an 8th
// stage exists.
const WATER_STAGE = STAGE_COLORS[5];

const linkClasses = ({ isActive }) =>
  [
    "block rounded-md px-3 py-2 text-sm transition-colors",
    isActive
      ? "bg-accent-soft font-medium text-accent dark:bg-accent-dark-soft dark:text-accent-dark"
      : "text-ink-soft hover:bg-paper-raised hover:text-ink dark:text-ink-dark-soft dark:hover:bg-paper-dark-raised dark:hover:text-ink-dark",
  ].join(" ");

const waterLinkClasses = ({ isActive }) =>
  [
    "block rounded-md px-3 py-2 text-sm transition-colors",
    isActive
      ? "font-medium " + WATER_STAGE.badge
      : "text-ink-soft hover:bg-paper-raised hover:text-ink dark:text-ink-dark-soft dark:hover:bg-paper-dark-raised dark:hover:text-ink-dark",
  ].join(" ");

// Persistent across every route (rendered once by App.jsx around the
// routed content, not per-page).
export default function NavBar() {
  const { theme, toggle } = useDarkMode();

  return (
    <nav className="flex w-56 shrink-0 flex-col border-r border-accent-soft-border/60 px-4 py-6 dark:border-accent-dark-soft">
      <div className="mb-6 px-3">
        <span className="font-display text-lg font-semibold text-ink dark:text-ink-dark">
          EcoLang
        </span>
      </div>
      <ul className="flex flex-col gap-1">
        <li>
          <NavLink to="/" end className={linkClasses}>
            Home
          </NavLink>
        </li>
        {PIPELINE_LINKS.map((link) => (
          <li key={link.to}>
            <NavLink to={link.to} className={linkClasses}>
              {link.label}
            </NavLink>
          </li>
        ))}
      </ul>

      {/* Divider + label: Water isn't a pipeline stage (there's no
          "Water" phase in src/main.cpp), it's a derived illustration
          built from Energy's own numbers - kept visually separate so
          the nav never implies an 8th compiler stage exists. */}
      <div className="mt-4 flex-1 border-t border-accent-soft-border/60 pt-4 dark:border-accent-dark-soft/60">
        <div className="mb-1 px-3 text-[10px] font-semibold tracking-widest text-ink-soft/70 uppercase dark:text-ink-dark-soft/70">
          Beyond the pipeline
        </div>
        <ul>
          <li>
            <NavLink to="/water" className={waterLinkClasses}>
              Water Impact
            </NavLink>
          </li>
        </ul>
      </div>

      <button
        type="button"
        onClick={toggle}
        className="mt-4 rounded-md border border-accent-soft-border px-3 py-2 text-left text-xs font-medium text-ink-soft shadow-soft transition-all duration-200 hover:-translate-y-0.5 hover:text-ink hover:shadow-soft-lg dark:border-accent-dark-soft dark:text-ink-dark-soft dark:shadow-soft-dark dark:hover:text-ink-dark dark:hover:shadow-soft-dark-lg"
      >
        {theme === "dark" ? "☾ Dark mode" : "☀ Light mode"}
      </button>
    </nav>
  );
}

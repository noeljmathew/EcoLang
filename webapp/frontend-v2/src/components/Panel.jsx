import { cn } from "../lib/utils";

// A bordered panel, one step lighter than the page background.
export function Panel({ className, ...props }) {
  return <div className={cn("overflow-hidden rounded-md border border-line bg-panel", className)} {...props} />;
}

// A small uppercase label row at the top of a panel, with optional
// content (counts, legends) on the right.
export function PanelHeader({ label, children, className }) {
  return (
    <div
      className={cn(
        "flex h-9 items-center justify-between gap-3 border-b border-line px-3 text-[11px] font-medium tracking-wide text-faint uppercase",
        className
      )}
    >
      <span>{label}</span>
      {children && <span className="tracking-normal normal-case">{children}</span>}
    </div>
  );
}

// One headline figure: a label above a value, with an optional note.
export function Stat({ label, value, note, valueClassName, className }) {
  return (
    <div className={cn("min-w-0 bg-panel px-4 py-3", className)}>
      <div className="text-[11px] font-medium tracking-wide text-faint uppercase">{label}</div>
      <div className={cn("mt-1 truncate font-mono text-[18px] text-fg", valueClassName)}>{value}</div>
      {note && <div className="mt-0.5 text-[12px] text-muted">{note}</div>}
    </div>
  );
}

// Padded body for pages that scroll as a whole.
export function PageBody({ className, ...props }) {
  return <div className={cn("flex flex-col gap-4 p-4 md:p-6", className)} {...props} />;
}

import { cn } from "../../lib/utils";

// Small outlined label. Stage badges pass a STAGE_COLORS[i].badge class.
export function Badge({ className, ...props }) {
  return (
    <span
      className={cn(
        "inline-flex items-center gap-1.5 rounded-sm border border-line-strong px-1.5 py-0.5 font-mono text-[11px] leading-none font-medium tracking-wide text-muted uppercase",
        className
      )}
      {...props}
    />
  );
}

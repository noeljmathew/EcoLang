import { Group, Panel, Separator } from "react-resizable-panels";
import { cn } from "../../lib/utils";

// Thin wrappers over react-resizable-panels. Sizes given as strings are
// percentages (e.g. defaultSize="60").
export function ResizableGroup({ className, ...props }) {
  return <Group className={cn("h-full w-full", className)} {...props} />;
}

export const ResizablePanel = Panel;

// A 1px divider with a wider invisible hit area for dragging.
export function ResizableHandle({ className, ...props }) {
  return (
    <Separator
      className={cn(
        "relative bg-line transition-colors duration-150 hover:bg-white/25 focus-visible:bg-white/40 focus-visible:outline-none",
        "aria-[orientation=vertical]:w-px aria-[orientation=horizontal]:h-px",
        "after:absolute after:inset-0 aria-[orientation=vertical]:after:-inset-x-1 aria-[orientation=horizontal]:after:-inset-y-1",
        className
      )}
      {...props}
    />
  );
}

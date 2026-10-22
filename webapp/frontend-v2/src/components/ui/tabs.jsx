import * as TabsPrimitive from "@radix-ui/react-tabs";
import { cn } from "../../lib/utils";

export const Tabs = TabsPrimitive.Root;

export function TabsList({ className, ...props }) {
  return (
    <TabsPrimitive.List
      className={cn("flex h-9 shrink-0 items-stretch gap-1 border-b border-line px-2", className)}
      {...props}
    />
  );
}

// The active tab gets a 1px underline in the neutral accent.
export function TabsTrigger({ className, ...props }) {
  return (
    <TabsPrimitive.Trigger
      className={cn(
        "-mb-px inline-flex items-center gap-1.5 border-b border-transparent px-2 text-[12px] font-medium text-muted transition-colors duration-150 hover:text-fg focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-white/40 data-[state=active]:border-accent data-[state=active]:text-fg",
        className
      )}
      {...props}
    />
  );
}

export function TabsContent({ className, ...props }) {
  return <TabsPrimitive.Content className={cn("min-h-0 flex-1 focus-visible:outline-none", className)} {...props} />;
}

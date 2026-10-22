import { clsx } from "clsx";
import { twMerge } from "tailwind-merge";

// Joins class names and lets a later Tailwind class override an earlier
// conflicting one (e.g. a caller's "px-2" replacing a default "px-3").
export function cn(...inputs) {
  return twMerge(clsx(inputs));
}

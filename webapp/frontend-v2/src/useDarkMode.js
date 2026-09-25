import { useEffect, useState } from "react";

const STORAGE_KEY = "ecolang:theme";

function getInitialTheme() {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored === "light" || stored === "dark") return stored;
  } catch {
    // localStorage unavailable (private mode, quota) - fall through to
    // the OS preference below.
  }
  if (typeof window !== "undefined" && window.matchMedia?.("(prefers-color-scheme: dark)").matches) {
    return "dark";
  }
  return "light";
}

// Light is the default/primary experience (per the design brief); dark
// is opt-in via a toggle in the nav, persisted across sessions the same
// way CompileContext persists its own state - a plain localStorage key,
// separate from that one so the two concerns never collide.
export function useDarkMode() {
  const [theme, setTheme] = useState(getInitialTheme);

  useEffect(() => {
    document.documentElement.classList.toggle("dark", theme === "dark");
    try {
      localStorage.setItem(STORAGE_KEY, theme);
    } catch {
      // Persistence is best-effort only.
    }
  }, [theme]);

  function toggle() {
    setTheme((current) => (current === "dark" ? "light" : "dark"));
  }

  return { theme, toggle };
}

import { useEffect, useState } from "react";

const QUERY = "(max-width: 767px)";

// True below the md breakpoint (768px), where split panes stack
// vertically instead of sitting side by side.
export function useIsNarrow() {
  const [narrow, setNarrow] = useState(() => window.matchMedia(QUERY).matches);

  useEffect(() => {
    const media = window.matchMedia(QUERY);
    const onChange = () => setNarrow(media.matches);
    media.addEventListener("change", onChange);
    return () => media.removeEventListener("change", onChange);
  }, []);

  return narrow;
}

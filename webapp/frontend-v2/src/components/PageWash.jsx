import { useLocation } from "react-router-dom";
import { pageAccentColor } from "../stages";

// A very subtle corner gradient wash, tinted with the current page's
// accent color - atmospheric, not a colored background. Fixed behind
// the routed content (App.jsx renders this once, outside <Routes>, so
// it re-reads the current accent color on every navigation instead of
// each page needing its own copy). Renders nothing on routes with no
// accent color (the landing page has its own bespoke multi-hue
// treatment, since it doesn't belong to a single stage).
export default function PageWash() {
  const location = useLocation();
  const color = pageAccentColor(location.pathname);
  if (!color) return null;

  return (
    <div
      aria-hidden="true"
      className={
        "pointer-events-none fixed inset-0 -z-10 bg-radial-[at_100%_0%] to-60% to-transparent " +
        color.washFrom
      }
    />
  );
}

import { useEffect, useState } from "react";
import { Link, useLocation } from "react-router-dom";
import { LoaderCircle, Menu, Play, X } from "lucide-react";
import { useCompile } from "../../context/CompileContext";
import { useSource } from "../../context/SourceContext";
import { Button } from "../ui/button";
import MobileMenu from "./MobileMenu";

const IS_MAC = typeof navigator !== "undefined" && /Mac|iPhone|iPad/.test(navigator.platform);

// Project name, current file name, and the Compile button. Cmd/Ctrl+Enter
// compiles from any page (the editor also binds it, since Monaco keeps
// that key combination for itself while focused).
export default function TopBar() {
  const { status } = useCompile();
  const { fileName, edited, compile } = useSource();
  const [menuOpen, setMenuOpen] = useState(false);
  const location = useLocation();

  useEffect(() => {
    function onKeyDown(event) {
      if ((event.metaKey || event.ctrlKey) && event.key === "Enter") {
        event.preventDefault();
        compile();
      }
    }
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [compile]);

  const loading = status === "loading";

  return (
    <header className="relative flex h-10 shrink-0 items-center gap-3 border-b border-line bg-panel pr-2 pl-1 md:pl-3">
      <Button
        variant="ghost"
        size="icon"
        className="md:hidden"
        aria-label={menuOpen ? "Close menu" : "Open menu"}
        aria-expanded={menuOpen}
        onClick={() => setMenuOpen((open) => !open)}
      >
        {menuOpen ? <X size={16} strokeWidth={1.75} /> : <Menu size={16} strokeWidth={1.75} />}
      </Button>

      <Link to="/" className="text-[13px] font-semibold tracking-tight text-fg">
        EcoLang
      </Link>
      <span className="text-faint">/</span>
      <span className="truncate font-mono text-[12px] text-muted" title="Current file">
        {fileName}
      </span>
      {edited && <span className="font-mono text-[11px] text-faint">(edited)</span>}

      <div className="ml-auto flex items-center gap-2">
        <span className="hidden font-mono text-[11px] text-faint sm:inline">{IS_MAC ? "⌘ Enter" : "Ctrl Enter"}</span>
        <Button variant="primary" size="sm" onClick={compile} disabled={loading}>
          {loading ? (
            <LoaderCircle size={14} strokeWidth={1.75} className="animate-spin" />
          ) : (
            <Play size={14} strokeWidth={1.75} />
          )}
          {loading ? "Compiling" : "Compile"}
        </Button>
      </div>

      {menuOpen && <MobileMenu key={location.pathname} onNavigate={() => setMenuOpen(false)} />}
    </header>
  );
}

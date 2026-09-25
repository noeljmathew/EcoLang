import { NavLink } from "react-router-dom";

const LINKS = [
  { to: "/", label: "Home", end: true },
  { to: "/compile", label: "Compile" },
  { to: "/tokens", label: "Tokens" },
  { to: "/ast", label: "AST" },
  { to: "/ir", label: "IR" },
  { to: "/optimizer", label: "Optimizer" },
  { to: "/energy", label: "Energy" },
  { to: "/codegen", label: "Code Generation" },
];

// Persistent across every route (rendered once by App.jsx around the
// routed <Outlet/>, not per-page) - a plain link list for now, real
// nav styling comes in a later pass.
export default function NavBar() {
  return (
    <nav className="nav-bar">
      <div className="nav-bar-title">EcoLang Compiler</div>
      <ul>
        {LINKS.map((link) => (
          <li key={link.to}>
            <NavLink to={link.to} end={link.end}>
              {link.label}
            </NavLink>
          </li>
        ))}
      </ul>
    </nav>
  );
}

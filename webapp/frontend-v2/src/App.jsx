import { Route, Routes } from "react-router-dom";
import "./App.css";
import NavBar from "./components/NavBar";
import LandingPage from "./pages/LandingPage";
import CompilePage from "./pages/CompilePage";
import TokensPage from "./pages/TokensPage";
import AstPage from "./pages/AstPage";
import IrPage from "./pages/IrPage";
import OptimizerPage from "./pages/OptimizerPage";
import EnergyPage from "./pages/EnergyPage";
import CodegenPage from "./pages/CodegenPage";

// The nav bar is rendered once here, outside <Routes>, so it's
// persistent across every route instead of being part of any one page.
export default function App() {
  return (
    <div className="app-shell">
      <NavBar />
      <main className="app-content">
        <Routes>
          <Route path="/" element={<LandingPage />} />
          <Route path="/compile" element={<CompilePage />} />
          <Route path="/tokens" element={<TokensPage />} />
          <Route path="/ast" element={<AstPage />} />
          <Route path="/ir" element={<IrPage />} />
          <Route path="/optimizer" element={<OptimizerPage />} />
          <Route path="/energy" element={<EnergyPage />} />
          <Route path="/codegen" element={<CodegenPage />} />
        </Routes>
      </main>
    </div>
  );
}

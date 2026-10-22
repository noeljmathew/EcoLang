import { Route, Routes, useLocation } from "react-router-dom";
import TopBar from "./components/shell/TopBar";
import ActivityRail from "./components/shell/ActivityRail";
import StatusBar from "./components/shell/StatusBar";
import { TooltipProvider } from "./components/ui/tooltip";
import LandingPage from "./pages/LandingPage";
import CompilePage from "./pages/CompilePage";
import TokensPage from "./pages/TokensPage";
import AstPage from "./pages/AstPage";
import IrPage from "./pages/IrPage";
import OptimizerPage from "./pages/OptimizerPage";
import EnergyPage from "./pages/EnergyPage";
import CodegenPage from "./pages/CodegenPage";
import WaterPage from "./pages/WaterPage";

// The IDE shell: top bar, activity rail, the routed page, status bar.
// The page wrapper is keyed by path so each navigation replays the
// short fade-in (animate-page-in in index.css).
export default function App() {
  const location = useLocation();

  return (
    <TooltipProvider delayDuration={300}>
      <div className="flex h-full flex-col">
        <TopBar />
        <div className="flex min-h-0 flex-1">
          <ActivityRail />
          <main className="min-w-0 flex-1 overflow-y-auto">
            <div key={location.pathname} className="h-full animate-page-in">
              <Routes location={location}>
                <Route path="/" element={<LandingPage />} />
                <Route path="/compile" element={<CompilePage />} />
                <Route path="/tokens" element={<TokensPage />} />
                <Route path="/ast" element={<AstPage />} />
                <Route path="/ir" element={<IrPage />} />
                <Route path="/optimizer" element={<OptimizerPage />} />
                <Route path="/energy" element={<EnergyPage />} />
                <Route path="/codegen" element={<CodegenPage />} />
                <Route path="/water" element={<WaterPage />} />
              </Routes>
            </div>
          </main>
        </div>
        <StatusBar />
      </div>
    </TooltipProvider>
  );
}

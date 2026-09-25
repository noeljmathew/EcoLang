import { AnimatePresence } from "framer-motion";
import { Route, Routes, useLocation } from "react-router-dom";
import NavBar from "./components/NavBar";
import PageTransition from "./components/PageTransition";
import PageWash from "./components/PageWash";
import LandingPage from "./pages/LandingPage";
import CompilePage from "./pages/CompilePage";
import TokensPage from "./pages/TokensPage";
import AstPage from "./pages/AstPage";
import IrPage from "./pages/IrPage";
import OptimizerPage from "./pages/OptimizerPage";
import EnergyPage from "./pages/EnergyPage";
import CodegenPage from "./pages/CodegenPage";
import WaterPage from "./pages/WaterPage";

// The nav bar is rendered once here, outside <Routes>, so it's
// persistent across every route instead of being part of any one page.
// <Routes location={location}> pinned to the current location (rather
// than letting Routes read it implicitly) is what lets <AnimatePresence
// mode="wait"> see the outgoing page during its exit animation before
// the new one mounts - the standard pairing for React Router + Framer
// Motion page transitions.
export default function App() {
  const location = useLocation();

  return (
    <div className="flex min-h-screen bg-paper text-ink dark:bg-paper-dark dark:text-ink-dark">
      <PageWash />
      <NavBar />
      <main className="flex-1 overflow-x-hidden px-6 py-6 md:px-10">
        <div className="mx-auto max-w-5xl">
          <AnimatePresence mode="wait">
            <Routes location={location} key={location.pathname}>
              <Route path="/" element={<PageTransition><LandingPage /></PageTransition>} />
              <Route path="/compile" element={<PageTransition><CompilePage /></PageTransition>} />
              <Route path="/tokens" element={<PageTransition><TokensPage /></PageTransition>} />
              <Route path="/ast" element={<PageTransition><AstPage /></PageTransition>} />
              <Route path="/ir" element={<PageTransition><IrPage /></PageTransition>} />
              <Route path="/optimizer" element={<PageTransition><OptimizerPage /></PageTransition>} />
              <Route path="/energy" element={<PageTransition><EnergyPage /></PageTransition>} />
              <Route path="/codegen" element={<PageTransition><CodegenPage /></PageTransition>} />
              <Route path="/water" element={<PageTransition><WaterPage /></PageTransition>} />
            </Routes>
          </AnimatePresence>
        </div>
      </main>
    </div>
  );
}

import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { BrowserRouter } from "react-router-dom";
import "./index.css";
import "./lib/monaco";
import App from "./App.jsx";
import { CompileProvider } from "./context/CompileContext.jsx";
import { SourceProvider } from "./context/SourceContext.jsx";

createRoot(document.getElementById("root")).render(
  <StrictMode>
    <BrowserRouter>
      <CompileProvider>
        <SourceProvider>
          <App />
        </SourceProvider>
      </CompileProvider>
    </BrowserRouter>
  </StrictMode>
);

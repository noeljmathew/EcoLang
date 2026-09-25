import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { BrowserRouter } from "react-router-dom";
import "./index.css";
import App from "./App.jsx";
import { CompileProvider } from "./context/CompileContext.jsx";

createRoot(document.getElementById("root")).render(
  <StrictMode>
    <BrowserRouter>
      <CompileProvider>
        <App />
      </CompileProvider>
    </BrowserRouter>
  </StrictMode>
);

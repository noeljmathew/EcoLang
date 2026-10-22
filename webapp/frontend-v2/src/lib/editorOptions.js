// Shared Monaco options: line numbers, no word wrap, no minimap, and the
// ecolang-dark theme defined in lib/monaco.js.
export const EDITOR_OPTIONS = {
  fontFamily: "Geist Mono, ui-monospace, Menlo, monospace",
  fontSize: 13,
  lineHeight: 20,
  minimap: { enabled: false },
  wordWrap: "off",
  scrollBeyondLastLine: false,
  renderLineHighlight: "line",
  padding: { top: 12, bottom: 12 },
  automaticLayout: true,
  overviewRulerLayers: 0,
  overviewRulerBorder: false,
  hideCursorInOverviewRuler: true,
  scrollbar: { verticalScrollbarSize: 8, horizontalScrollbarSize: 8 },
};

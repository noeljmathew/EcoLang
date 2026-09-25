// Simple, consistent-stroke line icons, one per pipeline stage - hand-
// drawn inline SVG rather than an icon library dependency (keeps the
// bundle small and gives exact control over the "one coherent family"
// look: 24x24 viewBox, 1.6 stroke width, round joins throughout).
// currentColor so each icon inherits its stage's color from the parent.
const ICON_PATHS = [
  // 0 SOURCE - a source file: page outline + a folded corner + text lines
  <>
    <path d="M6 3h8l4 4v14a1 1 0 0 1-1 1H6a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1Z" />
    <path d="M14 3v4h4" />
    <path d="M8 13h8M8 16h8M8 10h3" />
  </>,
  // 1 LEXER - source split into discrete tokens (a scan/segmentation mark)
  <>
    <rect x="3.5" y="9" width="4" height="6" rx="1" />
    <rect x="10" y="9" width="4" height="6" rx="1" />
    <rect x="16.5" y="9" width="4" height="6" rx="1" />
    <path d="M3.5 6.5h17M3.5 17.5h17" strokeDasharray="1.5 2.5" />
  </>,
  // 2 AST - a small syntax tree: root branching to two children
  <>
    <circle cx="12" cy="5" r="2" />
    <circle cx="6" cy="14" r="2" />
    <circle cx="18" cy="14" r="2" />
    <circle cx="6" cy="20" r="1.6" />
    <circle cx="10" cy="20" r="1.6" />
    <path d="M12 7v3M10.5 9 6 12M13.5 9l4.5 3M6 16v1.2M8.3 18.5 6.9 19.7" />
  </>,
  // 3 IR - stacked three-address-code lines
  <>
    <rect x="4" y="5" width="16" height="3.4" rx="1" />
    <rect x="4" y="10.3" width="11" height="3.4" rx="1" />
    <rect x="4" y="15.6" width="14" height="3.4" rx="1" />
  </>,
  // 4 OPTIMIZER - a gear
  <>
    <circle cx="12" cy="12" r="3.2" />
    <path d="M12 3.5v2.3M12 18.2v2.3M20.5 12h-2.3M5.8 12H3.5M17.7 6.3l-1.6 1.6M7.9 16.1l-1.6 1.6M17.7 17.7l-1.6-1.6M7.9 7.9 6.3 6.3" />
  </>,
  // 5 ENERGY - a bolt
  <>
    <path d="M13 3 5 13h5.5L10 21l8-11h-5.5L13 3Z" />
  </>,
  // 6 CODEGEN - code chevrons
  <>
    <path d="m9 8-4.5 4L9 16" />
    <path d="m15 8 4.5 4L15 16" />
  </>,
];

export default function StageIcon({ index, className }) {
  return (
    <svg
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.6"
      strokeLinecap="round"
      strokeLinejoin="round"
      className={className}
      aria-hidden="true"
    >
      {ICON_PATHS[index]}
    </svg>
  );
}

import { useState } from "react";
import { ChevronDown, ChevronRight, ChevronsDownUp, ChevronsUpDown } from "lucide-react";
import { Button } from "./ui/button";
import { PanelHeader } from "./Panel";

// Originally written for the first single-page frontend's AST panel,
// since removed. Stage 1: raw AST -> "display tree" of {label, line,
// children} nodes, framework-agnostic on purpose. Stage 2 below builds
// the actual elements, as JSX.

function isSimpleLeafExpr(expr) {
  return expr.type === "Number" || expr.type === "Variable";
}

function collapsedValueText(expr) {
  return expr.type === "Number" ? String(expr.value) : expr.name;
}

// Returns a flat infix string ("itemPrice * quantity") if `expr` is a
// single literal/variable, or one binary/comparison operator deep with
// simple operands on both sides. Returns null for anything more complex
// (two+ nested operators, or a call anywhere), left as a full tree instead.
function tryCollapseExpr(expr) {
  if (isSimpleLeafExpr(expr)) return collapsedValueText(expr);
  if (expr.type === "Binary" || expr.type === "Comparison") {
    if (isSimpleLeafExpr(expr.left) && isSimpleLeafExpr(expr.right)) {
      return collapsedValueText(expr.left) + " " + expr.operator + " " + collapsedValueText(expr.right);
    }
  }
  return null;
}

// Fully expands an expression into a display node. A Call with all-simple
// arguments still renders as one compact leaf line ("addTax(subtotal, 5)").
function expressionToDisplay(expr) {
  if (expr.type === "Number") return { label: String(expr.value), line: expr.line, children: [] };
  if (expr.type === "Variable") return { label: expr.name, line: expr.line, children: [] };
  if (expr.type === "String") return { label: JSON.stringify(expr.value), line: expr.line, children: [] };
  if (expr.type === "Call") {
    if (expr.args.every(isSimpleLeafExpr)) {
      const argsText = expr.args.map(collapsedValueText).join(", ");
      return { label: expr.callee + "(" + argsText + ")", line: expr.line, children: [] };
    }
    return { label: expr.callee + "(...)", line: expr.line, children: expr.args.map(expressionToDisplay) };
  }
  if (expr.type === "Binary" || expr.type === "Comparison") {
    return {
      label: expr.operator,
      line: expr.line,
      children: [expressionToDisplay(expr.left), expressionToDisplay(expr.right)],
    };
  }
  return { label: expr.type, line: expr.line, children: [] };
}

function assignmentLikeToDisplay(stmt, isDeclaration) {
  const value = isDeclaration ? stmt.initializer : stmt.value;
  if (value === null || value === undefined) {
    return { label: isDeclaration ? "int " + stmt.name : stmt.name, line: stmt.line, children: [] };
  }
  const flat = tryCollapseExpr(value);
  if (flat !== null) {
    return { label: stmt.name + " = " + flat, line: stmt.line, children: [] };
  }
  return { label: stmt.name + " =", line: stmt.line, children: [expressionToDisplay(value)] };
}

function printfToDisplay(stmt) {
  const formatText = '"' + stmt.format + '"';
  if (stmt.args.every(isSimpleLeafExpr)) {
    const argsText = stmt.args.map(collapsedValueText).join(", ");
    const label = "printf(" + formatText + (argsText ? ", " + argsText : "") + ")";
    return { label, line: stmt.line, children: [] };
  }
  return { label: "printf(" + formatText + ")", line: stmt.line, children: stmt.args.map(expressionToDisplay) };
}

function returnToDisplay(stmt) {
  if (!stmt.value) {
    return { label: "Return", line: stmt.line, children: [] };
  }
  const flat = tryCollapseExpr(stmt.value);
  if (flat !== null) {
    return { label: "Return " + flat, line: stmt.line, children: [] };
  }
  return { label: "Return", line: stmt.line, children: [expressionToDisplay(stmt.value)] };
}

function callStatementToDisplay(call) {
  if (call.args.every(isSimpleLeafExpr)) {
    const argsText = call.args.map(collapsedValueText).join(", ");
    return { label: call.callee + "(" + argsText + ")", line: call.line, children: [] };
  }
  return { label: "Call: " + call.callee, line: call.line, children: call.args.map(expressionToDisplay) };
}

function blockStatements(block) {
  return block.statements.map(statementToDisplay);
}

function ifToDisplay(stmt) {
  const condText = tryCollapseExpr(stmt.condition);
  const children = [];
  if (condText === null) {
    children.push(expressionToDisplay(stmt.condition));
  }
  children.push(...blockStatements(stmt.then));

  if (stmt.else) {
    if (stmt.else.type === "If") {
      const elseIfNode = ifToDisplay(stmt.else);
      elseIfNode.label = "Else " + elseIfNode.label;
      children.push(elseIfNode);
    } else {
      children.push({ label: "Else", line: stmt.else.line, children: blockStatements(stmt.else) });
    }
  }

  const label = condText !== null ? "If (" + condText + ")" : "If";
  return { label, line: stmt.line, children };
}

function whileToDisplay(stmt) {
  const condText = tryCollapseExpr(stmt.condition);
  const children = [];
  if (condText === null) {
    children.push(expressionToDisplay(stmt.condition));
  }
  children.push(...blockStatements(stmt.body));

  const label = condText !== null ? "While (" + condText + ")" : "While";
  return { label, line: stmt.line, children };
}

function forToDisplay(stmt) {
  const initDisp = statementToDisplay(stmt.init);
  const updateDisp = statementToDisplay(stmt.update);
  const condText = tryCollapseExpr(stmt.condition);
  const allSimple = initDisp.children.length === 0 && updateDisp.children.length === 0 && condText !== null;

  if (allSimple) {
    const label = "For (" + initDisp.label + "; " + condText + "; " + updateDisp.label + ")";
    return { label, line: stmt.line, children: blockStatements(stmt.body) };
  }

  initDisp.label = "Init: " + initDisp.label;
  updateDisp.label = "Update: " + updateDisp.label;
  const condNode = condText !== null
    ? { label: "Condition: " + condText, line: stmt.condition.line, children: [] }
    : expressionToDisplay(stmt.condition);
  if (condText === null) condNode.label = "Condition: " + condNode.label;

  const children = [initDisp, condNode, updateDisp, ...blockStatements(stmt.body)];
  return { label: "For", line: stmt.line, children };
}

function blockToDisplay(block) {
  return { label: "Block", line: block.line, children: blockStatements(block) };
}

function statementToDisplay(stmt) {
  switch (stmt.type) {
    case "VariableDeclaration": return assignmentLikeToDisplay(stmt, true);
    case "Assignment": return assignmentLikeToDisplay(stmt, false);
    case "Printf": return printfToDisplay(stmt);
    case "Return": return returnToDisplay(stmt);
    case "CallStatement": return callStatementToDisplay(stmt.call);
    case "If": return ifToDisplay(stmt);
    case "While": return whileToDisplay(stmt);
    case "For": return forToDisplay(stmt);
    case "Block": return blockToDisplay(stmt);
    default: return { label: stmt.type, line: stmt.line, children: [] };
  }
}

function functionToDisplay(fn) {
  const children = [];
  if (fn.params.length > 0) {
    children.push({
      label: "Parameters",
      line: null,
      children: fn.params.map((p) => ({ label: p + " : int", line: null, children: [] })),
    });
  }
  children.push(...blockStatements(fn.body));
  return { label: "Function: " + fn.name, line: fn.line, children };
}

function programToDisplay(program) {
  return { label: "Program", line: null, children: program.functions.map(functionToDisplay) };
}

// -- stage 2: display tree -> JSX with box-drawing connectors -----------
// Same connector characters and prefix math as the original. Collapse
// state is a Set of collapsed node ids ("root", "root.0", "root.0.1",
// ...) held by AstTree. Clicking a node with children toggles it.

function TreeNode({ node, nodeId, prefix, isLast, isRoot, collapsed, onToggle }) {
  const hasChildren = !!(node.children && node.children.length);
  const isCollapsed = collapsed.has(nodeId);
  const childPrefix = isRoot ? "" : prefix + (isLast ? "    " : "│   ");
  const Chevron = isCollapsed ? ChevronRight : ChevronDown;
  const hasLine = node.line !== null && node.line !== undefined;

  return (
    <div className="font-mono text-[12px] leading-6 whitespace-pre">
      <div className="group flex items-center hover:bg-hover">
        <span className="text-faint/70">{isRoot ? "" : prefix + (isLast ? "└── " : "├── ")}</span>
        {hasChildren ? (
          <button
            type="button"
            onClick={() => onToggle(nodeId)}
            aria-expanded={!isCollapsed}
            className="inline-flex items-center gap-1 rounded-sm text-fg hover:text-stage-rose focus-visible:ring-2 focus-visible:ring-white/40 focus-visible:outline-none"
          >
            <Chevron size={12} strokeWidth={1.75} className="text-faint" />
            {node.label}
          </button>
        ) : (
          <span className="text-fg">{node.label}</span>
        )}
        {hasChildren && isCollapsed && <span className="text-faint"> ...</span>}
        {hasLine && <span className="ml-3 text-[11px] text-faint opacity-0 group-hover:opacity-100">line {node.line}</span>}
      </div>
      {hasChildren && !isCollapsed &&
        node.children.map((child, index) => (
          <TreeNode
            key={index}
            node={child}
            nodeId={nodeId + "." + index}
            prefix={childPrefix}
            isLast={index === node.children.length - 1}
            isRoot={false}
            collapsed={collapsed}
            onToggle={onToggle}
          />
        ))}
    </div>
  );
}

// Every node id that has children, for "Collapse all".
function collapsibleIds(node, nodeId, out = []) {
  if (node.children && node.children.length) {
    out.push(nodeId);
    node.children.forEach((child, index) => collapsibleIds(child, nodeId + "." + index, out));
  }
  return out;
}

// `ast` is the raw AST JSON from the backend (data.ast, dumpAst()'s
// output, forwarded through app.py unchanged).
export default function AstTree({ ast }) {
  const [collapsed, setCollapsed] = useState(() => new Set());
  const tree = programToDisplay(ast);

  function toggle(nodeId) {
    setCollapsed((prev) => {
      const next = new Set(prev);
      if (next.has(nodeId)) {
        next.delete(nodeId);
      } else {
        next.add(nodeId);
      }
      return next;
    });
  }

  // Collapse all keeps the root and its functions open, so the list of
  // functions stays visible.
  function collapseAll() {
    const ids = collapsibleIds(tree, "root").filter((id) => id.split(".").length > 2);
    setCollapsed(new Set(ids));
  }

  return (
    <>
      <PanelHeader label="Syntax tree">
        <span className="flex items-center gap-1">
          <Button variant="ghost" size="sm" onClick={() => setCollapsed(new Set())}>
            <ChevronsUpDown size={14} strokeWidth={1.75} />
            Expand all
          </Button>
          <Button variant="ghost" size="sm" onClick={collapseAll}>
            <ChevronsDownUp size={14} strokeWidth={1.75} />
            Collapse all
          </Button>
        </span>
      </PanelHeader>
      <div className="overflow-x-auto px-3 py-2">
        <TreeNode node={tree} nodeId="root" prefix="" isLast isRoot collapsed={collapsed} onToggle={toggle} />
      </div>
    </>
  );
}

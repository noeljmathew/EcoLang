import { Link } from "react-router-dom";
import { FileCode } from "lucide-react";
import { Button } from "./ui/button";

// Shown by every stage page when there is no compiled result yet
// (nothing compiled this session, and nothing persisted from before).
export default function EmptyState() {
  return (
    <div className="m-4 flex flex-col items-start gap-3 rounded-md border border-dashed border-line-strong p-6 md:m-6">
      <p className="text-[13px] text-muted">Nothing compiled yet. Compile a program to see this stage.</p>
      <Button asChild size="sm">
        <Link to="/compile">
          <FileCode size={14} strokeWidth={1.75} />
          Open the compiler
        </Link>
      </Button>
    </div>
  );
}

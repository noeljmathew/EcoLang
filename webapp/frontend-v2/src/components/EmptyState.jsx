import { Link } from "react-router-dom";

// Shown by every stage page when there's no compiled result in context
// yet (nothing compiled this session, and nothing persisted from a
// previous one either).
export default function EmptyState() {
  return (
    <p className="empty-state">
      Nothing compiled yet — go to the <Link to="/compile">compiler</Link>.
    </p>
  );
}

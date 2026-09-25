import { motion } from "framer-motion";

// Wraps each routed page so App.jsx's <AnimatePresence> can fade/slide
// between them on navigation. Purely presentational - renders whatever
// page element it's given, untouched.
export default function PageTransition({ children }) {
  return (
    <motion.div
      initial={{ opacity: 0, y: 8 }}
      animate={{ opacity: 1, y: 0 }}
      exit={{ opacity: 0, y: -8 }}
      transition={{ duration: 0.25, ease: "easeOut" }}
    >
      {children}
    </motion.div>
  );
}

**Good configuration:**
smt.arith.solver=2 can solve the problem with 11 conflicts, and the non-linear reasoning centers around bounds conflicts.

**Bad configuration:**
smt.arith.solver=6 requires a lot of conflicts and the bounds analysis misses conflicts that smt.arith.solver=2 finds.

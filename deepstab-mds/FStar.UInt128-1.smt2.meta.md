**Good configuration:**
smt.arith.solver=2 can consistently solve the problem with a small number of conflicts, and the non-linear reasoning centers around bounds conflicts.

**Bad configuration:**
smt.arith.solver=6 with some random seeds the problem requires a lot of conflicts and solving time blows up by more than 10x. The bounds analysis misses conflicts that smt.arith.solver=2 finds.

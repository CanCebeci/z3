import z3
import z3.z3

def foo():
    pass

def main():
    s = z3.Solver()
    onc = z3.OnClause(s, foo)

    print(s.check())

    # This is needed to prevent a memory leak
    # (Debug mode prints: WARNING: Uncollected memory: 0: 13Z3_solver_ref)
    del z3.z3._my_hacky_class

if __name__ == '__main__':
    main()

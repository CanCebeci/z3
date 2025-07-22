import z3
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--smt", help="SMT file location", dest="smt", required=True)

    args = parser.parse_args()

    problem = z3.parse_smt2_file(args.smt)
    s = z3.Solver()
    s.add(problem)

    onc = z3.OnClause(s, lambda pr, deps, clause : print(pr, clause))

    print(s.check())

if __name__ == '__main__':
    main()

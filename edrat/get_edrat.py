import z3
import z3.z3
import argparse

declared_funcs = {'aux'}


def handle_define_let(clause):
    # The 'clause' is a pair [<binding>, <expr>]
    assert len(clause) == 2
    binding = clause[0]
    expr = clause[1]

    
    for child in expr.children():
        # Children may contain undeclared funcs/constants. Declare them.
        # huh.. is_app returns True on numeral literals. We'll have to check for apps the dirty way
        if z3.z3._ast_kind(child.ctx, child) == z3.Z3_APP_AST:
            func_name = child.decl().name()
            if func_name not in declared_funcs:                            
                declared_funcs.add(func_name) 
                domain = [str(child.decl().domain(i)) for i in range(child.decl().arity())]
                
                print(f'(declare-fun {func_name} ({" ".join(domain)}) {child.decl().range()})')

        #! TODO: also handle sort declarations
        #! TODO: rewrite aux(id) to aux!id

    print(f'(define-let {binding} {expr})')


def clause_eh(proof, deps, clause):
    match proof.decl().name():
        case "define-let":
            handle_define_let(clause)
        case "define-literal":
            print(f'(define-literal {clause[0]} {clause[1]})')
        case "assumption":
            print(f'a {" ".join([str(lit) for lit in clause])} 0')
        case "smt":
            print(f't {" ".join([str(lit) for lit in clause])} 0')
        case _:
            raise Exception(f'Unexpected proof step from Z3: {proof}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--smt", help="SMT file location", dest="smt", required=True)

    args = parser.parse_args()

    problem = z3.parse_smt2_file(args.smt)
    s = z3.Solver()
    s.add(problem)

    onc = z3.OnClause(s, clause_eh)

    print(s.check())

if __name__ == '__main__':
    main()

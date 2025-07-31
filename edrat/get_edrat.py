import z3
import z3.z3
import argparse

declared_funcs = {'aux'}


def expr_to_str(expr):
    #rewrite aux(id) to aux!id
    if z3.is_app(expr) and expr.decl().name().startswith('aux'):
        # Sole child is a constant id.
        assert expr.num_args() == 1
        assert expr.arg(0).num_args() == 0
        return 'aux!' + str(expr.arg(0))
    
    if z3.is_app(expr) and not z3.is_const(expr):
        args_str = [expr_to_str(expr.arg(i)) for i in range(expr.num_args())]
        return f'({expr.decl().name()} {" ".join(args_str)})'

    # Z3py may format the sexpr over multiple lines, so we remove \n explicitly
    return str(expr.sexpr()).replace('\n', '')


def print_declarations(clause):
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
                domain = [child.decl().domain(i).sexpr() for i in range(child.decl().arity())]
                
                print(f'(declare-fun {func_name} ({" ".join(domain)}) {child.decl().range().sexpr()})')

        #! TODO: also handle sort declarations

def clause_to_str(clause):
    return " ".join([str(lit) for lit in clause])

def clause_eh(proof, deps, clause):
    match proof.decl().name():
        case "debug":
            print(f'; debug: {clause_to_str(clause)}')
            # pass
        case "define-let":
            print_declarations(clause)
            print(f'(define-let {expr_to_str(clause[0])} {expr_to_str(clause[1])})')
        case "define-literal":
            print(f'(define-literal {expr_to_str(clause[0])} {expr_to_str(clause[1])})')
        case "assumption":
            print(f'a {clause_to_str(clause)} 0')
        case "smt":
            print(f't {clause_to_str(clause)} 0')
        case "inst":    # instances are smt clauses
            print(f't {clause_to_str(clause)} 0')
        case "farkas":    # farkas justification are smt clauses??
            print(f't {clause_to_str(clause)} 0')
        case "bound":    # farkas justification are smt clauses??
            print(f't {clause_to_str(clause)} 0')
        case "del":
            print(f'd {clause_to_str(clause)} 0')
        case "rup":
            print(f'{clause_to_str(clause)} 0')
        case "tseitin":
            #! TODO: check with Bruno that this is the right thing to do.
            print(f'a {clause_to_str(clause)} 0')
        case _:
            raise Exception(f'Unexpected proof step from Z3: {proof}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--smt", help="SMT file location", dest="smt", required=True)

    args = parser.parse_args()

    problem = z3.parse_smt2_file(args.smt)
    s = z3.Solver()
    z3.set_param("sat.euf", True)
    z3.set_param("tactic.default_tactic", "smt")
    s.add(problem)

    onc = z3.OnClause(s, clause_eh)

    if (s.check() == 'unsat'):
        print(0)

if __name__ == '__main__':
    main()

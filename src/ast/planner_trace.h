/*++
Copyright (c) 2026 Microsoft Corporation

Module Name:

    planner_trace.h

Abstract:

    Printing for smt.planner_trace (standard solver tracing format).

    Terms are written directly to the stream as single-line SMT-LIB, without
    let-aliases. Unlike mk_ismt2_pp, this creates no AST nodes (not even in the
    format manager, which shares the trace stream and would pollute the
    axiom-profiler log), so enabling the trace cannot perturb the solver.

--*/
#pragma once

#include "ast/ast.h"
#include <ostream>

class planner_trace_printer {
    std::ostream & m_out;
    svector<symbol> m_names; // bound variable names, innermost last

    void display_symbol(symbol const & s);
    void display_sort(sort * s);
    void display_app(app * t);
    void display_quantifier(quantifier * q);
    void display_numeral(rational const & val, bool is_int);
public:
    planner_trace_printer(std::ostream & out): m_out(out) {}
    void display(expr * e);
};

// `[eq] <lhs> <rhs>`: the two closed terms are equal (equivalent, for Bool).
// Terms with free variables are skipped, as are repeats of the previous pair.
void planner_trace_eq(std::ostream & out, expr * lhs, expr * rhs);

// `[eq] <fml> true`: a formula the solver now holds as true, e.g. a definition
// introduced by preprocessing.
void planner_trace_fact(ast_manager & m, std::ostream & out, expr * fml);

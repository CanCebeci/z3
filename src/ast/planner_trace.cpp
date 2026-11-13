/*++
Copyright (c) 2026 Microsoft Corporation

Module Name:

    planner_trace.cpp

Abstract:

    Printing for smt.planner_trace. See planner_trace.h.

--*/
#include "ast/planner_trace.h"
#include "ast/has_free_vars.h"
#include "ast/arith_decl_plugin.h"
#include "ast/bv_decl_plugin.h"
#include "util/smt2_util.h"
#include <sstream>

void planner_trace_printer::display_symbol(symbol const & s) {
    // numerical symbols print as k!<n> and need no quoting
    if (!s.is_numerical() && is_smt2_quoted_symbol(s))
        m_out << mk_smt2_quoted_symbol(s);
    else
        m_out << s;
}

void planner_trace_printer::display_sort(sort * s) {
    unsigned n = s->get_num_parameters();
    unsigned first = 0;
    // datatype sorts carry their own name as the first parameter
    if (n > 0 && s->get_parameter(0).is_symbol() && s->get_parameter(0).get_symbol() == s->get_name())
        first = 1;
    if (n == first) {
        display_symbol(s->get_name());
        return;
    }
    if (s->get_family_id() == bv_family_id && s->get_decl_kind() == BV_SORT) {
        m_out << "(_ BitVec " << s->get_parameter(0).get_int() << ")"; // internally named `bv`
        return;
    }
    bool all_int = true;
    for (unsigned i = first; i < n; ++i)
        all_int &= s->get_parameter(i).is_int();
    // indexed sorts, e.g. (_ BitVec 32); parametric sorts, e.g. (Array Int Int)
    m_out << (all_int ? "(_ " : "(");
    display_symbol(s->get_name());
    for (unsigned i = first; i < n; ++i) {
        parameter const & p = s->get_parameter(i);
        m_out << " ";
        if (p.is_ast() && is_sort(p.get_ast()))
            display_sort(to_sort(p.get_ast()));
        else
            m_out << p;
    }
    m_out << ")";
}

void planner_trace_printer::display_numeral(rational const & val, bool is_int) {
    bool neg = val.is_neg();
    rational abs_val = neg ? -val : val;
    if (neg)
        m_out << "(- ";
    if (is_int)
        m_out << abs_val;
    else if (abs_val.is_int())
        m_out << abs_val << ".0";
    else
        m_out << "(/ " << numerator(abs_val) << ".0 " << denominator(abs_val) << ".0)";
    if (neg)
        m_out << ")";
}

void planner_trace_printer::display_app(app * t) {
    func_decl * d = t->get_decl();
    if (is_app_of(t, arith_family_id, OP_NUM)) {
        display_numeral(d->get_parameter(0).get_rational(), d->get_parameter(1).get_int() != 0);
        return;
    }
    if (is_app_of(t, bv_family_id, OP_BV_NUM)) {
        m_out << "(_ bv" << d->get_parameter(0).get_rational() << " " << d->get_parameter(1).get_int() << ")";
        return;
    }
    // Integer parameters make an indexed identifier, e.g. (_ extract 7 0);
    // other parameters (e.g. datatype bookkeeping) are not part of the name.
    unsigned np = d->get_num_parameters();
    bool indexed = np > 0;
    for (unsigned i = 0; i < np; ++i)
        indexed &= d->get_parameter(i).is_int();
    unsigned num_args = t->get_num_args();
    if (num_args > 0)
        m_out << "(";
    if (indexed) {
        m_out << "(_ ";
        display_symbol(d->get_name());
        for (unsigned i = 0; i < np; ++i)
            m_out << " " << d->get_parameter(i).get_int();
        m_out << ")";
    }
    else
        display_symbol(d->get_name());
    for (expr * arg : *t) {
        m_out << " ";
        display(arg);
    }
    if (num_args > 0)
        m_out << ")";
}

void planner_trace_printer::display_quantifier(quantifier * q) {
    switch (q->get_kind()) {
    case forall_k: m_out << "(forall ("; break;
    case exists_k: m_out << "(exists ("; break;
    default:       m_out << "(lambda ("; break;
    }
    unsigned n = q->get_num_decls();
    for (unsigned i = 0; i < n; ++i) {
        m_out << (i > 0 ? " (" : "(");
        display_symbol(q->get_decl_name(i));
        m_out << " ";
        display_sort(q->get_decl_sort(i));
        m_out << ")";
    }
    m_out << ") ";

    // De Bruijn index 0 is the last declared variable
    unsigned old_sz = m_names.size();
    for (unsigned i = 0; i < n; ++i)
        m_names.push_back(q->get_decl_name(i));

    bool has_qid = q->get_qid() != symbol::null && !q->get_qid().is_numerical();
    bool annotated = q->get_num_patterns() > 0 || q->get_num_no_patterns() > 0 || has_qid;
    if (annotated)
        m_out << "(! ";
    display(q->get_expr());
    for (unsigned i = 0; i < q->get_num_patterns(); ++i) {
        m_out << " :pattern (";
        bool first = true;
        for (expr * p : *to_app(q->get_pattern(i))) {
            if (!first)
                m_out << " ";
            first = false;
            display(p);
        }
        m_out << ")";
    }
    for (unsigned i = 0; i < q->get_num_no_patterns(); ++i) {
        m_out << " :no-pattern ";
        display(q->get_no_pattern(i));
    }
    if (has_qid) {
        m_out << " :qid ";
        display_symbol(q->get_qid());
    }
    if (annotated)
        m_out << ")";
    m_out << ")";

    m_names.shrink(old_sz);
}

void planner_trace_printer::display(expr * e) {
    switch (e->get_kind()) {
    case AST_VAR: {
        unsigned idx = to_var(e)->get_idx();
        if (idx < m_names.size())
            display_symbol(m_names[m_names.size() - 1 - idx]);
        else
            m_out << "(:var " << idx << ")"; // free variable; not expected here
        break;
    }
    case AST_QUANTIFIER:
        display_quantifier(to_quantifier(e));
        break;
    case AST_APP:
        display_app(to_app(e));
        break;
    default:
        UNREACHABLE();
    }
}

static bool is_closed(expr * e) {
    if (is_app(e))
        return to_app(e)->is_ground();
    return is_quantifier(e) && !has_free_vars(e);
}

void planner_trace_eq(std::ostream & out, expr * lhs, expr * rhs) {
    // A rewrite is often reported twice in a row: by the rewriter for the root
    // term, then by the caller for the whole formula.
    static unsigned s_last_lhs = UINT_MAX, s_last_rhs = UINT_MAX;
    if (lhs == rhs || !is_closed(lhs) || !is_closed(rhs))
        return;
    if (lhs->get_id() == s_last_lhs && rhs->get_id() == s_last_rhs)
        return;
    s_last_lhs = lhs->get_id();
    s_last_rhs = rhs->get_id();
    // Distinct terms can print alike, e.g. the numeral -128 and the application
    // (- 128) it is folded from; such an equality says nothing in SMT-LIB.
    std::ostringstream l, r;
    planner_trace_printer(l).display(lhs);
    planner_trace_printer(r).display(rhs);
    if (l.str() == r.str())
        return;
    out << "[eq] " << l.str() << " " << r.str() << "\n";
}

void planner_trace_fact(ast_manager & m, std::ostream & out, expr * fml) {
    if (m.is_true(fml))
        return;
    planner_trace_eq(out, fml, m.mk_true());
}

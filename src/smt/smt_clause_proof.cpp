/*++
Copyright (c) 2019 Microsoft Corporation

Module Name:

    smt_clause_proof.cpp

Author:

    Nikolaj Bjorner (nbjorner) 2019-03-15

Revision History:

--*/
#include "smt/smt_clause_proof.h"
#include "smt/smt_context.h"
#include "ast/ast_pp.h"
#include "ast/ast_ll_pp.h"
#include <iostream>

#include <signal.h>

namespace smt {
    
    clause_proof::clause_proof(context& ctx):
        ctx(ctx), m(ctx.get_manager()), m_lits(m), m_pp(m),
        m_assumption(m), m_rup(m), m_del(m), m_smt(m), m_theory_conflict(m) {
        
        auto proof_log = ctx.get_fparams().m_proof_log;
        m_has_log = proof_log.is_non_empty_string();
        m_enabled = ctx.get_fparams().m_clause_proof || m_has_log;        
    }

    void clause_proof::init_pp_out() {
        if (m_has_log && !m_pp_out) {
            static unsigned id = 0;
            auto proof_log = ctx.get_fparams().m_proof_log;
            std::string log_name = proof_log.str();
            if (id > 0)
                log_name = std::to_string(id) + log_name;
            ++id;
            m_pp_out = alloc(std::ofstream, log_name);
            if (!*m_pp_out)
                throw default_exception(std::string("Could not open file ") + proof_log.str());
        }
    }

    clause_proof::status clause_proof::kind2st(clause_kind k) {
        switch (k) {
        case CLS_AUX: 
            return status::assumption;
        case CLS_TH_AXIOM:
            return status::th_assumption;
        case CLS_LEARNED:
            return status::lemma;
        case CLS_TH_LEMMA:
            return status::th_lemma;
        default:
            UNREACHABLE();
            return status::lemma;
        }
    }

    proof_ref clause_proof::justification2proof(status st, justification* j) {
        proof* r = nullptr;
        if (j)
            r = j->mk_proof(ctx.get_cr());
        if (r) 
            return proof_ref(r, m);
        if (!is_enabled())
            return proof_ref(m);
        switch (st) {
        case status::assumption:
            if (!m_assumption) 
                m_assumption = m.mk_const("assumption", m.mk_proof_sort());
            return m_assumption;
        case status::lemma:
            if (!m_rup)
                m_rup = m.mk_const("rup", m.mk_proof_sort());
            return m_rup;
        case status::th_lemma:
        case status::th_assumption:
            if (!m_smt)
                m_smt = m.mk_const("smt", m.mk_proof_sort());
            return m_smt;
        case status::th_conflict:
            if (!m_theory_conflict)
                m_theory_conflict = m.mk_const("theory-conflict", m.mk_proof_sort());
            return m_theory_conflict;
        case status::deleted:
            if (!m_del)
                m_del = m.mk_const("del", m.mk_proof_sort());
            return m_del;
        }
        UNREACHABLE();
        return proof_ref(m);
    }

    void clause_proof::add(clause& c, literal_buffer const* simp_lits) {
        if (!is_enabled())
            return;
        justification* j = c.get_justification();
        auto st = kind2st(c.get_kind());
        auto pr = justification2proof(st, j);
        CTRACE(mk_clause, pr.get(), tout << mk_bounded_pp(pr, m, 4) << "\n";);
        update(c, st, pr, simp_lits);        
    }

    void clause_proof::add(unsigned n, literal const* lits, clause_kind k, justification* j) {
        if (!is_enabled())
            return;
        auto st = kind2st(k);
        auto pr = justification2proof(st, j);
        CTRACE(mk_clause, pr.get(), tout << mk_bounded_pp(pr, m, 4) << "\n";);
        m_lits.reset();
        for (unsigned i = 0; i < n; ++i) 
            m_lits.push_back(ctx.literal2expr(lits[i]));
        update(st, m_lits, pr);
    }


    void clause_proof::shrink(clause& c, unsigned new_size) {
        if (!is_enabled())
            return;
        m_lits.reset();
        for (unsigned i = 0; i < new_size; ++i) 
            m_lits.push_back(ctx.literal2expr(c[i]));
        auto p = justification2proof(status::lemma, nullptr);
        update(status::lemma, m_lits, p);
        for (unsigned i = new_size; i < c.get_num_literals(); ++i) 
            m_lits.push_back(ctx.literal2expr(c[i]));
        p = justification2proof(status::deleted, nullptr);
        update(status::deleted, m_lits, p);
    }

    void clause_proof::add(literal lit, clause_kind k, justification* j) {
        if (!is_enabled())
            return;
        m_lits.reset();
        m_lits.push_back(ctx.literal2expr(lit));
        auto st = kind2st(k);
        auto pr = justification2proof(st, j);
        update(st, m_lits, pr);
    }

    void clause_proof::add(literal lit1, literal lit2, clause_kind k, justification* j, literal_buffer const* simp_lits) {
        if (!is_enabled())
            return;
        m_lits.reset();
        m_lits.push_back(ctx.literal2expr(lit1));
        m_lits.push_back(ctx.literal2expr(lit2));
        if (simp_lits) 
            for (auto lit : *simp_lits)
                m_lits.push_back(ctx.literal2expr(~lit));
        auto st = kind2st(k);
        auto pr = justification2proof(st, j);
        update(st, m_lits, pr);
    }

    void clause_proof::get_literal_dependencies(literal l, std::set<unsigned> &deps_acc) {
        // std::cerr << ctx.literal2expr(l) << '\n';

        SASSERT(ctx.get_assignment(l) != l_undef);
        SASSERT(ctx.get_assignment(l) == l_true);

        b_justification j = ctx.get_justification(l.var());
        // std::cerr << j.get_kind()  << "\n";

        switch (j.get_kind()) {
        case b_justification::AXIOM:
            // no dependency
            break;
        case b_justification::BIN_CLAUSE: 
            // Can I find where the clause came from?
            // literal other_lit = j.get_literal();
            SASSERT(false); // Not implemented yet. Disabled binary clause optimization for now.
            break;
        case b_justification::CLAUSE: {
            clause *c = j.get_clause();
            // If this clause was generated by a conflict, that conflict is a dependency.
            // std::cerr << "Clause kind: " << c->get_kind()  << "\n";
            if (c->get_kind() == CLS_LEARNED) {
                unsigned d = ctx.clause2conflict_id(c);
                SASSERT(d >= 0);
                deps_acc.insert(d);
            }

            // justification *c_jst = c->get_justification();

            for (literal c_lit : *c) {
                if (c_lit == l)
                    continue;

                // Dependencies of other assigned literals are transitive dependencies.
                // std::cerr << " and transitively (through "<< ~c_lit <<" on: \n";
                // TODO: we can save time here by not recursing on the same literals twice.
                get_literal_dependencies(~c_lit, deps_acc);
            }
            break;
        }
        case b_justification::JUSTIFICATION: {
            // This includes theory justifications as well as input assertions.
            // I am not sure how to deal with theory justifications yet.

            // I have temporarily modified the internalizer to make sure we can distinguish asserted units easily.
            // I am sure Z3 already has a way to do this, but I can't find it easily now.
            proof *pr = j.get_justification()->mk_proof(ctx.get_cr());
            // std::cerr << "Proof: " << mk_pp(pr, m) << "\n";
            if (pr == m.mk_const("cc-hack-assserted", m.mk_proof_sort())) {
                // There are no dependencies for input assertions
                /*NOP*/
            } else if (pr == m.mk_const("cc-hack-learned-unit", m.mk_proof_sort())) {
                auto ctj = dynamic_cast<unit_conflict_tracking_justification *>(j.get_justification());
                SASSERT(ctj);
                deps_acc.insert(ctj->get_conflict_id());
            } else {
                SASSERT(false); // Not implemented yet
            }

            break;
        }
        default:
            UNREACHABLE();
            break;
        }
    }

    void clause_proof::propagate_conflict(literal lit, justification const& jst, literal_vector const& ante) {
        if (!is_enabled())
            return;
        m_lits.reset();
        for (literal l : ante)
            m_lits.push_back(ctx.literal2expr(~l));
        m_lits.push_back(ctx.literal2expr(lit));
        proof_ref pr(m.mk_app(symbol("theory-conflict"), 0, nullptr, m.mk_proof_sort()), m);
        update(clause_proof::status::th_conflict, m_lits, pr);

        // Compute conflict dependencies
        literal_vector conflict_lits;
        for (literal l : ante)
            conflict_lits.push_back(l);
        conflict_lits.push_back(~lit);
        
        std::set<unsigned> deps;
        for (literal l : conflict_lits) {
            get_literal_dependencies(l, deps);
        }

        unsigned conflict_id = ctx.get_num_conflicts() - 1;
        // -- log --
        // std::cerr << "Conflict " << conflict_id << " depends on:";
        // for (unsigned i : deps) {
        //    std::cerr << ' ' << i;
        // }
        // std::cerr << '\n';
        // std::cerr << "(lits: ";
        // for (auto e : m_lits) {
        //    std::cerr << ' ' << mk_pp(e, ctx.get_manager());
        // }
        // std::cerr << '\n';
        // ---------

        ctx.m_conflict_dependencies.reserve(conflict_id + 1);
        for (unsigned i : deps) {
           ctx.m_conflict_dependencies[conflict_id].push_back(i);
        }
        
        
    }

    void clause_proof::del(clause& c) {
        update(c, status::deleted, justification2proof(status::deleted, nullptr), nullptr);
    }

    std::ostream& clause_proof::display_literals(std::ostream& out, expr_ref_vector const& v) {
        for (expr* e : v)
            if (m.is_not(e, e))                
                m_pp.display_expr_def(out << " (not ", e) << ")";
            else
                m_pp.display_expr_def(out << " ", e);
        return out;
    }

    std::ostream& clause_proof::display_hint(std::ostream& out, proof* p) {
        if (p)
            m_pp.display_expr_def(out << " ", p);
        return out;
    }

    void clause_proof::declare(std::ostream& out, expr* e) {
        m_pp.collect(e);
        m_pp.display_decls(out);
        m.is_not(e, e);
        m_pp.define_expr(out, e);
    }

    app *clause_proof::get_aux_repr(expr *e) {
        arith_util au(m);
        auto id = e->get_id();
        expr *id_expr[1] = {au.mk_int(id)};
        sort *s = e->get_sort();
        //! TODO: may need to manage more than one symbol when we have different sorts.
        return m.mk_app(symbol("aux"), 1, id_expr, s);
    }

    void clause_proof::declare_literal_with_on_clause(expr* e) {
        arith_util au(m);

        m_pp.collect(e);
        // m_pp.display_decls(out); // This is done by the callback.
        m.is_not(e, e);
        
        // -- The rest is adapted from define_expr
        bool should_define_lit = !m_pp.m_is_defined.is_marked(e);
        auto n = e;
        auto &m_is_defined = m_pp.m_is_defined;
        auto &m_defined = m_pp.m_defined;

        ptr_buffer<expr> visit;
        visit.push_back(n);
        while (!visit.empty()) {
            n = visit.back();
            if (m_is_defined.is_marked(n)) {
                visit.pop_back();
                continue;
            }
            if (is_app(n)) {
                bool all_visit = true;
                for (auto* e : *to_app(n)) {
                    if (m_is_defined.is_marked(e))
                        continue;
                    all_visit = false;
                    visit.push_back(e);
                }
                if (!all_visit)
                    continue;
                m_defined.push_back(n);
                m_is_defined.mark(n, true);
                visit.pop_back();
                if (to_app(n)->get_num_args() > 0) {
                    // ! here, int sexprs are used to represent z3 expression ids.
                    expr_ref_vector v(m);
                    v.push_back(get_aux_repr(n));

                    expr_ref_vector args(m);
                    for (auto* e : *to_app(n)) {
                        // Mimicking display_expr_def
                        expr *child_repr;

                        if (is_app(e) && to_app(e)->get_num_args() == 0)
                            child_repr = e;
                        else {
                            child_repr = get_aux_repr(e);
                        }

                        args.push_back(child_repr);
                    }
                    app *flat_app = m.mk_app(to_app(n)->get_decl(), args);
                    v.push_back(flat_app);

                    m_on_clause_eh(m_on_clause_ctx, m.mk_app(symbol("define-let"), 0, 0, m.mk_proof_sort()), 0, nullptr, v.size(), v.data());
                }
                continue;
            }

            expr_ref_vector v(m);
            v.push_back(get_aux_repr(n));
            v.push_back(n);
            m_on_clause_eh(m_on_clause_ctx, m.mk_app(symbol("define-let"), 0, 0, m.mk_proof_sort()), 0, nullptr, v.size(), v.data());  

            m_defined.push_back(n);
            m_is_defined.mark(n, true);
            visit.pop_back();        
        }

        if (should_define_lit) {
            literal l =  ctx.get_literal(e);
            int lit_int = l.sign() ? -l.var() : l.var();
            expr_ref_vector v(m);
            v.push_back(au.mk_int(lit_int));
            v.push_back(get_aux_repr(e));
            m_on_clause_eh(m_on_clause_ctx, m.mk_app(symbol("define-literal"), 0, 0, m.mk_proof_sort()), 0, nullptr, v.size(), v.data());
        }
    }

    void clause_proof::update(status st, expr_ref_vector& v, proof* p) {
        TRACE(clause_proof, tout << m_trail.size() << " " << st << " " << v << "\n";);
        // SASSERT(false);
        if (ctx.get_fparams().m_clause_proof)
            m_trail.push_back(info(st, v, p));
        if (m_on_clause_eh) {
            expr_ref_vector lits(m);
            arith_util au(m);

            for (expr* e : v) {
                literal l =  ctx.get_literal(e);
                int lit_int = l.sign() ? -l.var() : l.var();

                // ignore the false literal
                if (m.is_false(e))
                    continue;
                // handle the true literal
                if (m.is_true(e))
                    return; //skip the clause

                // literal 0 represents both true and false (see ctx.get_literal)
                SASSERT(l.var() != 0);

                lits.push_back(au.mk_int(lit_int));
            }

            // define expressions
            for (expr* e : v) {
                // ignore the false literal
                if (m.is_false(e))
                    continue;
                declare_literal_with_on_clause(e);
            }

            m_on_clause_eh(m_on_clause_ctx, p, 0, nullptr, lits.size(), lits.data());

            bool log_exprs = true;
            if (log_exprs) {
                proof *hint = m.mk_const("debug", m.mk_proof_sort());
                m_on_clause_eh(m_on_clause_ctx, hint, 0, nullptr, v.size(), v.data());
            }
        }
        
        if (m_has_log) {
            init_pp_out();
            auto& out = *m_pp_out;
            for (auto* e : v)
                declare(out, e);
            switch (st) {
            case clause_proof::status::assumption:
                if (!p || p->get_decl()->get_name() == "assumption") {
                    display_literals(out << "(assume", v) << ")\n";
                    break;
                }
                Z3_fallthrough;
            case clause_proof::status::th_conflict:
            case clause_proof::status::lemma:
            case clause_proof::status::th_lemma:
            case clause_proof::status::th_assumption:
                if (p)
                    declare(out, p);
                display_hint(display_literals(out << "(infer", v), p) << ")\n";
                break;
            case clause_proof::status::deleted:
                display_literals(out << "(del", v) << ")\n";
                break;
            default:
                UNREACHABLE();
            }
            out.flush();
        }
    }

    void clause_proof::update(clause& c, status st, proof* p, literal_buffer const* simp_lits) {
        if (!is_enabled())
            return;
        m_lits.reset();
        for (literal lit : c) 
            m_lits.push_back(ctx.literal2expr(lit));
        if (simp_lits) 
            for (auto lit : *simp_lits)
                m_lits.push_back(ctx.literal2expr(~lit));
        update(st, m_lits, p);        
    }

    proof_ref clause_proof::get_proof(bool inconsistent) {
        TRACE(context, tout << "get-proof " << ctx.get_fparams().m_clause_proof << "\n";);
        if (!ctx.get_fparams().m_clause_proof) 
            return proof_ref(m);
        expr_ref_vector ps(m);
        for (auto& info : m_trail) {
            expr_ref fact = mk_or(info.m_clause);
            proof* pr = info.m_proof;
            expr* args[2] = { pr, fact };
            unsigned num_args = 2, offset = 0;
            if (!pr) 
                offset = 1;
            switch (info.m_status) {
            case status::assumption:
                ps.push_back(m.mk_app(symbol("assumption"), num_args - offset, args + offset, m.mk_proof_sort()));
                break;
            case status::lemma:
                ps.push_back(m.mk_app(symbol("lemma"), num_args - offset, args + offset, m.mk_proof_sort()));
                break;
            case status::th_assumption:
                ps.push_back(m.mk_app(symbol("th-assumption"), num_args - offset, args + offset, m.mk_proof_sort()));
                break;
            case status::th_lemma:
                ps.push_back(m.mk_app(symbol("th-lemma"), num_args - offset, args + offset, m.mk_proof_sort()));
                break;
            case status::deleted:
                ps.push_back(m.mk_redundant_del(fact));
                break;
            }
        }
        if (inconsistent) 
            ps.push_back(m.mk_false());
        else 
            ps.push_back(m.mk_const("clause-trail-end", m.mk_bool_sort()));
        return proof_ref(m.mk_clause_trail(ps.size(), ps.data()), m);
    }

    std::ostream& operator<<(std::ostream& out, clause_proof::status st) {
        switch (st) {
        case clause_proof::status::assumption:
            return out << "asm";
        case clause_proof::status::th_assumption:
            return out << "th_asm";
        case clause_proof::status::lemma:
            return out << "lem";
        case clause_proof::status::th_lemma:
            return out << "th_lem";
        case clause_proof::status::deleted:
            return out << "del";
        default:
            return out << "unkn";
        }
    }

};



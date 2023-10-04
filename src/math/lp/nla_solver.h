/*++
Copyright (c) 2017 Microsoft Corporation

Author:
    Lev Nachmanson (levnach)
    Nikolaj Bjorner (nbjorner)

--*/
#pragma once
#include "util/vector.h"
#include "math/lp/lp_settings.h"
#include "util/rlimit.h"
#include "util/params.h"
#include "math/lp/lar_solver.h"
#include "math/lp/monic.h"
#include "math/lp/nla_core.h"
namespace nra {
    class solver;
}
namespace nla {
    class core;
    // nonlinear integer incremental linear solver
    class solver {
        core* m_core;
    public:

        solver(lp::lar_solver& s, params_ref const& p, reslimit& limit);
        ~solver();
        const auto& monics_with_changed_bounds() const { return m_core->monics_with_changed_bounds(); }
        void add_monic(lpvar v, unsigned sz, lpvar const* vs);
        void add_idivision(lpvar q, lpvar x, lpvar y);
        void add_rdivision(lpvar q, lpvar x, lpvar y);
        void add_bounded_division(lpvar q, lpvar x, lpvar y);
        void check_bounded_divisions();
        void set_relevant(std::function<bool(lpvar)>& is_relevant);
        void push();
        void pop(unsigned scopes);
        bool need_check();
        lbool check();
        void propagate();
        lbool check_power(lpvar r, lpvar x, lpvar y);
        bool is_monic_var(lpvar) const;
        bool influences_nl_var(lpvar) const;
        std::ostream& display(std::ostream& out) const;
        bool use_nra_model() const;
        core& get_core();
        nlsat::anum_manager& am();
        nlsat::anum const& am_value(lp::var_index v) const;
        void collect_statistics(::statistics & st);
        vector<nla::lemma> const& lemmas() const;
        vector<nla::ineq> const& literals() const;
        vector<nla::fixed_equality> const& fixed_equalities() const;
        vector<nla::equality> const& equalities() const;
    };
}

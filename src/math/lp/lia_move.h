/*++
Copyright (c) 2017 Microsoft Corporation

Module Name:

    <name>

Abstract:

    <abstract>

Author:
    Nikolaj Bjorner (nbjorner)
    Lev Nachmanson (levnach)

Revision History:


--*/
#pragma once
#include <string>
#include "util/debug.h"
namespace lp {
    enum class lia_move {
        sat,
        branch,
        cut,
        conflict,
        continue_with_check,
        undef,
        unsat,
        cancelled
    };
    inline std::string lia_move_to_string(lia_move m) {
        switch (m) {
        case lia_move::sat:
            return "sat";
        case lia_move::branch:
            return "branch";
        case lia_move::cut:
            return "cut";
        case lia_move::conflict:
            return "conflict";
        case lia_move::continue_with_check:
            return "continue_with_check";
        case lia_move::undef:
            return "undef";
        case lia_move::unsat:
            return "unsat";
        default:
            UNREACHABLE();
        };
        return "strange";
    }

    inline std::ostream& operator<<(std::ostream& out, lia_move const& m) {
        return out << lia_move_to_string(m);
    }
}

/*++
Sundance instrumentation.

Set Z3_TRACE_COLUMNS to print, on stderr, each column the arithmetic solver
adds to or removes from its tableau, and each scope the SMT core pushes or
pops.
--*/
#pragma once

#include <cstdlib>

inline bool sundance_trace_enabled() {
    static bool const on = std::getenv("Z3_TRACE_COLUMNS") != nullptr;
    return on;
}

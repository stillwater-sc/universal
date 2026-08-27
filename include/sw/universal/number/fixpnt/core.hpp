#pragma once
// core.hpp: the fixpnt arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the fixpnt headers (#1334, Phase 2).
//
//     #include <universal/number/fixpnt/fixpnt.hpp>   // everything, as before
//     #include <universal/number/fixpnt/core.hpp>     // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// It does NOT exclude <string>. assign(const std::string&) and the parse() declaration
// in fixpnt_fwd.hpp are part of the type's arithmetic surface, exactly as integer's
// string constructor is, and <string> is inside the epic's derived line budget
// (<cstdint>+<type_traits>+<limits>+<cmath>+<concepts> is 24,977 lines; adding <string>
// reaches 42,091, against a 45,000 ceiling). <map> backs the character lookup in
// assign(); <cstdio> backs fprintf(stderr, ...), the Phase 0 idiom adopted specifically
// so diagnostics do not require <iostream>. The regex/sstream parse() DEFINITION lives
// in iostream.hpp.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/fixpnt/exceptions.hpp>
#include <universal/number/fixpnt/fixpnt_fwd.hpp>
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/traits/fixpnt_traits.hpp>
#include <universal/number/fixpnt/numeric_limits.hpp>

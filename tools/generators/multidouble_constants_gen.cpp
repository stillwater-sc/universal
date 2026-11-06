// multidouble_constants_gen.cpp: generate the constant tables of dd, qd, dd_cascade, td_cascade
// and qd_cascade with the number systems themselves (#1849)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each entry is the type's own parse of the constant's ~340-digit reference in
// include/sw/math/constants/reference_constants.hpp (tools/scripts/generate_reference_constants.py):
// the nearest expansion, c0 the double nearest the constant, c1 the double nearest what c0 leaves,
// and so on. The components print with 17 significant digits, which read back as the same doubles.
// The tables in
//
//     include/sw/universal/number/dd/math/constants/dd_constants.hpp
//     include/sw/universal/number/qd/math/constants/qd_constants.hpp
//     include/sw/universal/number/dd_cascade/math/constants/dd_cascade_constants.hpp
//     include/sw/universal/number/td_cascade/math/constants/td_cascade_constants.hpp
//     include/sw/universal/number/qd_cascade/math/constants/qd_cascade_constants.hpp
//
// hold these values. The regression tests api/constant_table.cpp of each type check that they do,
// that they are the nearest expansions (efloat at 1024 bits, independently of the type's parse),
// and that the type's own math functions compute them (multidouble_constants.hpp).
//
// Build and run from the repository root:
//
//     g++ -std=c++20 -O2 -I include/sw tools/generators/multidouble_constants_gen.cpp -o multidouble_constants_gen
//     ./multidouble_constants_gen
#include <cstdio>
#include <universal/verification/multidouble_constant_tables.hpp>

int main() {
	using namespace sw::universal;
	using namespace sw::universal::multidouble_verification;
	std::printf("// dd\n");
	PrintConstantTable("dd", dd_constant_table(), 2);
	std::printf("\n// qd\n");
	PrintConstantTable("qd", qd_constant_table(), 4);
	std::printf("\n// dd_cascade\n");
	PrintConstantTable("dd_cascade", dd_cascade_constant_table(), 2);
	std::printf("\n// td_cascade\n");
	PrintConstantTable("td_cascade", td_cascade_constant_table(), 3);
	std::printf("\n// qd_cascade\n");
	PrintConstantTable("qd_cascade", qd_cascade_constant_table(), 4);
	return 0;
}

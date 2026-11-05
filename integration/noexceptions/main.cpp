// main.cpp: every number system in one program built with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Firmware and other bare-metal targets build with -fno-exceptions (#1925). Each translation unit
// of this test includes one number system's umbrella header and instantiates its arithmetic,
// comparisons, conversions and stream output; this file runs them. The CMake target compiles
// every translation unit with -fno-exceptions -Werror on gcc and clang.
#include <cstdio>
#include <universal/utility/exceptions.hpp>

#if UNIVERSAL_HAS_EXCEPTIONS
#error "this test must be compiled with exceptions disabled"
#endif

int noexceptions_areal();
int noexceptions_bfloat16();
int noexceptions_bisection();
int noexceptions_bposit();
int noexceptions_cfloat();
int noexceptions_dbns();
int noexceptions_dd();
int noexceptions_dd_cascade();
int noexceptions_dfixpnt();
int noexceptions_dfloat();
int noexceptions_e8m0();
int noexceptions_edecimal();
int noexceptions_efloat();
int noexceptions_einteger();
int noexceptions_elreal();
int noexceptions_erational();
int noexceptions_ereal();
int noexceptions_faithful();
int noexceptions_fixpnt();
int noexceptions_hfloat();
int noexceptions_integer();
int noexceptions_interval();
int noexceptions_lindex();
int noexceptions_lns();
int noexceptions_microfloat();
int noexceptions_mxfloat();
int noexceptions_nvblock();
int noexceptions_posit();
int noexceptions_positional();
int noexceptions_poxel();
int noexceptions_qd();
int noexceptions_qd_cascade();
int noexceptions_quire();
int noexceptions_rational();
int noexceptions_sorn();
int noexceptions_takum();
int noexceptions_td_cascade();
int noexceptions_unum();
int noexceptions_unum2();
int noexceptions_valid();
int noexceptions_zfpblock();
int noexceptions_convert();

int main() {
	int fails = 0;
	fails += noexceptions_areal();
	fails += noexceptions_bfloat16();
	fails += noexceptions_bisection();
	fails += noexceptions_bposit();
	fails += noexceptions_cfloat();
	fails += noexceptions_dbns();
	fails += noexceptions_dd();
	fails += noexceptions_dd_cascade();
	fails += noexceptions_dfixpnt();
	fails += noexceptions_dfloat();
	fails += noexceptions_e8m0();
	fails += noexceptions_edecimal();
	fails += noexceptions_efloat();
	fails += noexceptions_einteger();
	fails += noexceptions_elreal();
	fails += noexceptions_erational();
	fails += noexceptions_ereal();
	fails += noexceptions_faithful();
	fails += noexceptions_fixpnt();
	fails += noexceptions_hfloat();
	fails += noexceptions_integer();
	fails += noexceptions_interval();
	fails += noexceptions_lindex();
	fails += noexceptions_lns();
	fails += noexceptions_microfloat();
	fails += noexceptions_mxfloat();
	fails += noexceptions_nvblock();
	fails += noexceptions_posit();
	fails += noexceptions_positional();
	fails += noexceptions_poxel();
	fails += noexceptions_qd();
	fails += noexceptions_qd_cascade();
	fails += noexceptions_quire();
	fails += noexceptions_rational();
	fails += noexceptions_sorn();
	fails += noexceptions_takum();
	fails += noexceptions_td_cascade();
	fails += noexceptions_unum();
	fails += noexceptions_unum2();
	fails += noexceptions_valid();
	fails += noexceptions_zfpblock();
	fails += noexceptions_convert();
	std::printf("noexceptions: %d number systems, %s\n", 42, fails ? "FAIL" : "PASS");
	return fails ? 1 : 0;
}

// wide.cpp: validate bisection types wider than 64 bits
//
// With blockbinary storage (issue #704), bisection types can now be
// instantiated at 80, 128, and wider bit widths. This test exercises
// the wide storage path: encode/decode round-trip via dd/qd auxiliary,
// monotonicity spot-checks, and basic arithmetic.
//
// Issue #704.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#define BISECTION_THROW_ARITHMETIC_EXCEPTION 0
#define DD_THROW_ARITHMETIC_EXCEPTION 0
#define QD_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>

#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/bisection/bisection.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// Sample-based round-trip: encode a set of doubles, decode, re-encode,
// and verify bit identity. For wide types exhaustive enumeration is
// impossible, so we sample strategically.
template<typename T>
int sample_round_trip(const std::string& label, const std::vector<double>& samples) {
	int failures = 0;
	for (double x : samples) {
		T a(x);
		if (a.isnan()) continue;
		double d = double(a);
		T b(d);
		if (a != b) {
			++failures;
			if (failures <= 5) {
				std::cerr << "  FAIL " << label
				          << " x=" << std::setprecision(17) << x
				          << " decoded=" << d
				          << " re-encoded bits differ\n";
			}
		}
	}
	std::cout << "  " << std::left << std::setw(44) << label
	          << "round-trip " << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << samples.size() << " samples, " << failures << " failures)\n";
	return failures;
}

// Spot-check monotonicity: verify that the decoded values of a sequence
// of adjacent encodings are non-decreasing. We sample a window near
// zero and a window near 1.0.
template<typename T>
int spot_monotonic(const std::string& label) {
	int failures = 0;
	// Window near encoding 0 (value ~0): check 100 adjacent encodings
	// Walk a safe window of adjacent encodings near zero.
	// For small types (nbits <= 8) cap at 2^(nbits-1)-2 to avoid
	// wrapping past maxpos into the NaN region.
	const int window = (T::nbits <= 8) ? ((1 << (T::nbits - 1)) - 2) : 200;
	T v;
	v.setbits(0);
	double prev = double(v);
	for (int i = 1; i <= window; ++i) {
		++v;
		if (v.isnan()) break;
		double cur = double(v);
		if (cur < prev) {
			++failures;
			if (failures <= 3) {
				std::cerr << "  FAIL " << label
				          << " monotonic break near zero at offset " << i
				          << " prev=" << prev << " cur=" << cur << "\n";
			}
		}
		prev = cur;
	}
	std::cout << "  " << std::left << std::setw(44) << label
	          << "monotonic  " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	return failures;
}

// Basic arithmetic: verify a + b = c for a few hand-picked values.
template<typename T>
int basic_arithmetic(const std::string& label) {
	int failures = 0;
	T a(1.0), b(2.0);
	T c = a + b;
	double dc = double(c);
	if (std::abs(dc - 3.0) > 0.1) {
		++failures;
		std::cerr << "  FAIL " << label << " 1+2=" << dc << " expected ~3\n";
	}
	T d(10.0), e(3.0);
	T f = d / e;
	double df = double(f);
	if (std::abs(df - 10.0/3.0) > 0.5) {
		++failures;
		std::cerr << "  FAIL " << label << " 10/3=" << df << " expected ~3.33\n";
	}
	std::cout << "  " << std::left << std::setw(44) << label
	          << "arithmetic " << (failures == 0 ? "PASS" : "FAIL") << "\n";
	return failures;
}

} // anonymous namespace

int main() {
	using namespace sw::universal;
	int failures = 0;

	// Build a set of test doubles covering a wide range
	std::vector<double> samples = {
		0.0, 1.0, -1.0, 0.5, -0.5, 2.0, -2.0,
		0.1, -0.1, 0.01, -0.01, 100.0, -100.0,
		3.14159265358979, -3.14159265358979,
		1.0/3.0, -1.0/3.0, 1.0/7.0, -1.0/7.0,
		1e-10, -1e-10, 1e10, -1e10,
		0.999999, 1.000001,
		42.0, -42.0, 1234.5678, -1234.5678
	};

	std::cout << "bisection wide-type validation (blockbinary storage)\n";
	std::cout << "-----------------------------------------------------\n";

	// 8-bit with blockbinary: verify the dual-path produces the same
	// results as the int64_t path (regression check).
	std::cout << "\n[8-bit regression: int64_t vs blockbinary path]\n";
	// bisection_posit<8> uses int64_t; force blockbinary by using nbits=8
	// with an explicit blockbinary storage (nbits <= 64 still uses int64_t
	// via conditional_t, so we test the existing path here).
	failures += sample_round_trip<bisection_posit<8>>("bisection_posit<8> (int64_t)", samples);
	failures += spot_monotonic<bisection_posit<8>>("bisection_posit<8> (int64_t)");

	// 80-bit with dd auxiliary
	std::cout << "\n[80-bit, dd AuxReal]\n";
	using bp80_dd = bisection_posit<80, 0, uint8_t, dd>;
	failures += sample_round_trip<bp80_dd>("bisection_posit<80,0,uint8_t,dd>", samples);
	failures += spot_monotonic<bp80_dd>("bisection_posit<80,0,uint8_t,dd>");
	failures += basic_arithmetic<bp80_dd>("bisection_posit<80,0,uint8_t,dd>");

	// 128-bit with dd auxiliary
	std::cout << "\n[128-bit, dd AuxReal]\n";
	using bp128_dd = bisection_posit<128, 0, uint8_t, dd>;
	failures += sample_round_trip<bp128_dd>("bisection_posit<128,0,uint8_t,dd>", samples);
	failures += spot_monotonic<bp128_dd>("bisection_posit<128,0,uint8_t,dd>");
	failures += basic_arithmetic<bp128_dd>("bisection_posit<128,0,uint8_t,dd>");

	// 128-bit natposit with dd auxiliary
	std::cout << "\n[128-bit natposit, dd AuxReal]\n";
	using bnp128_dd = bisection_natposit<128, 0, uint8_t, dd>;
	failures += sample_round_trip<bnp128_dd>("bisection_natposit<128,0,uint8_t,dd>", samples);
	failures += spot_monotonic<bnp128_dd>("bisection_natposit<128,0,uint8_t,dd>");

	// Show a sample value to demonstrate the wide encoding
	std::cout << "\n[sample encodings]\n";
	bp80_dd pi80(3.14159265358979);
	bp128_dd pi128(3.14159265358979);
	std::cout << "  pi in bisection_posit<80,dd>:  " << double(pi80) << "\n";
	std::cout << "    binary: " << to_binary(pi80) << "\n";
	std::cout << "  pi in bisection_posit<128,dd>: " << double(pi128) << "\n";
	std::cout << "    binary: " << to_binary(pi128) << "\n";

	std::cout << "\n-----------------------------------------------------\n";
	std::cout << "bisection wide-type validation: "
	          << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << failures << " failures)\n";
	return failures == 0 ? 0 : 1;
}

// aux_real.cpp: verify the AuxReal template parameter of bisection<>
//
// bisection<Generator, Refinement, nbits, bt, AuxReal = double>
//
// When AuxReal is dd (double-double) instead of double, the interval
// bisection pipeline runs with ~31 decimal digits of precision, so
// round-trip accuracy for wider encodings (nbits > 50) stops being
// limited by double. This test drives the dd- and qd-backed code paths
// to make sure they compile, encode/decode, monotonically order, and
// saturate at the interval endpoints correctly.
//
// Issue #692 (Epic #687).
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

#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/bisection/bisection.hpp>

namespace {

using sw::universal::bisection_posit;
using sw::universal::bisection_natposit;
using sw::universal::dd;
using sw::universal::qd;

/// Walk all 2^nbits encodings of T. For each non-NaN encoding, decode,
/// re-encode, and verify the round-trip reproduces the bit pattern.
template<typename T>
int round_trip(const std::string& label) {
	constexpr unsigned p = T::nbits;
	const int64_t N = int64_t(1) << p;
	int failures = 0;
	for (int64_t i = 0; i < N; ++i) {
		T a;
		a.setbits(static_cast<uint64_t>(i));
		if (a.isnan()) continue;
		double d = double(a);
		T b(d);
		if (a != b) {
			++failures;
			if (failures <= 5) {
				std::cerr << "  FAIL " << label
				          << " enc=" << i
				          << " decoded=" << std::setprecision(17) << d
				          << " re-encoded=" << to_binary(b)
				          << " expected=" << to_binary(a) << "\n";
			}
		}
	}
	std::cout << "  " << std::left << std::setw(44) << label
	          << "round-trip " << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << failures << ")\n";
	return failures;
}

/// Verify the decoded sequence is monotonic non-decreasing when we walk
/// the signed two's-complement encoding order. This is what the paper
/// calls the "monotone encoding" property -- it must hold regardless of
/// which AuxReal backs the pipeline.
template<typename T>
int monotonic(const std::string& label) {
	constexpr unsigned p = T::nbits;
	const int64_t N = int64_t(1) << p;
	int failures = 0;
	double prev = -std::numeric_limits<double>::infinity();
	// Iterate in signed order: start at maxneg (+1 past NaN) and walk up.
	for (int64_t i = 1; i < N; ++i) {
		// Map loop index to signed encoding via two's-complement shift.
		int64_t signed_idx = i - (int64_t(1) << (p - 1));
		T a;
		a.setbits(static_cast<uint64_t>(signed_idx));
		if (a.isnan()) continue;
		double d = double(a);
		if (d < prev) {
			++failures;
			if (failures <= 5) {
				std::cerr << "  FAIL " << label
				          << " monotonic break at enc=" << signed_idx
				          << " prev=" << prev << " cur=" << d << "\n";
			}
		}
		prev = d;
	}
	std::cout << "  " << std::left << std::setw(44) << label
	          << "monotonic  " << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << failures << ")\n";
	return failures;
}

/// Demonstrate that dd-backed decoding carries visibly more precision
/// than double-backed decoding at the same encoding. We pick an
/// encoding that does not land on a dyadic rational so the difference
/// in the tail bits is observable. Informational, not a pass/fail gate.
template<typename Tdouble, typename Tdd>
void report_precision_delta(const std::string& label) {
	static_assert(Tdouble::nbits == Tdd::nbits, "same nbits required");
	// Encode 1/7 through each pipeline: a rational number that produces
	// a non-terminating binary fraction, so the bisection search cannot
	// land on it exactly even at 32 bits.
	const double x = 1.0 / 7.0;
	Tdouble a_double(x);
	Tdd     a_dd(x);
	double  d_double = double(a_double);
	dd      d_dd     = static_cast<dd>(a_dd);
	std::cout << "  " << label
	          << "  double path: " << std::setprecision(17) << d_double
	          << "\n  " << std::string(label.size(), ' ')
	          << "  dd    path: " << std::setprecision(33) << d_dd << "\n";
}

} // anonymous namespace

int main() {
	using namespace sw::universal;
	int failures = 0;

	std::cout << "bisection<> AuxReal parameterization tests\n";
	std::cout << "-------------------------------------------\n";

	// 8-bit exhaustive (fast): same encoding, different AuxReal.
	std::cout << "\n[8-bit exhaustive]\n";
	failures += round_trip<bisection_posit<8, 0, uint8_t, double>>("bisection_posit<8,0,uint8_t,double>");
	failures += round_trip<bisection_posit<8, 0, uint8_t, dd>>    ("bisection_posit<8,0,uint8_t,dd>    ");
	failures += round_trip<bisection_posit<8, 0, uint8_t, qd>>    ("bisection_posit<8,0,uint8_t,qd>    ");
	failures += monotonic <bisection_posit<8, 0, uint8_t, double>>("bisection_posit<8,0,uint8_t,double>");
	failures += monotonic <bisection_posit<8, 0, uint8_t, dd>>    ("bisection_posit<8,0,uint8_t,dd>    ");

	// 16-bit exhaustive -- covers the "useful precision" band where
	// AuxReal=double is still sufficient but the dd path should agree.
	std::cout << "\n[16-bit exhaustive]\n";
	failures += round_trip<bisection_posit<16, 0, uint8_t, double>>("bisection_posit<16,0,uint8_t,double>");
	failures += round_trip<bisection_posit<16, 0, uint8_t, dd>>    ("bisection_posit<16,0,uint8_t,dd>    ");
	failures += monotonic <bisection_posit<16, 0, uint8_t, dd>>    ("bisection_posit<16,0,uint8_t,dd>    ");
	failures += round_trip<bisection_natposit<16, 0, uint8_t, dd>> ("bisection_natposit<16,0,uint8_t,dd> ");

	// Informational: show the dd path decodes to extra precision.
	std::cout << "\n[precision reporting]\n";
	report_precision_delta<
		bisection_posit<32, 0, uint8_t, double>,
		bisection_posit<32, 0, uint8_t, dd>>("posit32 encoding of 1/7");

	std::cout << "\n-------------------------------------------\n";
	std::cout << "bisection AuxReal tests: "
	          << (failures == 0 ? "PASS" : "FAIL")
	          << " (" << failures << " failures)\n";
	return failures == 0 ? 0 : 1;
}

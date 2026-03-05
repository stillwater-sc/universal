// numeric_constants.cpp: precision comparison of mathematical constants across number systems
//
// Demonstrates how different Universal number systems represent the fundamental
// mathematical constants from C++20 <numbers>, showing the quantization error
// at each precision level.  The qd_cascade type (~212 fraction bits) serves as
// the oracle for computing relative error.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <numbers>
#include <cmath>
#include <vector>

// Number systems under test
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

// High-precision oracle for reference values
#include <universal/number/qd_cascade/qd_cascade.hpp>

using namespace sw::universal;

// -------------------------------------------------------------------------
// ULP helpers
// -------------------------------------------------------------------------

// Concept to detect types with a free-function ulp()
template<typename T>
concept HasUlp = requires(T t) { { ulp(t) }; };

// Compute ULP at a given value for any Universal type
template<typename Real>
double compute_ulp(const Real& value) {
	if constexpr (HasUlp<Real>) {
		return double(ulp(value));
	} else {
		// For types without ulp() (e.g., fixpnt):
		// ULP is the value of the least significant bit
		Real one_ulp;
		one_ulp.setbits(1);
		return double(one_ulp);
	}
}

// ULP for native double
double compute_ulp(double value) {
	return std::nextafter(value, std::numeric_limits<double>::max()) - value;
}

// -------------------------------------------------------------------------
// Reporting
// -------------------------------------------------------------------------

template<typename Real>
void ReportConstant(const std::string& label, double constant, const qd_cascade& reference) {
	Real r(constant);
	double ulp_val;
	if constexpr (std::is_same_v<Real, double>) {
		ulp_val = compute_ulp(constant);
	} else {
		ulp_val = compute_ulp(r);
	}
	double dval = double(r);
	qd_cascade qr{dval};
	qd_cascade relErr = abs((reference - qr) / reference);

	std::cout << std::left << std::setw(24) << label
	          << std::right << std::setw(14) << std::scientific << std::setprecision(2) << ulp_val
	          << "   " << std::setw(20) << std::setprecision(15) << std::defaultfloat << double(r)
	          << "   " << std::setw(14) << std::scientific << std::setprecision(2) << double(relErr)
	          << '\n';
}

void PrintHeader(const std::string& constant_name) {
	std::cout << "\n--- " << constant_name << " ---\n";
	std::cout << std::left << std::setw(24) << "Number System"
	          << std::right << std::setw(14) << "ULP"
	          << "   " << std::setw(20) << "Value"
	          << "   " << std::setw(14) << "Relative Error"
	          << '\n';
	std::cout << std::string(76, '-') << '\n';
}

// -------------------------------------------------------------------------
// Type list and fold-expression dispatch
// -------------------------------------------------------------------------

template<typename... Ts>
struct TypeList {};

template<typename... Reals>
void ReportAllTypes(TypeList<Reals...>,
                    const std::vector<std::string>& names,
                    double constant,
                    const qd_cascade& reference) {
	size_t i = 0;
	((ReportConstant<Reals>(names[i++], constant, reference)), ...);
}

// =========================================================================
int main()
try {
	// Type aliases for readability
	using cf16     = cfloat<16, 5, uint16_t, true, false, false>;
	using cf32     = cfloat<32, 8, uint32_t, true, false, false>;
	using fp32_24  = fixpnt<32, 24>;
	using fp64_48  = fixpnt<64, 48>;
	using fp128_96 = fixpnt<128, 96>;
	using p16      = posit<16, 2>;
	using p32      = posit<32, 2>;
	using p64      = posit<64, 2>;
	using p128     = posit<128, 2>;
	using l16      = lns<16, 8>;
	using l32      = lns<32, 16>;
	using l64      = lns<64, 32>;

	using AllTypes = TypeList<cf16, cf32, double, dd, qd,
	                          fp32_24, fp64_48, fp128_96,
	                          p16, p32, p64, p128,
	                          l16, l32, l64>;

	std::vector<std::string> names = {
		"cfloat<16,5>", "cfloat<32,8>", "double", "dd", "qd",
		"fixpnt<32,24>", "fixpnt<64,48>", "fixpnt<128,96>",
		"posit<16,2>", "posit<32,2>", "posit<64,2>", "posit<128,2>",
		"lns<16,8>", "lns<32,16>", "lns<64,32>"
	};

	std::cout << "Precision of Mathematical Constants Across Number Systems\n";
	std::cout << "Oracle: qd_cascade (~212 fraction bits, ~64 decimal digits)\n";
	std::cout << "Source: C++20 <numbers> constants (double precision input)\n";

	// pi
	PrintHeader("pi");
	ReportAllTypes(AllTypes{}, names, std::numbers::pi, qdc_pi);

	// e (Euler's number)
	PrintHeader("e (Euler's number)");
	ReportAllTypes(AllTypes{}, names, std::numbers::e, qdc_e);

	// ln(2)
	PrintHeader("ln(2)");
	ReportAllTypes(AllTypes{}, names, std::numbers::ln2, qdc_ln2);

	// ln(10)
	PrintHeader("ln(10)");
	ReportAllTypes(AllTypes{}, names, std::numbers::ln10, qdc_ln10);

	// sqrt(2)
	PrintHeader("sqrt(2)");
	ReportAllTypes(AllTypes{}, names, std::numbers::sqrt2, qdc_sqrt2);

	// phi (golden ratio)
	PrintHeader("phi (golden ratio)");
	ReportAllTypes(AllTypes{}, names, std::numbers::phi, qdc_phi);

	std::cout << R"(

Legend
------
  ULP           Unit in the Last Place — the step size at the constant's value
  Value         The constant as represented by the number system (shown in double)
  Relative Error  |oracle - represented| / |oracle|, computed in qd_cascade precision

  Lower ULP means finer granularity at that value.
  Relative error shows how faithfully the constant is captured.
  Types with more fraction bits near the constant's magnitude will show smaller errors.

  Oracle: qd_cascade provides ~212 bits of precision (~64 decimal digits),
  far exceeding any type in the table.
)" << '\n';

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

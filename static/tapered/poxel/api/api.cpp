// api.cpp: application programming interface of the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define POXEL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/poxel/poxel.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

int expect(bool ok, const char* what, bool report) {
	if (ok) return 0;
	if (report) std::cerr << "FAIL: " << what << '\n';
	return 1;
}

// constexpr encoder: poxel<9,2> is the posit<8,2> lattice plus a ubit
using X9 = sw::universal::poxel<9, 2, std::uint16_t>;
static_assert(X9::encode(false, 0, 0ull, 0u, false) == 0x080, "1.0 is the exact tile 2 * 0x40");
static_assert(X9::encode(false, 0, 1ull, 8u, false) == 0x081, "a hair above 1.0 is the open tile above it");
static_assert(X9::encode(true, 0, 0ull, 0u, false) == 0x180, "-1.0 is the two's complement of 0x080");
static_assert(X9::encode(false, 200, 0ull, 0u, false) == 0x0FF, "above maxpos: (maxpos, inf)");
static_assert(X9::encode(false, -200, 0ull, 0u, false) == 0x001, "below minpos: (0, minpos)");
static_assert(X9::encode(true, -200, 0ull, 0u, false) == 0x1FF, "above -minpos: (-minpos, 0)");
static_assert(X9::encode(true, 200, 0ull, 0u, false) == 0x101, "below -maxpos: (-inf, -maxpos)");

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "poxel API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	ReportTrivialityOfType<poxel9>();
	ReportTrivialityOfType<poxel17>();
	ReportTrivialityOfType<poxel33>();

	{
		int f = 0;
		f += expect(type_tag(poxel17{}) == "poxel< 17, 2, uint32_t>", "type_tag", reportTestCases);
		f += expect(is_poxel<poxel9> && !is_poxel<double>, "is_poxel trait", reportTestCases);
		f += expect(to_binary(X9(1.0)) == "0b0.10.00.000|0", "to_binary of 1.0", reportTestCases);
		f += expect(to_interval(X9(0.1)) == "(0.09375, 0.1015625)", "to_interval of 0.1", reportTestCases);
		// a 60-fraction-bit lattice: decimal where long double holds it (x86 80-bit), exact
		// hexfloat where it does not (long double == double); either way the two ends of
		// the tile above 1.0 must print differently
		using W = poxel<64, 0, std::uint64_t>;
		W w(1);
		++w;
		const std::string wide = to_interval(w);
		const bool hex = (wide == "(0x1.000000000000000p+0, 0x1.000000000000001p+0)");
		const bool dec = (wide == "(1, 1.0000000000000000009)");
		f += expect(hex || dec, "to_interval of a 60-fraction-bit tile keeps both lattice points apart", reportTestCases);
		if (!(hex || dec) && reportTestCases) std::cerr << "  got " << wide << '\n';
		f += expect(to_interval(X9(SpecificValue::infpos)) == "(16777216, inf)" && to_interval(X9(SpecificValue::infneg)) == "(-inf, -16777216)", "to_interval of the end tiles", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel", "type tag and text");
	}
	{
		int f = 0;
		const X9 a(3), b(3.0f), c(3.0), d(3.0l);
		f += expect(a == b && b == c && c == d && a.isexact(), "construction from int, float, double, long double", reportTestCases);
		f += expect(X9(SpecificValue::maxpos).upper<double>() == 16777216.0 && X9(SpecificValue::minpos).lower<double>() == 5.9604644775390625e-08, "maxpos and minpos of the posit<8,2> lattice", reportTestCases);
		f += expect(X9(SpecificValue::infpos).upper<double>() == INFINITY && X9(SpecificValue::infneg).lower<double>() == -INFINITY, "the end tiles", reportTestCases);
		f += expect(X9(SpecificValue::nar).isnar() && X9(SpecificValue::zero).iszero(), "nar and zero", reportTestCases);
		X9 one(1);
		++one;
		f += expect(!one.isexact() && one.lower<double>() == 1.0, "++ steps from 1.0 to the open tile above it", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel<9,2>", "construction and special tiles");
	}
	{
		int f = 0;
		using L = std::numeric_limits<poxel17>;
		f += expect(L::epsilon().isexact() && double(L::epsilon()) == std::ldexp(1.0, -11), "epsilon is the exact lattice gap above 1.0", reportTestCases);
		f += expect(!L::has_infinity && L::quiet_NaN().isnar() && L::round_style == std::round_toward_neg_infinity, "no infinity; values are stored as their tile's lower end", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel17", "numeric_limits");
	}
	{
		int f = 0;
		bool threw = false;
		try { poxel17 x = poxel17(1) / poxel17(0); (void)x; } catch (const poxel_divide_by_zero&) { threw = true; }
		f += expect(threw, "division by zero throws", reportTestCases);
		threw = false;
		try { poxel17 x = poxel17(1) / poxel17(SpecificValue::minpos).operator--(); (void)x; } catch (const poxel_divide_by_zero&) { threw = true; }
		f += expect(threw, "division by the (0, minpos) tile throws like division by zero", reportTestCases);
		threw = false;
		try { poxel17 x = poxel17(SpecificValue::nar) + poxel17(1); (void)x; } catch (const poxel_operand_is_nar&) { threw = true; }
		f += expect(threw, "a NaR operand throws", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel17", "arithmetic exceptions");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

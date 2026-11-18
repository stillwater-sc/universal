// ulp.cpp: testing ulp values and algebra for classic floating-point cfloat configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <vector>

// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
void TestULP(sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> v = 1.0f) {
	using namespace sw::universal;

	cfloat < nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> a(v), ulpAt(ulp(a));
	std::cout << type_tag(a) << " at " << std::setw(15) << a << " : " << to_binary(a) << " : ULP : " << to_binary(ulpAt) << " : " << ulpAt << '\n';
}

template<typename Real>
void TestNativeULP(Real v = 1.0f) {
	using namespace sw::universal;

	Real a(v), ulpAt(ulp(a));
	std::cout << type_tag(a) << " at " << std::setw(15) << a << " : " << to_binary(a) << " : ULP : " << to_binary(ulpAt) << " : " << ulpAt << '\n';
}

// cfloat ulp() against the native ulp() for the float and double layouts: the magnitude of the gap
// away from zero, at negative values, powers of two, subnormals and maxpos (#2107; it was ++a - a,
// which stepped toward zero for a negative a and into inf at maxpos)
template<typename Cfloat, typename Real>
int VerifyULPAgainstNative(bool reportTestCases) {
	using namespace sw::universal;
	using Lim = std::numeric_limits<Real>;
	std::vector<Real> samples = { Real(1), Real(1.5), Real(2), Real(3), Real(0.75), Real(1e10), Real(1e-10),
		Lim::min(), Lim::denorm_min(), Real(3) * Lim::denorm_min(), Lim::max(), std::nextafter(Lim::max(), Real(0)),
		Real(0) };
	for (int e = Lim::min_exponent - Lim::digits; e < Lim::max_exponent; e += 7) {
		samples.push_back(std::ldexp(Real(1), e));                     // powers of two, subnormal ones included
		samples.push_back(std::nextafter(std::ldexp(Real(1), e), Real(0)));
	}
	int fails = 0;
	for (Real v : samples) {
		for (Real x : { v, -v }) {
			const Real expected = sw::universal::ulp(x);
			const Real got = Real(ulp(Cfloat(x)));
			if (!(got == expected)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL: ulp(" << type_tag(Cfloat()) << "(" << x << ")) = " << got << ", native " << expected << '\n';
			}
		}
	}
	Cfloat inf; inf.setinf(true);
	if (!(ulp(inf).isinf() && !ulp(inf).sign())) { ++fails; if (reportTestCases) std::cerr << "FAIL: ulp(-inf) is not +inf\n"; }
	Cfloat nan; nan.setnan(NAN_TYPE_QUIET);
	if (!ulp(nan).isnan()) { ++fails; if (reportTestCases) std::cerr << "FAIL: ulp(NaN) is not NaN\n"; }
	return fails;
}

// every layout's ulp, exhaustively, against the sorted value set (#2133): the native comparison above
// only reaches cfloat<32,8> and <64,11>. The reference gap is computed in double from neighbouring
// values; where a layout without subnormals cannot hold it (a gap below min()), the expected ulp is
// that gap as the cfloat holds it. Layouts: with and without subnormals, max-exponent values and
// saturation, at 8 to 12 bits.
template<unsigned nbits, unsigned es, bool sub, bool maxexp, bool sat>
int VerifyULPExhaustive(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<nbits, es, std::uint8_t, sub, maxexp, sat>;
	std::vector<double> vals;
	for (unsigned u = 0; u < (1u << nbits); ++u) {
		C c;
		c.setbits(u);
		if (c.isnan() || c.isinf()) continue;
		const double d = double(c);
		if (d >= 0) vals.push_back(d);
	}
	std::sort(vals.begin(), vals.end());
	vals.erase(std::unique(vals.begin(), vals.end()), vals.end());
	int fails = 0;
	for (size_t i = 0; i < vals.size(); ++i) {
		const double x = vals[i];
		const double gap = (i + 1 < vals.size()) ? vals[i + 1] - x : x - vals[i - 1];   // maxpos: the gap below
		const C expected(gap);
		for (double sx : { x, -x }) {
			const C got = ulp(C(sx));
			if (!(double(got) == double(expected))) {
				++fails;
				if (reportTestCases && fails < 4) std::cerr << "FAIL: ulp(" << type_tag(C()) << "(" << sx << ")) = " << double(got) << ", expected " << double(expected) << '\n';
			}
		}
	}
	return fails;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "classic floating-point ULP tests";
	std::string test_tag    = "ulp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	constexpr bool hasSubnormals = true;
	constexpr bool noSupernormals = false;
	constexpr bool notSaturating = false;

	{
		TestULP<8, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating>();     // quarter precision
		TestULP<16, 5, uint16_t, hasSubnormals, noSupernormals, notSaturating>();   // half precision
		TestULP<32, 8, uint32_t, hasSubnormals, noSupernormals, notSaturating>();   // single precision
		TestULP<64, 11, uint32_t, hasSubnormals, noSupernormals, notSaturating>();   // double precision
		TestULP<128, 15, uint32_t, hasSubnormals, noSupernormals, notSaturating>();  // quad precision
		TestULP<256, 19, uint32_t, hasSubnormals, noSupernormals, notSaturating>();  // octo precision
	}

	{
		std::cout << "\nFP8 classic floating-point ULPs\n";
		constexpr size_t nbits = 8;
		constexpr size_t es = 2;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "FP8 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 0.1f; base < 4.0f; base *= 2.0f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}

	{
		std::cout << "\nhalf-precision FP16 classic floating-point ULPs\n";
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "FP16 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e4f; base *= 1.0e1f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}

	{
		std::cout << "\nBFLOAT16: Brain floating-point ULPs\n";
		constexpr size_t nbits = 16;
		constexpr size_t es = 8;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "bfloat16 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e10f; base *= 1.0e1f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}
	{
		std::cout << "\n32-bit classic floating-point ULPs as baseline\n";
		constexpr size_t nbits = 32;
		constexpr size_t es = 8;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "cfloat epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e30f; base *= 1.0e3f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}

	{
		std::cout << "\nNative IEEE-754 single precision float ULPs to reference\n";
		constexpr float eps = std::numeric_limits< float >::epsilon();
		std::cout << "float epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e30f; base *= 1.0e3f) {
			TestNativeULP(base);
		}
	}

	nrOfFailedTestCases += ReportTestResult(VerifyULPAgainstNative<cfloat<32, 8, uint32_t, hasSubnormals, noSupernormals, notSaturating>, float>(reportTestCases), "cfloat<32,8>", "ulp vs native");
	nrOfFailedTestCases += ReportTestResult(VerifyULPAgainstNative<cfloat<64, 11, uint32_t, hasSubnormals, noSupernormals, notSaturating>, double>(reportTestCases), "cfloat<64,11>", "ulp vs native");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive< 8, 3, true , false, false>(reportTestCases), "cfloat<8,3>", "ulp exhaustive, subnormals");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive< 8, 3, false, false, false>(reportTestCases), "cfloat<8,3>", "ulp exhaustive, no subnormals");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive< 8, 3, true , true , false>(reportTestCases), "cfloat<8,3>", "ulp exhaustive, max-exponent values");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive< 8, 3, false, true , true >(reportTestCases), "cfloat<8,3>", "ulp exhaustive, no subnormals, saturating");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive<10, 4, false, false, true >(reportTestCases), "cfloat<10,4>", "ulp exhaustive, saturating");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive<12, 4, false, false, false>(reportTestCases), "cfloat<12,4>", "ulp exhaustive, no subnormals");
	nrOfFailedTestCases += ReportTestResult(VerifyULPExhaustive< 9, 2, true , true , true >(reportTestCases), "cfloat<9,2>", "ulp exhaustive, every option");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

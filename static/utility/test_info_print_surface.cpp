// test_info_print_surface.cpp: info_print() reports a decode, for every type that offers one
//
// info_print(x, printPrecision) returned the literal string "TBD" -- "tbd" for ereal --
// and ignored both of its arguments in nine number systems: areal, cfloat, dbns, efloat,
// ereal, fixpnt, integer, lns and takum, plus the native IEEE-754 types, which the
// original survey missed because it only looked under number/ (#1556).
//
// It compiled, which is why nothing flagged it, and the areal manipulator surface suite
// from #1551 listed info_print and passed: its assertion was `!text.empty()`, and "TBD"
// is not empty.
//
// So the assertions here are the ones a placeholder cannot satisfy:
//
//   1. the text is not a placeholder -- no "TBD"/"tbd" anywhere in it;
//   2. it has the shape the implemented types established, "raw: ... : value ...";
//   3. the value field is populated;
//   4. DIFFERENT VALUES PRODUCE DIFFERENT TEXT. This is the one that matters. A stub
//      returns one constant for every input, so no per-type knowledge is needed to catch
//      it, and no future stub can slip through by being non-empty and well-shaped.
//   5. printPrecision reaches the value field, i.e. the second argument is not ignored.
//
// Not every number system has an info_print at all -- roughly two dozen (dd, qd, dfloat,
// hfloat, einteger, elreal, microfloat, ...) have none, so ReportFormats<Scalar> in
// verification/test_formats.hpp does not compile for them. That is a wider gap than
// #1556 describes and is reported separately; this suite covers the types that do offer
// one.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <limits>
#include <string>

#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	bool is_placeholder(const std::string& text) {
		return text.find("TBD") != std::string::npos || text.find("tbd") != std::string::npos;
	}

	int expect_true(bool actual, const std::string& what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// The whole contract, applied to one type's info_print through two distinct values.
	// a and b must differ; neither may be the type's NaN, which several types render
	// identically whatever the payload.
	template<typename Scalar>
	int VerifyInfoPrint(const char* name, const Scalar& a, const Scalar& b, bool reportTestCases) {
		int fails = 0;
		const std::string label(name);

		const std::string text = info_print(a);

		fails += expect_true(!text.empty(), label + ": info_print produces text", reportTestCases);
		fails += expect_true(!is_placeholder(text), label + ": info_print is not a placeholder", reportTestCases);

		// the shape the implemented types established: posit's "raw: <bits> <fields> : value <v>"
		fails += expect_true(text.rfind("raw: ", 0) == 0, label + ": info_print reports the raw encoding",
			reportTestCases);
		const std::size_t valueField = text.find(" : value ");
		fails += expect_true(valueField != std::string::npos, label + ": info_print reports a value",
			reportTestCases);
		if (valueField != std::string::npos) {
			const std::string value = text.substr(valueField + 9);
			fails += expect_true(!value.empty(), label + ": the value field is populated", reportTestCases);
			fails += expect_true(value.find_first_of("0123456789") != std::string::npos,
				label + ": the value field carries digits", reportTestCases);
		}

		// the argument is read: a stub returns one constant for every input
		fails += expect_true(info_print(a) != info_print(b),
			label + ": info_print distinguishes different values", reportTestCases);

		if (fails && reportTestCases) {
			std::cout << "         " << label << " -> " << text << '\n';
		}
		return fails;
	}

	// printPrecision is the second argument, and it must reach the value field.
	//
	// The length comparison alone is not enough of a check: it holds trivially when the two
	// renderings are IDENTICAL, which is exactly what an implementation that ignores
	// printPrecision produces. So the strings must actually differ -- every caller below
	// passes a value with more digits than 3 to spend -- and the length comparison stays as
	// the additional invariant that more digits never render shorter.
	template<typename Scalar>
	int VerifyPrintPrecisionIsRead(const char* name, const Scalar& v, bool reportTestCases) {
		const std::string label(name);
		const std::string terse   = info_print(v, 3);
		const std::string verbose = info_print(v, 17);
		int fails = expect_true(terse != verbose,
			label + ": printPrecision changes the rendering", reportTestCases);
		fails += expect_true(terse.size() <= verbose.size(),
			label + ": a wider printPrecision does not shorten the rendering", reportTestCases);
		if (fails && reportTestCases) {
			std::cout << "         " << label << " @3  -> " << terse << '\n';
			std::cout << "         " << label << " @17 -> " << verbose << '\n';
		}
		return fails;
	}

	int VerifyStaticTypes(bool reportTestCases) {
		int fails = 0;

		// posit is the reference implementation: if the contract below does not hold for
		// posit, the contract is wrong, not the type
		fails += VerifyInfoPrint("posit<16,1>", posit<16, 1>(1.5f), posit<16, 1>(2.5f), reportTestCases);

		fails += VerifyInfoPrint("areal<16,5>", areal<16, 5>(1.5f), areal<16, 5>(2.5f), reportTestCases);
		fails += VerifyInfoPrint("cfloat<16,5>",
			cfloat<16, 5, std::uint16_t, true, false, false>(1.5f),
			cfloat<16, 5, std::uint16_t, true, false, false>(2.5f), reportTestCases);
		fails += VerifyInfoPrint("dbns<8,3>",   dbns<8, 3>(1.5),   dbns<8, 3>(2.0),   reportTestCases);
		fails += VerifyInfoPrint("fixpnt<8,4>", fixpnt<8, 4>(1.5), fixpnt<8, 4>(2.5), reportTestCases);
		fails += VerifyInfoPrint("integer<16>", integer<16>(-10),  integer<16>(10),   reportTestCases);
		fails += VerifyInfoPrint("lns<8,3>",    lns<8, 3>(1.5),    lns<8, 3>(2.5),    reportTestCases);
		fails += VerifyInfoPrint("takum<16,5>", takum<16, 5>(1.5), takum<16, 5>(2.5), reportTestCases);
		// takum's info_print is constrained on is_any_takum, which covers the logarithmic
		// takum as well: it has to instantiate for both
		fails += VerifyInfoPrint("takum_log<16,5>", takum_log<16, 5>(1.5), takum_log<16, 5>(2.5),
			reportTestCases);

		return fails;
	}

	int VerifyElasticTypes(bool reportTestCases) {
		int fails = 0;

		// no fixed bit layout to decode, so these report the limb count and the scale
		fails += VerifyInfoPrint("efloat<4>", efloat<4>(1.5), efloat<4>(2.5), reportTestCases);
		fails += VerifyInfoPrint("ereal<8>",  ereal<8>(1.5),  ereal<8>(2.5),  reportTestCases);

		return fails;
	}

	int VerifyNativeTypes(bool reportTestCases) {
		int fails = 0;

		// the tenth stub: ReportFormats<Scalar> is generic over the native types too
		fails += VerifyInfoPrint("float",  1.5f, 2.5f, reportTestCases);
		fails += VerifyInfoPrint("double", 1.5,  2.5,  reportTestCases);

		// pretty_print(native) was the same stub, and has no "raw:" shape to check
		fails += expect_true(!is_placeholder(pretty_print(1.5)),
			"double: pretty_print is not a placeholder", reportTestCases);
		fails += expect_true(pretty_print(1.5) != pretty_print(2.5),
			"double: pretty_print distinguishes different values", reportTestCases);

		return fails;
	}

	int VerifyPrintPrecision(bool reportTestCases) {
		int fails = 0;

		// 1/3 has digits to spend at any precision, in every one of these
		fails += VerifyPrintPrecisionIsRead("posit<32,2>", posit<32, 2>(1.0) / posit<32, 2>(3.0), reportTestCases);
		fails += VerifyPrintPrecisionIsRead("cfloat<32,8>",
			cfloat<32, 8, std::uint32_t, true, false, false>(1.0f / 3.0f), reportTestCases);
		fails += VerifyPrintPrecisionIsRead("efloat<4>", efloat<4>(1.0) / efloat<4>(3.0), reportTestCases);
		fails += VerifyPrintPrecisionIsRead("ereal<8>",  ereal<8>(1.0)  / ereal<8>(3.0),  reportTestCases);
		fails += VerifyPrintPrecisionIsRead("double",    1.0 / 3.0,                       reportTestCases);

		return fails;
	}

	// The special encodings must render as themselves, not fall through to a numeric path
	// that reports a scale for a value that has none.
	int VerifySpecialEncodings(bool reportTestCases) {
		int fails = 0;

		using Cfloat = cfloat<16, 5, std::uint16_t, true, false, false>;
		fails += expect_true(info_print(Cfloat(0.0f)).find(" zero") != std::string::npos,
			"cfloat: zero reports zero", reportTestCases);
		fails += expect_true(
			info_print(Cfloat(std::numeric_limits<float>::quiet_NaN())).find(" nan") != std::string::npos,
			"cfloat: nan reports nan", reportTestCases);
		fails += expect_true(
			info_print(Cfloat(std::numeric_limits<float>::infinity())).find(" inf") != std::string::npos,
			"cfloat: inf reports inf", reportTestCases);

		fails += expect_true(info_print(0.0).find(" zero") != std::string::npos,
			"double: zero reports zero", reportTestCases);
		// std::numeric_limits, not 1.0/0.0: MSVC rejects a literal division by zero at
		// compile time outright (error C2124), where gcc and clang fold it to inf
		fails += expect_true(info_print(std::numeric_limits<double>::infinity()).find(" inf") != std::string::npos,
			"double: inf reports inf", reportTestCases);
		fails += expect_true(info_print(std::numeric_limits<double>::quiet_NaN()).find(" nan") != std::string::npos,
			"double: nan reports nan", reportTestCases);

		fails += expect_true(info_print(integer<16>(0)).find(" zero") != std::string::npos,
			"integer: zero reports zero", reportTestCases);
		fails += expect_true(info_print(efloat<4>(0.0)).find(" zero") != std::string::npos,
			"efloat: zero reports zero", reportTestCases);
		fails += expect_true(info_print(ereal<8>(0.0)).find(" zero") != std::string::npos,
			"ereal: zero reports zero", reportTestCases);

		return fails;
	}

	// components(efloat) called v.exponent(), which efloat does not have -- only scale() --
	// and nothing in the tree instantiated it, so the body was never compiled, the same
	// trap components(areal) fell into (#1453). It also read sign(), which returns int(-1)
	// or int(+1) and never 0, as a bool, so every value rendered negative. Repaired
	// alongside #1556; this pins it.
	int VerifyEfloatComponents(bool reportTestCases) {
		int fails = 0;

		fails += expect_true(components(efloat<4>(1.5)).rfind("(+, ", 0) == 0,
			"efloat: components reports a positive value as positive", reportTestCases);
		fails += expect_true(components(efloat<4>(-1.5)).rfind("(-, ", 0) == 0,
			"efloat: components reports a negative value as negative", reportTestCases);
		fails += expect_true(!is_placeholder(components(efloat<4>(1.5))),
			"efloat: components is not a placeholder", reportTestCases);
		fails += expect_true(components(efloat<4>(1.5)) != components(efloat<4>(2.5)),
			"efloat: components distinguishes different values", reportTestCases);

		return fails;
	}

}  // anonymous namespace

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "info_print surface across the number systems (#1556)";
	std::string test_tag    = "info_print";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// what each type reports, side by side
	std::cout << "posit<16,1>  : " << info_print(posit<16, 1>(1.5f)) << '\n';
	std::cout << "areal<16,5>  : " << info_print(areal<16, 5>(1.5f)) << '\n';
	std::cout << "cfloat<16,5> : " << info_print(cfloat<16, 5, std::uint16_t, true, false, false>(1.5f)) << '\n';
	std::cout << "dbns<8,3>    : " << info_print(dbns<8, 3>(1.5)) << '\n';
	std::cout << "fixpnt<8,4>  : " << info_print(fixpnt<8, 4>(1.5)) << '\n';
	std::cout << "integer<16>  : " << info_print(integer<16>(-10)) << '\n';
	std::cout << "lns<8,3>     : " << info_print(lns<8, 3>(1.5)) << '\n';
	std::cout << "takum<16,5>  : " << info_print(takum<16, 5>(1.5)) << '\n';
	std::cout << "efloat<4>    : " << info_print(efloat<4>(1.5)) << '\n';
	std::cout << "ereal<8>     : " << info_print(ereal<8>(1.5)) << '\n';
	std::cout << "double       : " << info_print(1.5) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyStaticTypes(reportTestCases), test_tag, "static types");
	nrOfFailedTestCases += ReportTestResult(VerifyElasticTypes(reportTestCases), test_tag, "elastic types");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeTypes(reportTestCases), test_tag, "native types");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialEncodings(reportTestCases), test_tag, "special encodings");
	nrOfFailedTestCases += ReportTestResult(VerifyPrintPrecision(reportTestCases), test_tag, "printPrecision");
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatComponents(reportTestCases), test_tag, "efloat components");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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

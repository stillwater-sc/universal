// test_report_formats.cpp: ReportFormats<Scalar> works for every number system
//
// ReportFormats called type_tag, type_field, to_binary, to_hex, to_triple, info_print,
// pretty_print and color_print unconditionally, so it only compiled for a type that
// provided all eight. Surveying include/sw/universal/number/, exactly five of the
// thirty-nine number systems do: cfloat, dbns, fixpnt, integer and lns. Even posit is
// missing type_field and to_hex, and type_field alone is absent from thirty of them.
//
// Nothing in the tree instantiated it, which is why a reporting helper that did not
// compile for 34 of the library's own types went unnoticed (#1582).
//
// Filling that surface would mean adding well over a hundred functions. Detecting each
// renderer instead makes ReportFormats work for every type today. This suite is the
// check: it instantiates ReportFormats over a broad spread of number systems --
// fixed-size, tapered, logarithmic, multi-component, elastic and native -- and every one
// of those instantiations is a compile-time assertion that the detection holds. Several
// of these types could not be passed to ReportFormats at all before.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_formats.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to print what every type reports instead of running the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const std::string& what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// Instantiating ReportFormats is the assertion: it either compiles for this type or it
	// does not. The output goes to a swallowed stream so the suite stays quiet.
	template<typename Scalar>
	int VerifyReportFormatsInstantiates(const char* name, const Scalar& v, bool reportTestCases) {
		std::streambuf* saved = std::cout.rdbuf();
		std::ostringstream swallow;
		std::cout.rdbuf(swallow.rdbuf());
		ReportFormats(v);
		std::cout.rdbuf(saved);

		int fails = expect_true(!swallow.str().empty(),
			std::string(name) + ": ReportFormats produces output", reportTestCases);
		// type_tag is the one renderer every number system has, so it is always reported
		fails += expect_true(swallow.str().find(type_tag(v)) != std::string::npos,
			std::string(name) + ": ReportFormats reports the type tag", reportTestCases);
		return fails;
	}

	int VerifyEveryTypeReports(bool reportTestCases) {
		int fails = 0;

		// the five that could already do this
		fails += VerifyReportFormatsInstantiates("cfloat<16,5>",
			cfloat<16, 5, std::uint16_t, true, false, false>(1.5f), reportTestCases);
		fails += VerifyReportFormatsInstantiates("dbns<8,3>",   dbns<8, 3>(1.5),   reportTestCases);
		fails += VerifyReportFormatsInstantiates("fixpnt<8,4>", fixpnt<8, 4>(1.5), reportTestCases);
		fails += VerifyReportFormatsInstantiates("integer<16>", integer<16>(-10),  reportTestCases);
		fails += VerifyReportFormatsInstantiates("lns<8,3>",    lns<8, 3>(1.5),    reportTestCases);

		// and the ones that could not: each of these is missing at least one renderer
		fails += VerifyReportFormatsInstantiates("posit<16,1>",  posit<16, 1>(1.5f), reportTestCases);
		fails += VerifyReportFormatsInstantiates("areal<16,5>",  areal<16, 5>(1.5f), reportTestCases);
		fails += VerifyReportFormatsInstantiates("takum<16,5>",  takum<16, 5>(1.5),  reportTestCases);
		fails += VerifyReportFormatsInstantiates("dd",           dd(1.5),            reportTestCases);
		fails += VerifyReportFormatsInstantiates("qd",           qd(1.5),            reportTestCases);
		fails += VerifyReportFormatsInstantiates("efloat<4>",    efloat<4>(1.5),     reportTestCases);
		fails += VerifyReportFormatsInstantiates("ereal<8>",     ereal<8>(1.5),      reportTestCases);

		// native types go through the same helper
		fails += VerifyReportFormatsInstantiates("float",  1.5f, reportTestCases);
		fails += VerifyReportFormatsInstantiates("double", 1.5,  reportTestCases);

		return fails;
	}

	// ReportFormatSurface names what a type offers. It is what makes the gap legible
	// rather than a build error.
	int VerifySurfaceReport(bool reportTestCases) {
		int fails = 0;

		// cfloat has the whole surface
		const std::string cf = ReportFormatSurface(cfloat<16, 5, std::uint16_t, true, false, false>(1.5f));
		for (const char* fn : { "type_tag", "type_field", "to_binary", "to_hex",
		                        "to_triple", "info_print", "pretty_print", "color_print" }) {
			fails += expect_true(cf.find(fn) != std::string::npos,
				std::string("cfloat offers ") + fn, reportTestCases);
		}

		// posit does not: it has no type_field, which is the whole point of detecting
		const std::string p = ReportFormatSurface(posit<16, 1>(1.5f));
		fails += expect_true(p.find("type_tag") != std::string::npos, "posit offers type_tag", reportTestCases);
		fails += expect_true(p.find("info_print") != std::string::npos, "posit offers info_print", reportTestCases);
		fails += expect_true(p.find("type_field") == std::string::npos,
			"posit does not offer type_field, and the surface says so", reportTestCases);

		// and the two reports differ, which is the gap made visible
		fails += expect_true(cf != p, "the surface distinguishes types", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "ReportFormats across the number systems (#1582)";
	std::string test_tag    = "ReportFormats";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	std::cout << "---- posit<16,1>\n";  ReportFormats(posit<16, 1>(1.5f));
	std::cout << "---- efloat<4>\n";    ReportFormats(efloat<4>(1.5));
	std::cout << "---- ereal<8>\n";     ReportFormats(ereal<8>(1.5));
	std::cout << "---- dd\n";           ReportFormats(dd(1.5));
	std::cout << "\nsurfaces:\n"
	          << "  cfloat : " << ReportFormatSurface(cfloat<16, 5, std::uint16_t, true, false, false>()) << '\n'
	          << "  posit  : " << ReportFormatSurface(posit<16, 1>()) << '\n'
	          << "  dd     : " << ReportFormatSurface(dd()) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEveryTypeReports(reportTestCases), test_tag, "every type reports");
	nrOfFailedTestCases += ReportTestResult(VerifySurfaceReport(reportTestCases), test_tag, "surface report");
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

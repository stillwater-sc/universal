// nar_and_classify.cpp: classification on the encoding, and NaR propagating through the posit math
// functions (#2071)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// User decision (#2071): NaR is the Posit Standard's single exception value, and it propagates.
// isnan(NaR) is true and isinf(NaR) false; min, max, nextafter, nexttoward and copysign of anything
// with NaR are NaR. Before, isinf and isnan of NaR were both true, max(NaR, x) was x while min(NaR, x)
// was NaR, copysign(x, NaR) flipped x, and nextafter(x, NaR) stepped up or down depending on x.
// fpclassify and isnormal went through long double: posit<1024,5> maxpos was FP_INFINITE and minpos
// FP_ZERO. nexttoward compared x as a posit<256,5>, which rounds a wider x. fdp read past a short
// vector when NDEBUG removed its assert.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <complex>
#include <string>
#include <vector>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	int report(bool ok, const std::string& what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL " << what << '\n';
		return ok ? 0 : 1;
	}

	template<typename Posit>
	int VerifyClassification(bool reportTestCases) {
		const std::string tag = type_tag(Posit()) + ' ';
		int fails = 0;
		Posit nar;
		nar.setnar();
		const Posit zero(0), maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos), maxneg(SpecificValue::maxneg), minneg(SpecificValue::minneg);
		fails += report(fpclassify(zero) == FP_ZERO && fpclassify(nar) == FP_NAN, tag + "fpclassify of 0 and NaR", reportTestCases);
		for (const Posit& p : { maxpos, minpos, maxneg, minneg, Posit(1), Posit(-3) }) {
			fails += report(fpclassify(p) == FP_NORMAL && isnormal(p) && isfinite(p) && !isinf(p) && !isnan(p) && !isdenorm(p),
			                tag + "fpclassify/isnormal/isfinite of " + to_binary(p), reportTestCases);
		}
		fails += report(!isnormal(zero) && !isnormal(nar) && isfinite(zero), tag + "isnormal of 0 and NaR", reportTestCases);
		fails += report(isnan(nar) && !isinf(nar) && !isfinite(nar) && !isdenorm(nar), tag + "NaR is a NaN and not an infinity", reportTestCases);
		return fails;
	}

	template<typename Posit>
	int VerifyNaRPropagation(bool reportTestCases) {
		const std::string tag = type_tag(Posit()) + ' ';
		int fails = 0;
		Posit nar;
		nar.setnar();
		for (const Posit& x : { Posit(0), Posit(1.5), Posit(-2), Posit(SpecificValue::maxpos), Posit(SpecificValue::maxneg), nar }) {
			const std::string at = to_binary(x);
			fails += report(min(nar, x).isnar() && min(x, nar).isnar(), tag + "min with NaR, x = " + at, reportTestCases);
			fails += report(max(nar, x).isnar() && max(x, nar).isnar(), tag + "max with NaR, x = " + at, reportTestCases);
			fails += report(copysign(x, nar).isnar() && copysign(nar, x).isnar(), tag + "copysign with NaR, x = " + at, reportTestCases);
			fails += report(nextafter(x, nar).isnar() && nextafter(nar, x).isnar(), tag + "nextafter with NaR, x = " + at, reportTestCases);
		}
		// the ordinary cases are unchanged
		fails += report(min(Posit(1), Posit(2)) == Posit(1) && max(Posit(1), Posit(2)) == Posit(2), tag + "min/max", reportTestCases);
		fails += report(copysign(Posit(3), Posit(-1)) == Posit(-3) && copysign(Posit(-3), Posit(1)) == Posit(3), tag + "copysign", reportTestCases);
		fails += report(copysign(Posit(3), Posit(0)) == Posit(3), tag + "copysign(3, 0) is 3", reportTestCases);
		return fails;
	}

	// nextafter steps one encoding toward the target, and returns the target when they are equal
	template<unsigned nbits, unsigned es>
	int VerifyNextafterExhaustive(bool reportTestCases) {
		using Posit = posit<nbits, es>;
		int fails = 0;
		for (unsigned i = 0; i < (1u << nbits); ++i) {
			Posit x;
			x.setbits(i);
			if (x.isnar()) continue;
			for (unsigned j = 0; j < (1u << nbits); ++j) {
				Posit t;
				t.setbits(j);
				if (t.isnar()) continue;
				Posit want = x;
				if (x < t) ++want;
				else if (x > t) --want;
				const Posit got = nextafter(x, t);
				if (got != want || got.isnar()) {
					++fails;
					if (reportTestCases && fails < 10) std::cerr << "FAIL nextafter(" << x << ", " << t << ") = " << got << ", expected " << want << '\n';
				}
			}
		}
		return fails;
	}

	int VerifyNexttoward(bool reportTestCases) {
		int fails = 0;
		// x = 1 + 2^-300 is a posit<512,6> but not a posit<256,5>, where it rounds to 1: the direction
		// toward 1 is down, and the old comparison in posit<256,5> saw x == target and did not move
		using Wide = posit<512, 6>;
		Wide x(1.0);
		x += Wide(std::ldexp(1.0, -300));
		Wide below = x;
		--below;
		fails += report(nexttoward(x, posit<256, 5>(1.0)) == below, "nexttoward(posit<512,6>(1 + 2^-300), 1) steps down", reportTestCases);
		Wide above = x;
		++above;
		fails += report(nexttoward(x, posit<256, 5>(2.0)) == above, "nexttoward(posit<512,6>(1 + 2^-300), 2) steps up", reportTestCases);
		posit<256, 5> nar256;
		nar256.setnar();
		fails += report(nexttoward(posit<16, 1>(1.0), nar256).isnar(), "nexttoward(x, NaR) is NaR", reportTestCases);
		fails += report(nexttoward(posit<16, 1>(1.0), posit<256, 5>(1.0)) == posit<16, 1>(1.0), "nexttoward(1, 1) is 1", reportTestCases);
		// a narrow x, and a target between x and its neighbour: still one step
		posit<16, 1> one(1.0), next(1.0);
		++next;
		posit<256, 5> between(1.0);
		between += posit<256, 5>(std::ldexp(1.0, -40));
		fails += report(nexttoward(one, between) == next, "nexttoward(posit<16,1>(1), 1 + 2^-40) steps up", reportTestCases);
		return fails;
	}

	int VerifyIntegerPowerAndComplex(bool reportTestCases) {
		int fails = 0;
		using P = posit<32, 2>;
		fails += report(integer_power(P(2), 10) == P(1024) && integer_power(P(2), -3) == P(0.125), "integer_power of 2", reportTestCases);
		fails += report(integer_power(P(-3), 3) == P(-27) && integer_power(P(7), 0) == P(1), "integer_power of -3 and x^0", reportTestCases);
		const complex<P> z(P(1.5), P(-2));
		fails += report(real(z) == P(1.5) && imag(z) == P(-2), "real and imag", reportTestCases);
		const complex<P> c = conj(z);
		fails += report(c.real() == P(1.5) && c.imag() == P(2), "conj", reportTestCases);
		return fails;
	}

	int VerifyFdpBounds(bool reportTestCases) {
		int fails = 0;
		using P = posit<16, 1>;
		const std::vector<P> x = { P(1), P(2), P(3) }, y = { P(4), P(5), P(6) }, shortY = { P(4), P(5) };
		fails += report(fdp(x, y) == P(32), "fdp of equal lengths is 32", reportTestCases);
		fails += report(fdp(x, shortY).isnar(), "fdp with a shorter y is NaR", reportTestCases);
		fails += report(fdp_stride(std::size_t(3), x, std::size_t(1), y, std::size_t(1)) == P(32), "fdp_stride is 32", reportTestCases);
		fails += report(fdp_stride(std::size_t(3), x, std::size_t(0), y, std::size_t(1)).isnar(), "fdp_stride with a zero stride is NaR", reportTestCases);
		fails += report(fdp_stride(std::size_t(3), x, std::size_t(1), shortY, std::size_t(1)).isnar(), "fdp_stride past y is NaR", reportTestCases);
		quire<P> q;
		fdp_qc(q, std::size_t(3), x, std::size_t(1), shortY, std::size_t(1));
		fails += report(q.isnar(), "fdp_qc past y makes the quire NaR", reportTestCases);
		return fails;
	}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite  = "posit NaR propagation and classification (#2071)";
	std::string test_tag    = "NaR and classify";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyClassification<posit<1024, 5>>(reportTestCases), test_tag, "posit<1024,5>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyClassification<posit<8, 0>>(reportTestCases), test_tag, "classify posit<8,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyClassification<posit<32, 2>>(reportTestCases), test_tag, "classify posit<32,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyClassification<posit<1024, 5>>(reportTestCases), test_tag, "classify posit<1024,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyNaRPropagation<posit<16, 1>>(reportTestCases), test_tag, "NaR posit<16,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyNaRPropagation<posit<128, 4>>(reportTestCases), test_tag, "NaR posit<128,4>");
	nrOfFailedTestCases += ReportTestResult((VerifyNextafterExhaustive<8, 0>(reportTestCases)), test_tag, "nextafter posit<8,0>");
	nrOfFailedTestCases += ReportTestResult((VerifyNextafterExhaustive<7, 2>(reportTestCases)), test_tag, "nextafter posit<7,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyNexttoward(reportTestCases), test_tag, "nexttoward");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerAndComplex(reportTestCases), test_tag, "integer_power, real, imag, conj");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpBounds(reportTestCases), test_tag, "fdp bounds");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyNextafterExhaustive<10, 1>(reportTestCases)), test_tag, "nextafter posit<10,1>");
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

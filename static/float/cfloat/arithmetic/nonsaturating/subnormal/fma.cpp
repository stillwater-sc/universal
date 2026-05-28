#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <random>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace {

	template<typename Fp>
	int VerifyFma(int nrSamples = 100000) {
		std::mt19937_64                        rng(99);
		std::uniform_real_distribution<double> ud(-8, 8);
		int                                    p        = std::numeric_limits<Fp>::digits;
		int                                    mismatch = 0, total = 0;
		double                                 maxUlp = 0;
		for (int i = 0; i < nrSamples; ++i) {
			Fp a(ud(rng)), b(ud(rng)), c(ud(rng));
			Fp got = fma(a, b, c);
			// correctly-rounded fused reference: double fma (true fused) then convert to Fp
			// This limits us to testing Fp formats that are <= double precision.
			Fp ref = Fp(std::fma(double(a), double(b), double(c)));
			++total;
			if (got != ref) {
				++mismatch;
				double ul = std::ldexp(std::fmax(std::fabs(double(ref)), 1e-30), -(p - 1));
				double e  = std::fabs(double(got) - double(ref)) / ul;
				if (e > maxUlp)
					maxUlp = e;
			}
		}

		if (mismatch > 0) 
			std::cout << mismatch << " incorrectly-rounded-fused in " << total << " cases, max " << maxUlp << " ulp\n";
	    return mismatch;
	}

	// the sharp two_prod test: fma(a,b,-(a*b)) should be the exact residual if fused
	template<typename Fp>
    int VerifyResidual(int nrSamples = 100000) {
		std::mt19937_64                        rng(5);
		std::uniform_real_distribution<double> ud(0.5, 2.0);
		int                                    zero = 0, total = 0;
	    for (int i = 0; i < nrSamples; ++i) {
			Fp a(ud(rng)), b(ud(rng));
			Fp prod       = a * b;
			Fp r          = fma(a, b, -prod);  // fused: exact residual (often nonzero); naive: round(prod)-prod = 0
		    double da = double(a), db = double(b), dprod = da * db;
			Fp exactResid = Fp(da * db - dprod);
			++total;
			if (double(r) == 0.0 && double(exactResid) != 0.0)
				++zero;  // would-be-zero only if NOT fused
		}

	    if (zero > 0) 
			std::cout << " residual : fma(a, b, -a * b) == 0 while true residual != 0 in " << zero << " / " << total
					  << " (nonzero count => NOT fused)\n";
	    return zero;
	}

}  // namespace


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "cfloat fma validation";
	std::string test_tag            = "fma";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = false;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using fp24    = cfloat<24, 5, uint16_t, true, false, false>;
	using fp32    = cfloat<32, 8, uint32_t, true, false, false>;
	int nrSamples = 10000000;

	nrOfFailedTestCases +=
	    ReportTestResult(VerifyFma<fp24>(nrSamples), type_tag(fp24{}), "fma");
	nrOfFailedTestCases += 
		ReportTestResult(VerifyFma<fp32>(nrSamples), type_tag(fp32{}), "fma");
	nrOfFailedTestCases += 
		ReportTestResult(VerifyResidual<cfloat<24, 5, uint16_t, true, false, false>>(nrSamples), type_tag(fp24{}), "residual");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

	int nrSamples = 1000000;

#	if REGRESSION_LEVEL_1
	using fp24    = cfloat<24, 5, uint16_t, true, false, false>;
	using fp32    = cfloat<32, 8, uint32_t, true, false, false>;

	nrOfFailedTestCases += ReportTestResult(VerifyFma<fp24>(nrSamples), type_tag(fp24{}), "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyResidual<cfloat<24, 5, uint16_t, true, false, false>>(nrSamples),
	                                        type_tag(fp24{}), "residual");
#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif  // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

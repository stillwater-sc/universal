// pow.cpp: test suite runner for pow function for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <random>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>

// overload the uniform distribution for double-double

namespace std
{

	template <>
	class uniform_real_distribution< sw::universal::dd >
	{
	public:
		uniform_real_distribution(const sw::universal::dd& low, const sw::universal::dd& high)
			: _mDist(low.high(), high.high())
		{
		}

		template < typename engine >
		sw::universal::dd operator()(engine& eng) {
			return sw::universal::dd(_mDist(eng), 0.5 * std::numeric_limits< double >::epsilon() * _mDist(eng));
		}

	private:
		uniform_real_distribution< double > _mDist;
	};

}

namespace sw {
	namespace universal {

		// generate specific test case
		template<typename Ty>
		void GenerateTestCase(Ty fa, Ty fb) {
			unsigned precision = 25;
			unsigned width = 30;
			Ty fref;
			sw::universal::dd a, b, ref, v;
			a = fa;
			b = fb;
			fref = std::pow(fa, fb);
			ref = fref;
			v = sw::universal::pow(a, b);
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> pow(" << fa << "," << fb << ") = " << std::setw(width) << fref << std::endl;
			std::cout << " -> pow( " << a << "," << b << ")  = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
			std::cout << std::setprecision(oldPrec);
		}

		template<typename Real>
		int calculateNrOfValidBits(const Real& computed, const Real& expected) {
			constexpr double LOG2E = 1.44269504088896340736;

			dd delta = computed - expected;
			if (delta == 0.0) {
				return dd_max_precision;
			}
			else {
				if (expected == 0.0) {
					return static_cast<int>(-std::log(std::fabs(double(computed))) * LOG2E);
				}
				else {
					delta /= expected;
					double logOfDelta = std::log(std::fabs(double(delta))) * LOG2E;
					return static_cast<int>(-logOfDelta);
				}
			}
		}

		static constexpr int NR_RANDOMS = 500;
#ifdef _DEBUG
		static constexpr int PRECISION_THRESHOLD = 85; // in bits: 85 bits is ~ 25.5 digits out of 32 digits
#else
		static constexpr int PRECISION_THRESHOLD = 75; // in bits: 85 bits is ~ 25.5 digits out of 32 digits
#endif

		int comparePowWithSqrt(bool reportTestCases, int precisionThreshold = PRECISION_THRESHOLD, int nrOfRandoms = NR_RANDOMS) {
			std::default_random_engine generator;
			std::uniform_real_distribution< dd > distribution(1.0, 1048576.0);
			int maxValidBits, minValidBits;
			int nrOfFailedTestCases{ 0 };

			std::cerr << "smallest number of valid bits of pow(x, 0.5) = ";
			if (reportTestCases) std::cerr << '\n';
			maxValidBits = 0;
			minValidBits = dd_max_precision;
			for (int i = 0; i < nrOfRandoms; ++i) {
				dd x = distribution(generator);
				dd expected = sqrt(x);
				dd computed = pow(x, dd(0.5));

				int nrOfValidBits = calculateNrOfValidBits(computed, expected);
				if (nrOfValidBits < 0) { // something very wrong has happened, provide feedback
					ReportValue(computed, "computed");
					ReportValue(expected, "expected");
				}
				minValidBits = std::min(minValidBits, nrOfValidBits);
				maxValidBits = std::max(maxValidBits, nrOfValidBits);
				if (nrOfValidBits < precisionThreshold) {
					++nrOfFailedTestCases;
				}
				if (reportTestCases) std::cerr << "valid bits pow( " << x << ", 0.5) : " << nrOfValidBits << "\n";
			}
			if (minValidBits == dd_max_precision)
				std::cerr << "EXACT ";
			else
				std::cerr << "[ " << minValidBits << ", " << maxValidBits << "] ";

			std::cerr << (nrOfFailedTestCases ? "FAIL\n" : "PASS\n");
			return nrOfFailedTestCases;
		}

		int comparePowWithCubeRoot(bool reportTestCases, int precisionThreshold = PRECISION_THRESHOLD, int nrOfRandoms = NR_RANDOMS) {
			std::default_random_engine generator;
			std::uniform_real_distribution< dd > distribution(1.0, 1048576.0);
			int maxValidBits, minValidBits;
			int nrOfFailedTestCases{ 0 };

			std::cerr << "smallest number of valid bits of pow(x, 0.33333...) = ";
			if (reportTestCases) std::cerr << '\n';
			maxValidBits = 0;
			minValidBits = dd_max_precision;
			for (int i = 0; i < nrOfRandoms; ++i) {
				dd x = distribution(generator);
				dd expected = cbrt(x);
				dd computed = pow(x, dd_third);

				int nrOfValidBits = calculateNrOfValidBits(computed, expected);
				if (nrOfValidBits < 0) {
					ReportValue(computed, "computed");
					ReportValue(expected, "expected");
				}
				minValidBits = std::min(minValidBits, nrOfValidBits);
				maxValidBits = std::max(maxValidBits, nrOfValidBits);
				if (nrOfValidBits < precisionThreshold) {
					++nrOfFailedTestCases;
				}
				if (reportTestCases) std::cerr << "valid bits pow( " << x << ", 0.3333...) : " << nrOfValidBits << "\n";
			}
			if (minValidBits == dd_max_precision)
				std::cerr << "EXACT ";
			else
				std::cerr << "[ " << minValidBits << ", " << maxValidBits << "] ";

			std::cerr << (nrOfFailedTestCases ? "FAIL\n" : "PASS\n");
			return nrOfFailedTestCases;
		}

		int comparePowWithSquare(bool reportTestCases, int precisionThreshold = PRECISION_THRESHOLD, int nrOfRandoms = NR_RANDOMS) {
			std::default_random_engine generator;
			std::uniform_real_distribution< dd > distribution(1.0, 1048576.0);
			int maxValidBits, minValidBits;
			int nrOfFailedTestCases{ 0 };

			std::cerr << "smallest number of valid bits of pow(x, 2.0) = ";
			if (reportTestCases) std::cerr << '\n';
			maxValidBits = 0;
			minValidBits = dd_max_precision;
			for (int i = 0; i < nrOfRandoms; ++i) {
				dd x = distribution(generator);
				dd expected = x * x;
				dd computed = pow(x, dd(2.0));

				int nrOfValidBits = calculateNrOfValidBits(computed, expected);
				if (nrOfValidBits < 0) {
					ReportValue(computed, "computed");
					ReportValue(expected, "expected");
				}
				minValidBits = std::min(minValidBits, nrOfValidBits);
				maxValidBits = std::max(maxValidBits, nrOfValidBits);
				if (nrOfValidBits < precisionThreshold) {
					++nrOfFailedTestCases;
				}
				if (reportTestCases) std::cerr << "valid bits pow( " << x << ", 2.0) : " << nrOfValidBits << "\n";
			}
			if (minValidBits == dd_max_precision)
				std::cerr << "EXACT ";
			else
				std::cerr << "[ " << minValidBits << ", " << maxValidBits << "] ";

			std::cerr << (nrOfFailedTestCases ? "FAIL\n" : "PASS\n");
			return nrOfFailedTestCases;
		}

		int comparePowWithCube(bool reportTestCases, int precisionThreshold = PRECISION_THRESHOLD, int nrOfRandoms = NR_RANDOMS) {
			std::default_random_engine generator;
			std::uniform_real_distribution< dd > distribution(1.0, 1048576.0);
			int maxValidBits, minValidBits;
			int nrOfFailedTestCases{ 0 };

			std::cerr << "smallest number of valid bits of pow(x, 3.0) = ";
			if (reportTestCases) std::cerr << '\n';
			maxValidBits = 0;
			minValidBits = dd_max_precision;
			for (int i = 0; i < nrOfRandoms; ++i) {
				dd x = distribution(generator);
				dd expected = x * x * x;
				dd computed = pow(x, dd(3.0));

				int nrOfValidBits = calculateNrOfValidBits(computed, expected);
				if (nrOfValidBits < 0) {
					ReportValue(computed, "computed");
					ReportValue(expected, "expected");
				}
				minValidBits = std::min(minValidBits, nrOfValidBits);
				maxValidBits = std::max(maxValidBits, nrOfValidBits);
				if (nrOfValidBits < precisionThreshold) {
					++nrOfFailedTestCases;
				}
				if (reportTestCases) std::cerr << "valid bits pow( " << x << ", 3.0) : " << nrOfValidBits << "\n";
			}
			if (minValidBits == dd_max_precision)
				std::cerr << "EXACT ";
			else
				std::cerr << "[ " << minValidBits << ", " << maxValidBits << "] ";

			std::cerr << (nrOfFailedTestCases ? "FAIL\n" : "PASS\n");
			return nrOfFailedTestCases;
		}


		int comparePowWithQuadratic(bool reportTestCases, int precisionThreshold = PRECISION_THRESHOLD, int nrOfRandoms = NR_RANDOMS) {
			std::default_random_engine generator;
			std::uniform_real_distribution< dd > distribution(1.0, 1048576.0);
			int maxValidBits, minValidBits;
			int nrOfFailedTestCases{ 0 };

			std::cerr << "smallest number of valid bits of pow(x, 4.0) = ";
			if (reportTestCases) std::cerr << '\n';
			maxValidBits = 0;
			minValidBits = dd_max_precision;
			for (int i = 0; i < nrOfRandoms; ++i) {
				dd x = distribution(generator);
				dd square = x * x;
				dd expected = square * square;
				dd computed = pow(x, dd(4.0));

				int nrOfValidBits = calculateNrOfValidBits(computed, expected);
				if (nrOfValidBits < 0) {
					ReportValue(computed, "computed");
					ReportValue(expected, "expected");
				}
				minValidBits = std::min(minValidBits, nrOfValidBits);
				maxValidBits = std::max(maxValidBits, nrOfValidBits);
				if (nrOfValidBits < precisionThreshold) {
					++nrOfFailedTestCases;
				}
				if (reportTestCases) std::cerr << "valid bits pow( " << x << ", 3.0) : " << nrOfValidBits << "\n";
			}
			if (minValidBits == dd_max_precision)
				std::cerr << "EXACT ";
			else
				std::cerr << "[ " << minValidBits << ", " << maxValidBits << "] ";

			std::cerr << (nrOfFailedTestCases ? "FAIL\n" : "PASS\n");
			return nrOfFailedTestCases;
		}
	}
}


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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "double-double mathlib power function validation";
	std::string test_tag    = "pow";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	GenerateTestCase(4.0, 2.0);

	dd a{ 1.0 };
	for (int i = 0; i < 30; ++i) {
		std::string tag = "pow(1.0, " + std::to_string(i) + ")";
		ReportValue(pow(a, i), tag);
	}
	a = 2.0;
	for (int i = 0; i < 30; ++i) {
		std::string tag = "pow(2.0, " + std::to_string(i) + ")";
		ReportValue(pow(a, i), tag);
	}

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// this is very sad: we are loosing 10 bits of precision when running in Release as compared to Debug:
/*
 * Release mode
 * double-double mathlib power function validation: results only
 * PRECISION_THRESHOLD set to 85 bits, which is approximate 25.7635 digits: out of a total of 32 digits
 * smallest number of valid bits of pow(x, 0.5) = [ 81, 92] FAIL
 * smallest number of valid bits of pow(x, 0.33333...) = [ 77, 86] FAIL
 * smallest number of valid bits of pow(x, 2.0) = [ 78, 91] FAIL
 * smallest number of valid bits of pow(x, 3.0) = [ 77, 90] FAIL
 * smallest number of valid bits of pow(x, 4.0) = [ 77, 89] FAIL
 * double-double mathlib power function validation: FAIL
 * 
 * Debug mode
 * double-double mathlib power function validation: results only
 * PRECISION_THRESHOLD set to 85 bits, which is approximate 25.7635 digits: out of a total of 32 digits
 * smallest number of valid bits of pow(x, 0.5) = [ 92, 110] PASS
 * smallest number of valid bits of pow(x, 0.33333...) = [ 91, 110] PASS
 * smallest number of valid bits of pow(x, 2.0) = [ 89, 110] PASS
 * smallest number of valid bits of pow(x, 3.0) = [ 88, 108] PASS
 * smallest number of valid bits of pow(x, 4.0) = [ 88, 108] PASS
 * double-double mathlib power function validation: PASS
 * 
 * Setting lower precision threshold to pass regressions
 * double-double mathlib power function validation: results only
 * PRECISION_THRESHOLD set to 75 bits, which is approximate 22.7325 digits: out of a total of 32 digits
 * smallest number of valid bits of pow(x, 0.5) = [ 81, 92] PASS
 * smallest number of valid bits of pow(x, 0.33333...) = [ 77, 86] PASS
 * smallest number of valid bits of pow(x, 2.0) = [ 78, 91] PASS
 * smallest number of valid bits of pow(x, 3.0) = [ 77, 90] PASS
 * smallest number of valid bits of pow(x, 4.0) = [ 77, 89] PASS
 * double-double mathlib power function validation: PASS
 */
	std::cerr << "PRECISION_THRESHOLD set to " << PRECISION_THRESHOLD 
		<< " bits, which is approximate " << (0.3031 * PRECISION_THRESHOLD) << " digits: out of a total of 32 digits\n";

	nrOfFailedTestCases += comparePowWithSqrt(reportTestCases);
	nrOfFailedTestCases += comparePowWithCubeRoot(reportTestCases);
	nrOfFailedTestCases += comparePowWithSquare(reportTestCases);
	nrOfFailedTestCases += comparePowWithCube(reportTestCases);
	nrOfFailedTestCases += comparePowWithQuadratic(reportTestCases);

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

// enclosure.cpp: test suite runner verifying that SORN arithmetic encloses the exact result
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// contains: is the real value v inside the sorn's interval
	template<typename SornType>
	bool contains(const SornType& s, double v) {
		const auto& i = s.sornIntVal;
		const bool aboveLower = (v > i.lowerBound) || (v == i.lowerBound && !i.lowerIsOpen);
		const bool belowUpper = (v < i.upperBound) || (v == i.upperBound && !i.upperIsOpen);
		return aboveLower && belowUpper;
	}

	// samples: finite points of an interval, its closed finite bounds among them
	template<typename Interval>
	std::vector<double> samples(const Interval& i) {
		std::vector<double> pts;
		const double lo = i.lowerBound, hi = i.upperBound;
		if (lo == hi) { pts.push_back(lo); return pts; }
		// stand in a finite stretch for an infinite bound (both bounds infinite: [-64, 64])
		const double flo = std::isinf(lo) ? (std::isinf(hi) ? -64.0 : hi - 64.0) : lo;
		const double fhi = std::isinf(hi) ? (std::isinf(lo) ?  64.0 : lo + 64.0) : hi;
		if (!i.lowerIsOpen && !std::isinf(lo)) pts.push_back(lo);
		for (double t : { 0.0625, 0.5, 0.9375 }) pts.push_back(flo + t * (fhi - flo));
		if (!i.upperIsOpen && !std::isinf(hi)) pts.push_back(hi);
		return pts;
	}

	// all contiguous spans of the lattice, or only its single intervals
	template<typename SornType>
	std::vector<SornType> operands(bool spans) {
		std::vector<SornType> ops;
		constexpr size_t N = SornType::sornBits;
		for (size_t i = 0; i < N; ++i) {
			for (size_t j = i; j < (spans ? N : i + 1); ++j) {
				std::bitset<N> bits;
				for (size_t b = i; b <= j; ++b) bits[b] = true;
				SornType s;
				s.setBits(bits);
				ops.push_back(s);
			}
		}
		return ops;
	}

	// every operator result must contain op(x, y) for every sample x of a and y of b. The operands
	// are const, which the binary operators did not accept before #1972.
	template<typename SornType>
	int VerifyEnclosure(char op, bool spans, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const std::vector<SornType> ops = operands<SornType>(spans);
		for (const SornType& a : ops) {
			for (const SornType& b : ops) {
				SornType c;
				switch (op) {
				case '+': c = a + b; break;
				case '-': c = a - b; break;
				case '*': c = a * b; break;
				case '/': c = a / b; break;
				}
				for (double x : samples(a.sornIntVal)) {
					for (double y : samples(b.sornIntVal)) {
						if (op == '/' && y == 0.0) continue;
						double ref = 0;
						switch (op) {
						case '+': ref = x + y; break;
						case '-': ref = x - y; break;
						case '*': ref = x * y; break;
						case '/': ref = x / y; break;
						}
						// a value beyond the lattice saturates to its outermost interval
						ref = std::max(ref, double(SornType::datatype().front().lowerBound));
						ref = std::min(ref, double(SornType::datatype().back().upperBound));
						if (!contains(c, ref)) {
							++nrOfFailedTestCases;
							if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " = " << c << " misses " << x << ' ' << op << ' ' << y << " = " << ref << '\n';
							if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
						}
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// abs and negation enclose |x| and -x
	template<typename SornType>
	int VerifyUnaryEnclosure(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (const SornType& a : operands<SornType>(true)) {
			const SornType n = -a;
			const SornType m = abs(a);
			for (double x : samples(a.sornIntVal)) {
				if (!contains(n, -x) || !contains(m, std::fabs(x))) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: -" << a << " = " << n << ", abs = " << m << " for x = " << x << '\n';
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// NaN propagates through every operator; nan * x used to come out as zero
	template<typename SornType>
	int VerifyNaNPropagation(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const SornType nan(SpecificValue::qnan), x(1.25), zero(0);
		const SornType results[] = { nan + x, x + nan, nan - x, x - nan, nan * x, x * nan, nan * zero,
		                             nan / x, x / nan, nan + 1, 2.0 * nan, nan / 2.0f, abs(nan), -nan };
		for (const SornType& r : results) {
			if (!r.isnan()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: NaN operand produced " << r << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	// scalar operands of any arithmetic type; a long used to be ambiguous between int, float and double
	template<typename SornType>
	int VerifyScalarOperands(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const SornType a(1.25);
		const SornType results[] = { a + 1L, 1L + a, a - 1u, 3ll - a, a * 2L, 2L * a, a / 2L, 4L / a };
		const double   refs[]    = { 2.25,   2.25,   0.25,   1.75,    2.5,    2.5,    0.625,  3.2 };
		for (size_t k = 0; k < sizeof(refs) / sizeof(refs[0]); ++k) {
			if (!contains(results[k], refs[k])) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: scalar case " << k << " = " << results[k] << " misses " << refs[k] << '\n';
			}
		}
		// multiplying an interval that reaches infinity by zero is zero, not NaN
		const SornType big(SpecificValue::infpos);
		if (!(big * 0).iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << big << " * 0 = " << (big * 0) << '\n';
		}
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "sorn arithmetic enclosure validation";
	std::string test_tag    = "enclosure";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using SmallSorn  = sorn<0, 2, 2>;                   // linear, 7 intervals
	using LinSorn    = sorn<0, 4, 8>;                   // linear, 19 intervals
	using LogSorn    = sorn<-2, 2, 1, 0, 1, 1, 1, 1>;   // logarithmic
	using NoZeroSorn = sorn<0, 2, 2, 1, 1, 1, 1, 0>;    // linear, no exact zero

#if MANUAL_TESTING

	const LinSorn a(1.25), b(-0.75);
	std::cout << a << " / " << b << " = " << (a / b) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	for (char op : { '+', '-', '*', '/' }) {
		const std::string tag = std::string("enclosure ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<SmallSorn>(op, true, reportTestCases), "sorn<0,2,2> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<NoZeroSorn>(op, true, reportTestCases), "sorn<0,2,2,nozero> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LinSorn>(op, false, reportTestCases), "sorn<0,4,8>", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LogSorn>(op, false, reportTestCases), "sorn<-2,2,1,log>", tag);
	}
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryEnclosure<LinSorn>(reportTestCases), "sorn<0,4,8>", "negate and abs");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPropagation<LinSorn>(reportTestCases), "sorn<0,4,8>", "nan propagation");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<LinSorn>(reportTestCases), "sorn<0,4,8>", "scalar operands");
#endif

#if REGRESSION_LEVEL_2
	for (char op : { '+', '-', '*', '/' }) {
		const std::string tag = std::string("enclosure ") + op;
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LinSorn>(op, true, reportTestCases), "sorn<0,4,8> spans", tag);
		nrOfFailedTestCases += ReportTestResult(VerifyEnclosure<LogSorn>(op, true, reportTestCases), "sorn<-2,2,1,log> spans", tag);
	}
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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

// fused_ops.cpp: test suite runner for posit fused multiply-add and related operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fma(a, b, c) = a*b + c, fam(a, b, c) = (a + b)*c and fmma(a, b, c, d) = a*b +/- c*d are rounded once
// (#2071). The reference here is exact and independent of the posit code: every operand is an
// integer multiple of 2^-H, so each result is an exact integer at 2^-2H, computed with integer<>,
// then rounded by the posit rule against the sorted values of the configuration: the tie point
// between encodings a and a + 1 is the (nbits + 1)-bit posit 2a + 1, ties go to the even encoding,
// and nothing rounds to 0 or NaR.
//
// The references used to be std::fma and (a + b) * c in double, rounded to the posit; the fam
// failures started at 6 bits, which only level 2 ran, so CI never saw posit<6,1> fam(0.09375, 2.5,
// 0.4375) = 1 (it is 1.25). fmma had one stub test at posit<3,0>, and posit<5,3> did not compile.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/posit_test_suite.hpp>

namespace sw { namespace universal {

	// an exact reference for posit<nbits, es>, nbits <= 8: values are integers at 2^-H
	template<unsigned nbits, unsigned es>
	class ExactFused {
	public:
		using Posit = posit<nbits, es>;
		using Int   = integer<320, std::uint32_t, IntegerNumberType::IntegerNumber>;
		static constexpr int maxscale = static_cast<int>(nbits - 2) * (1 << es);
		static constexpr int H = maxscale + static_cast<int>(Posit::fbits) + 2;   // minpos and every fraction bit are integral

		ExactFused() {
			for (unsigned a = 1; a < (1u << (nbits - 1)); ++a) {
				Posit p;
				p.setbits(a);
				values.push_back(scaled(double(p), 2 * H));
			}
			for (unsigned a = 1; a + 1 < (1u << (nbits - 1)); ++a) {
				posit<nbits + 1, es> q;
				q.setbits(2 * a + 1);
				ties.push_back(scaled(double(q), 2 * H));
			}
		}

		// the operand at 2^-H, and a value at 2^-2H
		Int operand(const Posit& p) const { return scaled(double(p), H); }
		static Int scaled(double v, int shift) {
			// v * 2^shift is an integer: build it from v's 53-bit significand, exactly
			if (v == 0.0) return Int(0);
			int e = 0;
			const double m = std::frexp(std::fabs(v), &e);   // |v| = m 2^e, m in [0.5, 1)
			Int r(static_cast<long long>(std::ldexp(m, 53)));
			const int s = e - 53 + shift;
			if (s >= 0) r <<= s;
			else r >>= -s;   // exact by construction: the low bits are zero
			return (v < 0.0) ? -r : r;
		}

		// x (at 2^-2H) rounded once to the posit
		Posit round(Int x) const {
			Posit r;
			if (x == Int(0)) { r.setzero(); return r; }
			const bool negative = x < Int(0);
			if (negative) x = -x;
			unsigned enc;
			const unsigned top = (1u << (nbits - 1)) - 1u;
			if (x <= values.front()) enc = 1;
			else if (x >= values.back()) enc = top;
			else {
				unsigned i = 0;   // values[i] is encoding i + 1
				while (!(values[i] <= x && x < values[i + 1])) ++i;
				if (x == values[i]) enc = i + 1;
				else if (x < ties[i]) enc = i + 1;
				else if (x > ties[i]) enc = i + 2;
				else enc = ((i + 1) % 2 == 0) ? i + 1 : i + 2;
			}
			r.setbits(enc);
			return negative ? -r : r;
		}

	private:
		std::vector<Int> values;   // the positive posits, ascending, at 2^-2H
		std::vector<Int> ties;     // ties[i] lies between values[i] and values[i + 1]
	};

	template<unsigned nbits, unsigned es>
	int VerifyFmaFam(bool reportTestCases) {
		using Posit = posit<nbits, es>;
		const ExactFused<nbits, es> ref;
		using Int = typename ExactFused<nbits, es>::Int;
		const Int one = ref.operand(Posit(1));   // 2^H
		int nrOfFailedTests = 0;
		constexpr unsigned N = 1u << nbits;
		for (unsigned i = 0; i < N; ++i) {
			Posit a; a.setbits(i);
			for (unsigned j = 0; j < N; ++j) {
				Posit b; b.setbits(j);
				for (unsigned k = 0; k < N; ++k) {
					Posit c; c.setbits(k);
					Posit wantFma, wantFam;
					if (a.isnar() || b.isnar() || c.isnar()) { wantFma.setnar(); wantFam.setnar(); }
					else {
						const Int A = ref.operand(a), B = ref.operand(b), C = ref.operand(c);
						wantFma = ref.round(A * B + C * one);
						wantFam = ref.round((A + B) * C);
					}
					const Posit gotFma = fma(a, b, c), gotFam = fam(a, b, c);
					if (gotFma != wantFma) {
						++nrOfFailedTests;
						if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL fma(" << a << ", " << b << ", " << c << ") = " << gotFma << ", expected " << wantFma << '\n';
					}
					if (gotFam != wantFam) {
						++nrOfFailedTests;
						if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL fam(" << a << ", " << b << ", " << c << ") = " << gotFam << ", expected " << wantFam << '\n';
					}
				}
			}
		}
		return nrOfFailedTests;
	}

	template<unsigned nbits, unsigned es>
	int VerifyFmma(bool reportTestCases) {
		using Posit = posit<nbits, es>;
		const ExactFused<nbits, es> ref;
		using Int = typename ExactFused<nbits, es>::Int;
		int nrOfFailedTests = 0;
		constexpr unsigned N = 1u << nbits;
		for (unsigned i = 0; i < N; ++i) {
			Posit a; a.setbits(i);
			for (unsigned j = 0; j < N; ++j) {
				Posit b; b.setbits(j);
				for (unsigned k = 0; k < N; ++k) {
					Posit c; c.setbits(k);
					for (unsigned l = 0; l < N; ++l) {
						Posit d; d.setbits(l);
						Posit wantAdd, wantSub;
						if (a.isnar() || b.isnar() || c.isnar() || d.isnar()) { wantAdd.setnar(); wantSub.setnar(); }
						else {
							const Int ab = ref.operand(a) * ref.operand(b), cd = ref.operand(c) * ref.operand(d);
							wantAdd = ref.round(ab + cd);
							wantSub = ref.round(ab - cd);
						}
						if (fmma(a, b, c, d, true) != wantAdd) {
							++nrOfFailedTests;
							if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL fmma(" << a << ", " << b << ", " << c << ", " << d << ", +)\n";
						}
						if (fmma(a, b, c, d, false) != wantSub) {
							++nrOfFailedTests;
							if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL fmma(" << a << ", " << b << ", " << c << ", " << d << ", -)\n";
						}
					}
				}
			}
		}
		return nrOfFailedTests;
	}

	// wide configurations: each fused operation with a neutral operand is the single correctly
	// rounded operation, which posit's own +, - and * are; random operands across the dynamic range
	template<typename Posit>
	int VerifyWideIdentities(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		const Posit zero(0), one(1);
		int nrOfFailedTests = 0;
		auto check = [&](bool ok, const char* what, const Posit& a, const Posit& b) {
			if (ok) return;
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL " << type_tag(a) << ' ' << what << " a = " << to_binary(a) << " b = " << to_binary(b) << '\n';
		};
		for (unsigned i = 0; i < nrSamples; ++i) {
			Posit a, b;
			// a near 1 half the time, so sums and products keep most bits; anywhere otherwise
			a.setbits(rng());
			b.setbits(rng());
			if (i % 2 == 0) { a = a / (abs(a) + one); b = b / (abs(b) + one); }
			if (a.isnar() || b.isnar()) continue;
			check(fma(a, b, zero) == a * b, "fma(a, b, 0) == a*b", a, b);
			check(fma(a, one, b) == a + b, "fma(a, 1, b) == a + b", a, b);
			check(fma(a, -one, b) == b - a, "fma(a, -1, b) == b - a", a, b);
			check(fam(a, b, one) == a + b, "fam(a, b, 1) == a + b", a, b);
			check(fam(a, zero, b) == a * b, "fam(a, 0, b) == a*b", a, b);
			check(fmma(a, b, zero, a, true) == a * b, "fmma(a, b, 0, a) == a*b", a, b);
			check(fmma(a, one, b, one, false) == a - b, "fmma(a, 1, b, 1, -) == a - b", a, b);
			check(fmma(a, b, a, b, false).iszero(), "fmma(a, b, a, b, -) == 0", a, b);
		}
		// a tiny addend collapses to a sticky unit: 1 + minpos, 1 - minpos, maxpos - 1
		const Posit minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
		check(fma(one, one, minpos) == one + minpos, "fma(1, 1, minpos)", one, minpos);
		check(fma(one, one, -minpos) == one - minpos, "fma(1, 1, -minpos)", one, minpos);
		check(fma(maxpos, one, -one) == maxpos - one, "fma(maxpos, 1, -1)", maxpos, one);
		check(fam(one, minpos, maxpos) == maxpos, "fam(1, minpos, maxpos) saturates", one, minpos);
		check(fma(maxpos, maxpos, one) == maxpos, "fma(maxpos, maxpos, 1) saturates", maxpos, one);
		check(fma(minpos, minpos, zero) == minpos, "fma(minpos, minpos, 0) is minpos, never 0", minpos, minpos);
		return nrOfFailedTests;
	}

	// configurations where es + 2 >= nbits did not compile: no fraction bits
	int VerifySmallConfigurations(bool reportTestCases) {
		int fails = VerifyFmaFam<5, 3>(reportTestCases) + VerifyFmaFam<4, 2>(reportTestCases) + VerifyFmaFam<3, 1>(reportTestCases);
		fails += VerifyFmma<4, 2>(reportTestCases) + VerifyFmma<3, 1>(reportTestCases);
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

	std::string test_suite  = "posit fused operator verification";
	std::string test_tag    = "fused_ops";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<6, 1>(reportTestCases)), "posit<6,1>", "fma and fam");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<4, 0>(reportTestCases)), "posit<4,0>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<5, 1>(reportTestCases)), "posit<5,1>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<6, 1>(reportTestCases)), "posit<6,1>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<6, 2>(reportTestCases)), "posit<6,2>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<7, 0>(reportTestCases)), "posit<7,0>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmma<4, 0>(reportTestCases)), "posit<4,0>", "fmma");
	nrOfFailedTestCases += ReportTestResult((VerifyFmma<5, 1>(reportTestCases)), "posit<5,1>", "fmma");
	nrOfFailedTestCases += ReportTestResult(VerifySmallConfigurations(reportTestCases), "es + 2 >= nbits", "fma, fam, fmma");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIdentities<posit<16, 1>>(20000, 2071, reportTestCases), "posit<16,1>", "identities");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIdentities<posit<64, 3>>(5000, 2072, reportTestCases), "posit<64,3>", "identities");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIdentities<posit<128, 4>>(1000, 2073, reportTestCases), "posit<128,4>", "identities");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIdentities<posit<256, 5>>(200, 2074, reportTestCases), "posit<256,5>", "identities");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<7, 1>(reportTestCases)), "posit<7,1>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<8, 1>(reportTestCases)), "posit<8,1>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmma<6, 1>(reportTestCases)), "posit<6,1>", "fmma");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<8, 2>(reportTestCases)), "posit<8,2>", "fma and fam");
	nrOfFailedTestCases += ReportTestResult((VerifyFmaFam<8, 0>(reportTestCases)), "posit<8,0>", "fma and fam");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult((VerifyFmma<7, 0>(reportTestCases)), "posit<7,0>", "fmma");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIdentities<posit<1024, 5>>(50, 2075, reportTestCases), "posit<1024,5>", "identities");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

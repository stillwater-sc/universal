// log_kernel.cpp: regression tests for efloat's fixed-point logarithm kernel (#1795)
//
// log, log2, log10, log1p, asinh, acosh, atanh and pow evaluate log x in log_working:
//     log x = e ln 2 + log c + 2 atanh(v),   v = (m - c) / (m + c),   |v| < 2^-10,
// with c = j / 512 from a table, and atanh(v) = v S(v^2) summed in fixed point
// (atanh_working_small, on fixed_series::arctangent_series without the alternation), or by the
// AGM far above 8192 bits. Ziv's strategy retries an evaluation it cannot round, so a kernel
// whose error broke its bound could still give correctly rounded results, only slower: the
// rounded results do not test the bound. This test does, against a reference that shares none
// of the kernel's code -- four exact square roots and the plain series
//     log m = 32 atanh((r - 1) / (r + 1)),   r = m^(1/16),
// in efloat arithmetic at 2P + 256 bits, with ln 2 = 2 atanh(1/3) summed alike:
//   - log_working at P + exp_log_guard_bits bits within the declared
//     2^(scale - working + log_error_bits);
//   - x random over many binades, next to 1 down to 1 +- 2^-P, at the table's grid points and
//     half-way between them, either side of sqrt(2), and large and tiny (e ln 2 dominating);
//   - atanh_working_small alone, for v from 2^-10 down to below 2^-P;
//   - at level 4, precisions either side of the AGM switch (45000 working bits from the table).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 11 precisions from 24 to 2048 bits, random arguments and the edges, and 8192 bits.
//   LEVEL 2 -- four times the random cases.
//   LEVEL 3 -- sixteen times, and 4096 and 8192 bits.
//   LEVEL 4 -- sixty-four times, and either side of the AGM switch.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using E = sw::universal::efloat<3200>;

	// 2 atanh(u) = 2 (u + u^3/3 + u^5/5 + ...) at `bits`
	E reference_two_atanh(const E& u, unsigned bits) {
		E x(u);
		x.set_precision(bits);
		const E u2 = x * x;
		E power = x, sum = x;
		for (unsigned k = 1; ; ++k) {
			power *= u2;
			const E term = power / (2 * k + 1);
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 4) break;
			sum += term;
		}
		sum.setexponent(sum.scale() + 1);
		return sum;
	}

	// log x for a finite x > 0 at `bits`: x = 2^e m, m in [1/2, 1), log m from m^(1/16)
	E reference_log(const E& x, unsigned bits) {
		E m(x);
		m.set_precision(bits);
		const int64_t e = x.scale() + 1;
		m.setexponent(-1);
		for (int i = 0; i < 4; ++i) m = sqrt(m);   // exact square roots, rounded at `bits`
		E one(1);
		one.set_precision(bits);
		E y = reference_two_atanh((m - one) / (m + one), bits);
		y.setexponent(y.scale() + 4);   // times 16
		if (e != 0) {
			E third(1);
			third.set_precision(bits);
			third /= 3;
			E ln2 = reference_two_atanh(third, bits);
			E ew;
			ew.set_precision(bits);
			ew = e;
			y += ew * ln2;
		}
		return y;
	}

	// a full-width P-bit value in [1, 2) times 2^e
	E random_value(std::mt19937_64& rng, unsigned P, int64_t e) {
		E v(1);
		v.set_precision(P);
		for (unsigned i = 0; i < (P + 31) / 32; ++i) v.setlimb(i, static_cast<uint32_t>(rng()) | (i == 0 ? 0x80000000u : 0u));
		v.set_precision(P);
		v.setexponent(e);
		return v;
	}

	// c + d or c - d for d a random P-bit value below 2^-e
	E next_to(std::mt19937_64& rng, unsigned P, const E& c, int64_t e) {
		const E d = random_value(rng, P, -e);
		E x(c);
		x.set_precision(P);
		x = (rng() & 1u) ? x + d : x - d;
		x.set_precision(P);
		return x;
	}

	bool log_within_bound(const E& x, unsigned P, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		if (x == E(1)) return true;
		const unsigned working = P + exp_log_guard_bits;
		const unsigned bits    = (P <= 8192) ? 2 * P + 256 : P + 256;
		const E w   = log_working(x, working);
		const E ref = reference_log(x, bits);
		const E err = w - ref;
		if (err.iszero()) return true;
		const int64_t bound = w.scale() - static_cast<int64_t>(working) + static_cast<int64_t>(log_error_bits);
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			std::cerr << "FAIL: log_working at P = " << P << ", x = " << double(x) << " (scale " << x.scale()
			          << "): error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	// atanh_working_small(v) within 2^-(working - 3) of atanh v, relative
	bool atanh_within_bound(const E& v, unsigned P, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		const unsigned working = P + exp_log_guard_bits;
		const E w = atanh_working_small(v, working);
		E ref = reference_two_atanh(v, 2 * P + 256);
		ref.setexponent(ref.scale() - 1);
		const E err = w - ref;
		if (err.iszero()) return true;
		const int64_t bound = ref.scale() - static_cast<int64_t>(working) + 3;
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			std::cerr << "FAIL: atanh_working_small at P = " << P << ", v scale " << v.scale()
			          << ": error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	int VerifyLogKernel(const std::vector<unsigned>& precisions, unsigned cases, bool reportTestCases) {
		int fails = 0;
		std::mt19937_64 rng(1795);
		for (unsigned P : precisions) {
			const unsigned n  = (P > 1024) ? std::max(4u, cases / 4u) : cases;
			const int64_t  Ps = static_cast<int64_t>(P);
			auto check = [&](const E& x) { if (!log_within_bound(x, P, reportTestCases)) ++fails; };
			const E one(1);
			for (unsigned c = 0; c < n; ++c) {
				check(random_value(rng, P, static_cast<int64_t>(rng() % 64) - 32));          // many binades
				check(next_to(rng, P, one, 1 + static_cast<int64_t>(rng() % P)));            // next to 1
				const unsigned j = 362u + static_cast<unsigned>(rng() % 363u);               // a grid point, or half-way
				E g(static_cast<double>(2 * j + (rng() & 1u)) / 1024.0);
				g.setexponent(g.scale() + static_cast<int64_t>(rng() % 9) - 4);
				check(next_to(rng, P, g, 12 + static_cast<int64_t>(rng() % P)));
			}
			for (int64_t e : { int64_t(1), int64_t(10), int64_t(11), Ps / 2, Ps - 1, Ps }) check(next_to(rng, P, one, e));
			check(next_to(rng, P, E(1.4142135623730951), 30));   // either side of sqrt(2)
			check(next_to(rng, P, E(1.4142135623730951), Ps));
			for (int64_t e : { int64_t(-3000), int64_t(-61), int64_t(62), int64_t(2999) }) check(random_value(rng, P, e));
			// the series alone, v from the table's 2^-10 down past 2^-P
			for (int64_t e : { int64_t(-11), int64_t(-20), -Ps / 2, -(Ps + 34) / 2, -Ps - 8 }) {
				E v = random_value(rng, P + 64, e);
				if (rng() & 1u) v = -v;
				if (!atanh_within_bound(v, P, reportTestCases)) ++fails;
			}
		}
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat fixed-point logarithm kernel";
	std::string test_tag            = "log kernel";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::vector<unsigned> precisions = { 24, 32, 53, 64, 100, 128, 256, 512, 1000, 1400, 2048 };

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel({ 128 }, 20, reportTestCases), "efloat", "log kernel 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel(precisions, 16, reportTestCases), "efloat", "error bounds, 24 to 2048 bits");
	// from about 4096 bits a block of the series holds more than one group of terms
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel({ 8192 }, 4, reportTestCases), "efloat", "error bounds, 8192 bits");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel(precisions, 64, reportTestCases), "efloat", "error bounds, 4x cases");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel({ 4096, 8192 }, 16, reportTestCases), "efloat", "error bounds, 4096 and 8192 bits, 4x cases");
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel(precisions, 256, reportTestCases), "efloat", "error bounds, 16x cases");
#	endif
#	if REGRESSION_LEVEL_4
	// working = P + 64 either side of 45000 = log_agm_bits_per_series_bit x 10
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel({ 44864, 45056 }, 4, reportTestCases), "efloat", "error bounds, either side of the AGM switch");
	nrOfFailedTestCases += ReportTestResult(VerifyLogKernel(precisions, 1024, reportTestCases), "efloat", "error bounds, 64x cases");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

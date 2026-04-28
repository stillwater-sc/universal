#pragma once
//  lns_test_suite.hpp : test suite for the logarithmic number system (lns)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_algorithms.hpp>

namespace sw { namespace universal {

template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
int VerifyAddition(bool reportTestCases) {
	constexpr size_t nbits = LnsType::nbits;
	// constexpr size_t rbits = LnsType::rbits;
	// constexpr Behavior behavior = LnsType::behavior;
	// using bt = typename LnsType::BlockType;
	constexpr size_t NR_ENCODINGS = (1ull << nbits);

	int nrOfFailedTestCases = 0;

	LnsType a, b, c, cref;
	for (size_t i = 0; i < NR_ENCODINGS; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 0; j < NR_ENCODINGS; ++j) {
			b.setbits(j);
			double db = double(b);

			double ref = da + db;
			if (reportTestCases && !isInRange<LnsType>(ref)) {
				std::cerr << da << " + " << db << " = " << ref << " which is not in range " << range(a) << '\n';
			}
			c    = a + b;
			cref = ref;
			// Use the per-algorithm tolerance contract: bit-exact for
			// DoubleTrip / Direct, value-domain relative tolerance for the
			// approximate algorithms (Lookup, Polynomial, ArnoldBailey).
			// See lns_eq_within_alg_tolerance / lns_addsub_log_error_bound
			// in lns_addsub_algorithms.hpp. Both-NaN is treated as equivalent
			// inside the helper; one-sided NaN is a fail.
			if (!lns_eq_within_alg_tolerance(c, cref)) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportBinaryArithmeticError("FAIL", "+", a, b, c, cref);
			}
			else {
				if (reportTestCases)
					ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
			}
			if (nrOfFailedTestCases > 24)
				return nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
int VerifySubtraction(bool reportTestCases) {
	constexpr size_t nbits = LnsType::nbits;
	// constexpr size_t rbits = LnsType::rbits;
	// constexpr Behavior behavior = LnsType::behavior;
	// using bt = typename LnsType::BlockType;
	constexpr size_t NR_ENCODINGS = (1ull << nbits);

	int nrOfFailedTestCases = 0;

	LnsType a{}, b{}, c{}, cref{};
	for (size_t i = 0; i < NR_ENCODINGS; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 0; j < NR_ENCODINGS; ++j) {
			b.setbits(j);
			double db = double(b);

			double ref = da - db;
			if (reportTestCases && !isInRange<LnsType>(ref)) {
				std::cerr << da << " - " << db << " = " << ref << " which is not in range " << range(a) << '\n';
			}
			c    = a - b;
			cref = ref;
			// Per-algorithm tolerance: see lns_eq_within_alg_tolerance.
			if (!lns_eq_within_alg_tolerance(c, cref)) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportBinaryArithmeticError("FAIL", "-", a, b, c, cref);
			}
			else {
				if (reportTestCases)
					ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, ref);
			}
			if (nrOfFailedTestCases > 24)
				return nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
int VerifyMultiplication(bool reportTestCases) {
	constexpr size_t nbits = LnsType::nbits;
	// constexpr size_t rbits = LnsType::rbits;
	// constexpr Behavior behavior = LnsType::behavior;
	// using bt = typename LnsType::BlockType;
	constexpr size_t NR_ENCODINGS = (1ull << nbits);

	int nrOfFailedTestCases = 0;

	LnsType a{}, b{}, c{}, cref{};
	for (size_t i = 0; i < NR_ENCODINGS; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 0; j < NR_ENCODINGS; ++j) {
			b.setbits(j);
			double db = double(b);

			double ref = da * db;
			if (reportTestCases && !isInRange<LnsType>(ref)) {
				std::cerr << da << " * " << db << " = " << ref << " which is not in range " << range(a) << '\n';
			}
			c    = a * b;
			cref = ref;
			//				std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
			//				std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
			if (c != cref) {
				if (c.isnan() && cref.isnan())
					continue;  // NaN non-equivalence
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportBinaryArithmeticError("FAIL", "*", a, b, c, cref);
				//					std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
				//					std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
			} else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			}
			if (nrOfFailedTestCases > 24)
				return nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
int VerifyDivision(bool reportTestCases) {
	constexpr size_t nbits = LnsType::nbits;
	// constexpr size_t rbits = LnsType::rbits;
	// constexpr Behavior behavior = LnsType::behavior;
	// using bt = typename LnsType::BlockType;
	constexpr size_t NR_ENCODINGS = (1ull << nbits);

	int     nrOfFailedTestCases = 0;
	bool    firstTime           = true;
	LnsType a{}, b{}, c{}, cref{};
	double  ref{};
	if (reportTestCases)
		a.debugConstexprParameters();
	for (size_t i = 0; i < NR_ENCODINGS; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 0; j < NR_ENCODINGS; ++j) {
			b.setbits(j);
			double db = double(b);
#if LNS_THROW_ARITHMETIC_EXCEPTION
			try {
				c   = a / b;
				ref = da / db;
			} catch (const lns_divide_by_zero& err) {
				if (b.iszero()) {
					// correctly caught divide by zero
					if (firstTime) {
						if (reportTestCases)
							std::cout << "Correctly caught divide by zero exception : " << err.what() << '\n';
						firstTime = false;
					}
					continue;
				} else {
					// Spurious throw with non-zero divisor: count and skip the
					// rest of this iteration -- c/ref/cref are stale and would
					// trigger a follow-up false-positive failure below.
					++nrOfFailedTestCases;
					if (reportTestCases)
						ReportBinaryArithmeticError("FAIL", "/", a, b, c, cref);
					if (nrOfFailedTestCases > 24)
						return nrOfFailedTestCases;
					continue;
				}
			}
#else
			c   = a / b;
			ref = da / db;
#endif
			if (reportTestCases && !isInRange<LnsType>(ref)) {
				std::cerr << da << " / " << db << " = " << ref << " which is not in range " << range(a) << '\n';
			}
			cref = ref;
			//				std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
			//				std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
			if (c != cref) {
				if (c.isnan() && cref.isnan())
					continue;  // NaN non-equivalence
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportBinaryArithmeticError("FAIL", "/", a, b, c, cref);
			} else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, c, ref);
			}
			if (nrOfFailedTestCases > 24)
				return nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

} } // namespace sw::universal

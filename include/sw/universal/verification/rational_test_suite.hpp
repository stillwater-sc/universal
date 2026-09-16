#pragma once
// rational_test_suite.hpp: verification suite for the rational number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>  // std::cerr, for the failure reports this file writes itself
#include <numeric>   // std::gcd, to skip the encodings that are not normalized
#include <type_traits>
#include <universal/verification/test_reporters.hpp>

// the rational number class, and its is_rational trait, are configured by the test that includes this

namespace sw { namespace universal {

	// A rational holds its numerator and denominator in a blockbinary (base2) or in a blockdigit
	// (base8, base10, base16). Component is the digit specializations' name for that field, and
	// their nbits is a compatibility alias for ndigits, so the two families enumerate different
	// spaces: 2^nbits bit patterns against radix^ndigits digit encodings (#1526).
	template<typename RationalType, typename = void>
	struct rational_has_digit_component : std::false_type {};
	template<typename RationalType>
	struct rational_has_digit_component<RationalType, std::void_t<typename RationalType::Component>> : std::true_type {};

	// the largest encoding count these sweeps will walk: 4096 per component is about 16.7M pairs
	constexpr unsigned long long exhaustive_encoding_limit = 4096ull;

	// radix^ndigits: how many encodings a digit component holds, sign apart. A configuration past
	// the limit answers one over it rather than the product, which would wrap -- 16^16 is 2^64, and
	// a span of 0 would satisfy the caller's static_assert and then sweep nothing.
	template<unsigned ndigits, unsigned radix>
	constexpr unsigned long long encoding_span() noexcept {
		unsigned long long span = 1ull;
		for (unsigned i = 0; i < ndigits; ++i) {
			if (span > exhaustive_encoding_limit / radix) return exhaustive_encoding_limit + 1ull;
			span *= radix;
		}
		return span;
	}

	// every normalized value of a base2 rational, round tripped through a double: a positive
	// denominator and no common factor, both fields in range. The other encodings do not round-trip
	// by construction -- 6/8 comes back as the 3/4 it equals, and a value needing a field of
	// 2^(nbits-1), like 1/128 in a rational<8>, has no normalized form -- so they are skipped rather
	// than counted as conversion failures (#1523).
	template<typename RationalType>
	int VerifyBinaryRationalAssignment(bool reportTestCases) {
		constexpr unsigned nbits = RationalType::nbits;
		static_assert(nbits <= 20, "rational state space is too large to exhaustively test with ValidateAssignment<rational>");

		constexpr unsigned NR_ENCODINGS = (1ull << nbits);
		constexpr int      half         = (1 << (nbits - 1));
		auto signedValue = [](unsigned bits) { return (bits & (half)) ? static_cast<int>(bits) - 2 * half : static_cast<int>(bits); };
		int nrOfFailedTestCases = 0;

		RationalType a{}, b{};
		for (unsigned numerator = 0; numerator < NR_ENCODINGS; ++numerator) {
			for (unsigned denominator = 0; denominator < NR_ENCODINGS; ++denominator) {
				const int nv = signedValue(numerator), dv = signedValue(denominator);
				if (dv <= 0) continue;                                    // not normalized, or a NaN encoding
				if (std::gcd(nv < 0 ? -nv : nv, dv) != 1) continue;       // not in lowest terms
				a.set(numerator, denominator);
				double da = double(a);
				b = da;
				if (a != b) {
					if (a.isnan() && b.isnan()) continue;
					++nrOfFailedTestCases;
					if (reportTestCases) ReportAssignmentError("FAIL", "=", da, b, a);
				}
				if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

	// every normalized encoding of a base8, base10 or base16 rational, round tripped through a
	// double. The component space is radix^ndigits digit vectors plus a sign, not 2^ndigits values
	// passed through set(), so this walks the raw encodings: it writes the digits with setdigit()
	// and hands the pair to the constructor that does not normalize. Skipped, as above, are the
	// encodings that cannot round-trip by construction: a zero denominator is the NaN encoding, a
	// pair with a common factor comes back in lowest terms, and negative zero comes back positive.
	template<typename RationalType>
	int VerifyDigitRationalAssignment(bool reportTestCases) {
		using Component = typename RationalType::Component;
		using Digit     = typename Component::Digit;
		constexpr unsigned ndigits = Component::ndigits;
		constexpr unsigned radix   = Component::radix;

		constexpr unsigned long long SPAN = encoding_span<ndigits, radix>();
		static_assert(SPAN <= exhaustive_encoding_limit, "rational state space is too large to exhaustively test with ValidateAssignment<rational>");

		auto encode = [](unsigned long long magnitude, bool negative) {
			Component c;
			c.clear();
			for (unsigned i = 0; i < ndigits; ++i) {
				c.setdigit(i, static_cast<Digit>(magnitude % radix));
				magnitude /= radix;
			}
			c.setsign(negative);
			return c;
		};

		int nrOfFailedTestCases = 0;

		RationalType b{};
		for (unsigned long long denominator = 1; denominator < SPAN; ++denominator) {
			const Component d = encode(denominator, false);
			for (unsigned long long magnitude = 0; magnitude < SPAN; ++magnitude) {
				if (std::gcd(magnitude, denominator) != 1ull) continue;   // not in lowest terms
				for (unsigned sign = 0; sign < 2u; ++sign) {
					if (sign == 1u && magnitude == 0ull) continue;        // negative zero
					const RationalType a(encode(magnitude, sign == 1u), d);
					double da = double(a);
					b = da;
					if (a != b) {
						if (a.isnan() && b.isnan()) continue;
						++nrOfFailedTestCases;
						if (reportTestCases) ReportAssignmentError("FAIL", "=", da, b, a);
					}
					if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
				}
			}
		}

		return nrOfFailedTestCases;
	}

	// set(n, d) must preserve the value it was handed: reducing by the gcd cannot change n / d, and
	// neither can moving the sign to the numerator. Every encoding with a non-zero denominator
	// qualifies, the signed minimum -2^(nbits-1) included.
	//
	// The assignment sweep cannot see this. It compares what set() produced against the round trip
	// of that value's own double, so when the reduction mangles the pair both sides carry the same
	// wrong value and the case passes: set(-8, 7) in a rational<4> answered -1/1, and -1/1 round
	// trips perfectly (#1525).
	template<typename RationalType>
	int VerifyBinaryRationalNormalization(bool reportTestCases) {
		constexpr unsigned nbits = RationalType::nbits;
		static_assert(nbits <= 20, "rational state space is too large to exhaustively test with ValidateNormalization<rational>");

		constexpr unsigned NR_ENCODINGS = (1ull << nbits);
		constexpr int      half         = (1 << (nbits - 1));
		auto signedValue = [](unsigned bits) { return (bits & (half)) ? static_cast<int>(bits) - 2 * half : static_cast<int>(bits); };
		int nrOfFailedTestCases = 0;

		RationalType a{};
		for (unsigned numerator = 0; numerator < NR_ENCODINGS; ++numerator) {
			for (unsigned denominator = 0; denominator < NR_ENCODINGS; ++denominator) {
				const int nv = signedValue(numerator), dv = signedValue(denominator);
				if (dv == 0) continue;                                    // the NaN encoding
				a.set(nv, dv);
				const double expected = static_cast<double>(nv) / static_cast<double>(dv);
				const double observed = double(a);
				if (observed != expected) {
					++nrOfFailedTestCases;
					if (reportTestCases)
						std::cerr << "FAIL: set(" << nv << ", " << dv << ") = " << observed
						          << " expected " << expected << '\n';
					if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
				}
			}
		}

		return nrOfFailedTestCases;
	}

	// the signed minimum -2^(nbits-1) is a usable numerator: the arithmetic that lands on it, the
	// reduction that starts from it, and the value that needs it all have to come out exact. Every
	// operator routes through normalize(), which used to take |n| in the field's own width (#1525).
	template<typename RationalType>
	int VerifyBinaryRationalSignedMinimum(bool reportTestCases) {
		constexpr unsigned  nbits        = RationalType::nbits;
		constexpr long long signedMin    = -(1ll << (nbits - 1));
		constexpr long long maxMagnitude = -(signedMin + 1);   // 2^(nbits-1) - 1
		int nrOfFailedTestCases = 0;

		auto check = [&](const RationalType& r, double expected, const char* what) {
			if (double(r) != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					std::cerr << "FAIL: " << what << " = " << double(r) << " expected " << expected << '\n';
			}
		};

		RationalType a{}, b{}, c{};
		a.set(signedMin + 1, 1);                               // -(2^(nbits-1) - 1) + -1
		b.set(-1, 1);
		c = a;
		c += b;
		check(c, double(signedMin), "arithmetic onto the signed minimum");

		a.set(signedMin, 2);                                   // reduce, starting from the minimum
		check(a, double(signedMin) / 2.0, "the signed minimum over 2");

		a.set(signedMin, maxMagnitude);                        // the pair that needs the minimum
		check(a, double(signedMin) / double(maxMagnitude), "the signed minimum over the maximum");

		b = double(signedMin) / double(maxMagnitude);          // and converting that value back
		check(b, double(signedMin) / double(maxMagnitude), "conversion onto the signed minimum");

		return nrOfFailedTestCases;
	}

	// the same value-preservation check for a digit-based rational. Its components are
	// sign-magnitude, so there is no signed minimum to negate and every magnitude the field holds is
	// usable; what is checked is that the reduction leaves the value alone.
	template<typename RationalType>
	int VerifyDigitRationalNormalization(bool reportTestCases) {
		using Component = typename RationalType::Component;
		constexpr unsigned ndigits = Component::ndigits;
		constexpr unsigned radix   = Component::radix;

		constexpr unsigned long long SPAN = encoding_span<ndigits, radix>();
		static_assert(SPAN <= exhaustive_encoding_limit, "rational state space is too large to exhaustively test with ValidateNormalization<rational>");

		int nrOfFailedTestCases = 0;

		RationalType a{};
		const long long limit = static_cast<long long>(SPAN) - 1;
		for (long long nv = -limit; nv <= limit; ++nv) {
			for (long long dv = -limit; dv <= limit; ++dv) {
				if (dv == 0) continue;                                    // the NaN encoding
				a.set(nv, dv);
				const double expected = static_cast<double>(nv) / static_cast<double>(dv);
				const double observed = double(a);
				if (observed != expected) {
					++nrOfFailedTestCases;
					if (reportTestCases)
						std::cerr << "FAIL: set(" << nv << ", " << dv << ") = " << observed
						          << " expected " << expected << '\n';
					if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
				}
			}
		}

		return nrOfFailedTestCases;
	}

	// set(n, d) preserves the value it was handed, in whatever base the type is
	template<typename RationalType, std::enable_if_t<is_rational<RationalType>, bool> = true >
	int ValidateNormalization(bool reportTestCases) {
		if constexpr (rational_has_digit_component<RationalType>::value) {
			return VerifyDigitRationalNormalization<RationalType>(reportTestCases);
		}
		else {
			return VerifyBinaryRationalNormalization<RationalType>(reportTestCases)
			     + VerifyBinaryRationalSignedMinimum<RationalType>(reportTestCases);
		}
	}

	// round trip every normalized encoding of a rational through a double, in whatever base it is
	template<typename RationalType, std::enable_if_t<is_rational<RationalType>, bool> = true >
	int ValidateAssignment(bool reportTestCases) {
		if constexpr (rational_has_digit_component<RationalType>::value) {
			return VerifyDigitRationalAssignment<RationalType>(reportTestCases);
		}
		else {
			return VerifyBinaryRationalAssignment<RationalType>(reportTestCases);
		}
	}

}}  // namespace sw::universal

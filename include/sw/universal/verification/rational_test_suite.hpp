#pragma once
// rational_test_suite.hpp: verification suite for the rational number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
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

	// radix^ndigits: how many encodings a digit component holds, sign apart
	template<unsigned ndigits, unsigned radix>
	constexpr unsigned long long encoding_span() noexcept {
		unsigned long long span = 1ull;
		for (unsigned i = 0; i < ndigits; ++i) span *= radix;
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
		static_assert(SPAN <= 4096ull, "rational state space is too large to exhaustively test with ValidateAssignment<rational>");

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

// ieee754.cpp : native IEEE-754 operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyFloatingPointScales(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		long long largestScale = std::numeric_limits<Real>::max_exponent - 1;
		Real r = sw::universal::ipow<Real>(static_cast<size_t>(largestScale));
		for (long long i = 0; i < largestScale + 1; ++i) {
			if (largestScale - i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL : " << std::setw(4) << largestScale - i << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}
		// this gets us to 1.0, next enumerate the negative scaled normals
		int smallestScale = std::numeric_limits<Real>::min_exponent - 1;
		for (int i = -1; i > smallestScale; --i) {
			if (i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL : " << std::setw(4) << i << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}
		// this gets us to the smallest normal, next enumerate the subnormals
		for (int i = 0; i < sw::universal::ieee754_parameter<Real>::fbits; ++i) {
			if (smallestScale - i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << std::setw(4) << (smallestScale - i) << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test functions for extract_fields.hpp

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyExtractFields(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		bool s{ false };
		uint64_t rawExp{ 0 };
		uint64_t rawFrac{ 0 };
		uint64_t bits{ 0 };

		constexpr int bias = ieee754_parameter<Real>::bias;
		constexpr int fbits = ieee754_parameter<Real>::fbits;

		// Test case 1: +1.0 should have s=0, exp=bias, frac=0
		{
			Real value = Real(1.0);
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != static_cast<uint64_t>(bias) || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(+1.0): s=" << s << " exp=" << rawExp << " (expected " << bias << ") frac=" << rawFrac << '\n';
			}
		}

		// Test case 2: -1.0 should have s=1, exp=bias, frac=0
		{
			Real value = Real(-1.0);
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != true || rawExp != static_cast<uint64_t>(bias) || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(-1.0): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		// Test case 3: +2.0 should have s=0, exp=bias+1, frac=0
		{
			Real value = Real(2.0);
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != static_cast<uint64_t>(bias + 1) || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(+2.0): s=" << s << " exp=" << rawExp << " (expected " << (bias + 1) << ") frac=" << rawFrac << '\n';
			}
		}

		// Test case 4: +0.5 should have s=0, exp=bias-1, frac=0
		{
			Real value = Real(0.5);
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != static_cast<uint64_t>(bias - 1) || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(+0.5): s=" << s << " exp=" << rawExp << " (expected " << (bias - 1) << ") frac=" << rawFrac << '\n';
			}
		}

		// Test case 5: +0.0 should have s=0, exp=0, frac=0
		{
			Real value = Real(0.0);
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != 0 || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(+0.0): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		// Test case 6: -0.0 should have s=1, exp=0, frac=0
		{
			Real value = Real(-0.0);
			extractFields(value, s, rawExp, rawFrac, bits);
			// Note: -0.0 may or may not preserve sign depending on context
			if (rawExp != 0 || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(-0.0): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		// Test case 7: +infinity should have s=0, exp=all-ones, frac=0
		{
			Real value = std::numeric_limits<Real>::infinity();
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != ieee754_parameter<Real>::eallset || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(+inf): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		// Test case 8: -infinity should have s=1, exp=all-ones, frac=0
		{
			Real value = -std::numeric_limits<Real>::infinity();
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != true || rawExp != ieee754_parameter<Real>::eallset || rawFrac != 0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(-inf): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		// Test case 9: 1.5 should have s=0, exp=bias, frac=msb set (0.5 in binary)
		{
			Real value = Real(1.5);
			extractFields(value, s, rawExp, rawFrac, bits);
			uint64_t expectedFrac = uint64_t(1) << (fbits - 1);  // 0.1 in binary = msb of fraction
			if (s != false || rawExp != static_cast<uint64_t>(bias) || rawFrac != expectedFrac) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(1.5): s=" << s << " exp=" << rawExp << " frac=" << std::hex << rawFrac << " (expected " << expectedFrac << ")" << std::dec << '\n';
			}
		}

		// Test case 10: smallest subnormal (exp=0, frac=1)
		{
			Real value = std::numeric_limits<Real>::denorm_min();
			extractFields(value, s, rawExp, rawFrac, bits);
			if (s != false || rawExp != 0 || rawFrac != 1) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: extractFields(denorm_min): s=" << s << " exp=" << rawExp << " frac=" << rawFrac << '\n';
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test functions for set_fields.hpp

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifySetFields(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		constexpr int bias = ieee754_parameter<Real>::bias;
		constexpr int fbits = ieee754_parameter<Real>::fbits;

		Real value{};

		// Test case 1: Construct +1.0 (s=0, exp=bias, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(bias), static_cast<uint32_t>(0));
			} else {
				setFields(value, false, static_cast<uint64_t>(bias), static_cast<uint64_t>(0));
			}
			if (value != Real(1.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(+1.0): got " << value << " expected 1.0\n";
			}
		}

		// Test case 2: Construct -1.0 (s=1, exp=bias, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, true, static_cast<uint32_t>(bias), static_cast<uint32_t>(0));
			} else {
				setFields(value, true, static_cast<uint64_t>(bias), static_cast<uint64_t>(0));
			}
			if (value != Real(-1.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(-1.0): got " << value << " expected -1.0\n";
			}
		}

		// Test case 3: Construct +2.0 (s=0, exp=bias+1, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(bias + 1), static_cast<uint32_t>(0));
			} else {
				setFields(value, false, static_cast<uint64_t>(bias + 1), static_cast<uint64_t>(0));
			}
			if (value != Real(2.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(+2.0): got " << value << " expected 2.0\n";
			}
		}

		// Test case 4: Construct +0.5 (s=0, exp=bias-1, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(bias - 1), static_cast<uint32_t>(0));
			} else {
				setFields(value, false, static_cast<uint64_t>(bias - 1), static_cast<uint64_t>(0));
			}
			if (value != Real(0.5)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(+0.5): got " << value << " expected 0.5\n";
			}
		}

		// Test case 5: Construct +0.0 (s=0, exp=0, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(0), static_cast<uint32_t>(0));
			} else {
				setFields(value, false, static_cast<uint64_t>(0), static_cast<uint64_t>(0));
			}
			if (value != Real(0.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(+0.0): got " << value << " expected 0.0\n";
			}
		}

		// Test case 6: Construct +infinity (s=0, exp=all-ones, frac=0)
		{
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(0xFF), static_cast<uint32_t>(0));
			} else {
				setFields(value, false, static_cast<uint64_t>(0x7FF), static_cast<uint64_t>(0));
			}
			if (value != std::numeric_limits<Real>::infinity()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(+inf): got " << value << " expected inf\n";
			}
		}

		// Test case 7: Construct 1.5 (s=0, exp=bias, frac=msb set)
		{
			uint64_t frac = uint64_t(1) << (fbits - 1);  // 0.1 in binary
			if constexpr (sizeof(Real) == 4) {
				setFields(value, false, static_cast<uint32_t>(bias), static_cast<uint32_t>(frac));
			} else {
				setFields(value, false, static_cast<uint64_t>(bias), static_cast<uint64_t>(frac));
			}
			if (value != Real(1.5)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setFields(1.5): got " << value << " expected 1.5\n";
			}
		}

		return nrOfFailedTests;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifySetBit(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		constexpr int signBitIndex = (sizeof(Real) == 4) ? 31 : 63;

		// Test case 1: Start with +1.0, set sign bit to make -1.0
		{
			Real value = Real(1.0);
			setbit(value, signBitIndex, true);
			if (value != Real(-1.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setbit(1.0, sign=true): got " << value << " expected -1.0\n";
			}
		}

		// Test case 2: Start with -1.0, clear sign bit to make +1.0
		{
			Real value = Real(-1.0);
			setbit(value, signBitIndex, false);
			if (value != Real(1.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setbit(-1.0, sign=false): got " << value << " expected 1.0\n";
			}
		}

		// Test case 3: Start with +1.0, set fraction msb to make 1.5
		{
			Real value = Real(1.0);
			constexpr int fracMsbIndex = (sizeof(Real) == 4) ? 22 : 51;  // MSB of fraction field
			setbit(value, fracMsbIndex, true);
			if (value != Real(1.5)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setbit(1.0, frac_msb=true): got " << value << " expected 1.5\n";
			}
		}

		// Test case 4: Start with 1.5, clear fraction msb to make 1.0
		{
			Real value = Real(1.5);
			constexpr int fracMsbIndex = (sizeof(Real) == 4) ? 22 : 51;  // MSB of fraction field
			setbit(value, fracMsbIndex, false);
			if (value != Real(1.0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setbit(1.5, frac_msb=false): got " << value << " expected 1.0\n";
			}
		}

		// Test case 5: Start with 0.0, set exponent bit to create a power of 2
		{
			Real value = Real(0.0);
			// For float: exp bits are 30-23, for double: 62-52
			// Set exp bit corresponding to bias to get 1.0
			constexpr int expBitIndex = (sizeof(Real) == 4) ? 23 : 52;  // LSB of exponent (bit 0 of exp)
			// Setting bits to form bias: for float bias=127=0111'1111, for double bias=1023=011'1111'1111
			// First set the fraction MSB to avoid zero mantissa issues
			// Actually, let's test setting LSB of exponent on 0.0
			// 0.0 = 0.00000000.00000000000000000000000
			// Setting exp LSB gives: 0.00000001.00000000000000000000000 = 2^(1-127) = 2^-126 = min normal
			setbit(value, expBitIndex, true);
			Real expected = (sizeof(Real) == 4) ? std::numeric_limits<float>::min() : std::numeric_limits<double>::min();
			if (value != expected) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: setbit(0.0, exp_lsb=true): got " << value << " expected " << expected << '\n';
			}
		}

		return nrOfFailedTests;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyCheckNaN(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		int nan_type = NAN_TYPE_NEITHER;

		// Test case 1: Regular number should not be NaN
		{
			Real value = Real(1.0);
			bool isNaN = checkNaN(value, nan_type);
			if (isNaN || nan_type != NAN_TYPE_NEITHER) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkNaN(1.0): isNaN=" << isNaN << " type=" << nan_type << '\n';
			}
		}

		// Test case 2: Zero should not be NaN
		{
			Real value = Real(0.0);
			bool isNaN = checkNaN(value, nan_type);
			if (isNaN || nan_type != NAN_TYPE_NEITHER) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkNaN(0.0): isNaN=" << isNaN << " type=" << nan_type << '\n';
			}
		}

		// Test case 3: Infinity should not be NaN
		{
			Real value = std::numeric_limits<Real>::infinity();
			bool isNaN = checkNaN(value, nan_type);
			if (isNaN) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkNaN(inf): isNaN=" << isNaN << " type=" << nan_type << '\n';
			}
		}

		// Test case 4: quiet_NaN should be detected as NaN
		{
			Real value = std::numeric_limits<Real>::quiet_NaN();
			bool isNaN = checkNaN(value, nan_type);
			if (!isNaN || nan_type != NAN_TYPE_QUIET) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkNaN(quiet_NaN): isNaN=" << isNaN << " type=" << nan_type << " (expected NAN_TYPE_QUIET=" << NAN_TYPE_QUIET << ")\n";
			}
		}

		// Test case 5: signaling_NaN should be detected as NaN
		// Note: Some compilers/platforms may convert sNaN to qNaN
		{
			Real value = std::numeric_limits<Real>::signaling_NaN();
			bool isNaN = checkNaN(value, nan_type);
			if (!isNaN) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkNaN(signaling_NaN): isNaN=" << isNaN << " type=" << nan_type << '\n';
			}
			// Note: We don't strictly check for NAN_TYPE_SIGNALLING as some platforms convert sNaN to qNaN
		}

		return nrOfFailedTests;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyCheckInf(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		int inf_type = INF_TYPE_NEITHER;

		// Test case 1: Regular number should not be infinity
		{
			Real value = Real(1.0);
			bool isInf = checkInf(value, inf_type);
			if (isInf || inf_type != INF_TYPE_NEITHER) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkInf(1.0): isInf=" << isInf << " type=" << inf_type << '\n';
			}
		}

		// Test case 2: Zero should not be infinity
		{
			Real value = Real(0.0);
			bool isInf = checkInf(value, inf_type);
			if (isInf || inf_type != INF_TYPE_NEITHER) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkInf(0.0): isInf=" << isInf << " type=" << inf_type << '\n';
			}
		}

		// Test case 3: NaN should not be infinity
		{
			Real value = std::numeric_limits<Real>::quiet_NaN();
			bool isInf = checkInf(value, inf_type);
			if (isInf) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkInf(NaN): isInf=" << isInf << " type=" << inf_type << '\n';
			}
		}

		// Test case 4: +infinity should be detected
		{
			Real value = std::numeric_limits<Real>::infinity();
			bool isInf = checkInf(value, inf_type);
			if (!isInf || inf_type != INF_TYPE_POSITIVE) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkInf(+inf): isInf=" << isInf << " type=" << inf_type << " (expected INF_TYPE_POSITIVE=" << INF_TYPE_POSITIVE << ")\n";
			}
		}

		// Test case 5: -infinity should be detected
		{
			Real value = -std::numeric_limits<Real>::infinity();
			bool isInf = checkInf(value, inf_type);
			if (!isInf || inf_type != INF_TYPE_NEGATIVE) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: checkInf(-inf): isInf=" << isInf << " type=" << inf_type << " (expected INF_TYPE_NEGATIVE=" << INF_TYPE_NEGATIVE << ")\n";
			}
		}

		return nrOfFailedTests;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyFieldRoundTrip(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		// Test extracting fields and using them to reconstruct the value
		auto testRoundTrip = [&](Real original, const char* name) {
			bool s{ false };
			uint64_t rawExp{ 0 };
			uint64_t rawFrac{ 0 };
			uint64_t bits{ 0 };

			extractFields(original, s, rawExp, rawFrac, bits);

			Real reconstructed{};
			if constexpr (sizeof(Real) == 4) {
				setFields(reconstructed, s, static_cast<uint32_t>(rawExp), static_cast<uint32_t>(rawFrac));
			} else {
				setFields(reconstructed, s, rawExp, rawFrac);
			}

			// For NaN, we can't use == comparison
			if (std::isnan(original)) {
				if (!std::isnan(reconstructed)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: round-trip " << name << ": original is NaN but reconstructed is not\n";
				}
			}
			else if (original != reconstructed) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: round-trip " << name << ": original=" << original << " reconstructed=" << reconstructed << '\n';
			}
		};

		testRoundTrip(Real(1.0), "1.0");
		testRoundTrip(Real(-1.0), "-1.0");
		testRoundTrip(Real(0.0), "0.0");
		testRoundTrip(Real(2.0), "2.0");
		testRoundTrip(Real(0.5), "0.5");
		testRoundTrip(Real(1.5), "1.5");
		testRoundTrip(Real(3.14159265358979323846), "pi");
		testRoundTrip(Real(2.71828182845904523536), "e");
		testRoundTrip(std::numeric_limits<Real>::max(), "max");
		testRoundTrip(std::numeric_limits<Real>::min(), "min");
		testRoundTrip(std::numeric_limits<Real>::denorm_min(), "denorm_min");
		testRoundTrip(std::numeric_limits<Real>::infinity(), "+inf");
		testRoundTrip(-std::numeric_limits<Real>::infinity(), "-inf");
		testRoundTrip(std::numeric_limits<Real>::quiet_NaN(), "qNaN");

		return nrOfFailedTests;
	}

} } // namespace sw::universal

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void NativeEnvironment(Real r) {
	using namespace sw::universal;

	std::cout << "scale of " << r << " is 2^" << scale(r) << " ~ 10^" << int(scale(r) / 3.3) << '\n';
	std::cout << to_binary(r, true) << " " << r << '\n';
	std::cout << color_print(r) << " " << r << '\n';
}

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void DescendingScales() {
	std::string size("unknown");

	switch (sizeof(Real)) {
	case 4:
		size = "single";
		break;
	case 8:
		size = "double";
		break;
	case 16:
		size = "quadruple";
		break;
	}
	std::cout << "IEEE-754 " << size << " precision scales:             in descending order\n";

	auto oldPrecision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Real>::digits10);

	long long largestScale = std::numeric_limits<Real>::max_exponent - 1;
	Real r = sw::universal::ipow<double>(largestScale);
	for (long long i = 0; i < largestScale + 1; ++i) {
		std::cout << std::setw(4) << largestScale - i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to 1.0, next enumerate the negative scaled normals
	int smallestScale = std::numeric_limits<Real>::min_exponent - 1;
	for (int i = -1; i > smallestScale; --i) {
		std::cout << std::setw(4) << i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to the smallest normal, next enumerate the subnormals
	for (int i = 0; i < sw::universal::ieee754_parameter<Real>::fbits; ++i) {
		std::cout << std::setw(4) << (smallestScale - i) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	std::cout << std::setprecision(oldPrecision);
}

template<typename RealType,
	     typename = typename std::enable_if< std::is_floating_point<RealType>::value, RealType>::type>
void InfinityAdditions() {
	std::cout << "IEEE-754 addition with infinites\n";
	constexpr RealType fa = std::numeric_limits<RealType>::infinity();
	constexpr RealType fb = -fa;
	constexpr unsigned COLWITH = 15;
	std::cout << std::setw(COLWITH) << fa << " + " << std::setw(COLWITH) << fa << " = " << std::setw(COLWITH) << (fa + fa) << " : " << sw::universal::to_binary(fa + fa) << '\n';
	std::cout << std::setw(COLWITH) << fa << " + " << std::setw(COLWITH) << fb << " = " << std::setw(COLWITH) << (fa + fb) << " : " << sw::universal::to_binary(fa + fb) << '\n';
	std::cout << std::setw(COLWITH) << fb << " + " << std::setw(COLWITH) << fa << " = " << std::setw(COLWITH) << (fb + fa) << " : " << sw::universal::to_binary(fb + fa) << '\n';
	std::cout << std::setw(COLWITH) << fb << " + " << std::setw(COLWITH) << fb << " = " << std::setw(COLWITH) << (fb + fb) << " : " << sw::universal::to_binary(fb + fb) << '\n';
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

	std::string test_suite  = "IEEE-754 floating-point operators";
	std::string test_tag    = "special cases";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// compare bits of different real number representations
	
	float f         = 1.0e1;
	double d        = 1.0e10;
#if LONG_DOUBLE_SUPPORT
	long double ld  = 1.0e100;
#else
	std::cout << "This environment does not support a native long double format\n";
#endif

	NativeEnvironment(f);
	NativeEnvironment(d);
#if LONG_DOUBLE_SUPPORT
	NativeEnvironment(ld);
#endif

	// show all the different presentations for the different IEEE-754 native formats
	valueRepresentations(f);
	valueRepresentations(d);
#if LONG_DOUBLE_SUPPORT
	valueRepresentations(ld);
#endif

	// show the scales that an IEEE-754 type contains
	DescendingScales<float>();

	// show the results of addition with infinites
	InfinityAdditions<float>();

	int largestScale = std::numeric_limits<float>::max_exponent - 1;
	float r = sw::universal::ipow<float>(static_cast<size_t>(largestScale));
	std::cout << "largest scale  : " << largestScale << " value : " << r << '\n';
	int smallestScale = std::numeric_limits<float>::min_exponent - 1;
	r = sw::universal::ipow<float>(static_cast<size_t>(smallestScale));
	std::cout << "smallest scale : " << smallestScale << " value : " << r << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<double>(reportTestCases), "double", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// show the results of addition with infinites
	InfinityAdditions<float>();
	InfinityAdditions<double>();

	std::cout << "\nNative floating-point ranges\n";
	std::cout << float_range() << '\n';
	std::cout << double_range() << '\n';
	std::cout << longdouble_range() << '\n';

	std::cout << "\nTest cases\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<double>(reportTestCases), "double", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<long double>(reportTestCases), "long double", test_tag);
#endif

	// extract_fields.hpp tests
	std::cout << "\nExtract fields tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyExtractFields<float>(reportTestCases), "float", "extractFields");
	nrOfFailedTestCases += ReportTestResult(VerifyExtractFields<double>(reportTestCases), "double", "extractFields");

	// set_fields.hpp tests
	std::cout << "\nSet fields tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifySetFields<float>(reportTestCases), "float", "setFields");
	nrOfFailedTestCases += ReportTestResult(VerifySetFields<double>(reportTestCases), "double", "setFields");

	std::cout << "\nSetbit tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifySetBit<float>(reportTestCases), "float", "setbit");
	nrOfFailedTestCases += ReportTestResult(VerifySetBit<double>(reportTestCases), "double", "setbit");

	// NaN and Inf detection tests
	std::cout << "\nNaN detection tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyCheckNaN<float>(reportTestCases), "float", "checkNaN");
	nrOfFailedTestCases += ReportTestResult(VerifyCheckNaN<double>(reportTestCases), "double", "checkNaN");

	std::cout << "\nInfinity detection tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyCheckInf<float>(reportTestCases), "float", "checkInf");
	nrOfFailedTestCases += ReportTestResult(VerifyCheckInf<double>(reportTestCases), "double", "checkInf");

	// Round-trip tests
	std::cout << "\nField round-trip tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFieldRoundTrip<float>(reportTestCases), "float", "field round-trip");
	nrOfFailedTestCases += ReportTestResult(VerifyFieldRoundTrip<double>(reportTestCases), "double", "field round-trip");

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if	REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

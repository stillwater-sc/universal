//  api.cpp : test suite runner for the class interface of the simplified floating-point type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/internal/f2s/f2s.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

#define NOW_
#ifdef NOW

		// Provides a decimal representation of v.
		// Returns true if it succeeds, otherwise the result cannot be trusted.
		// There will be *length digits inside the buffer (not null-terminated).
		// If the function returns true then
		//        v == (double) (buffer * 10^decimal_exponent).
		// The digits in the buffer are the shortest representation possible: no
		// 0.09999999999999999 instead of 0.1. The shorter representation will even be
		// chosen even if the longer one would be closer to v.
		// The last digit will be closest to the actual v. That is, even if several
		// digits might correctly yield 'v' when read again, the closest will be
		// computed.
		bool Grisu3(double v, char buffer[], int& length, int& decimal_exponent) {
			using F2S = f2s<std::uint64_t>;
			F2S w;
			w.set(false, scale(v)-52, significant(v), ieee754_parameter<double>::fbits);
			// boundary_minus and boundary_plus are the boundaries between v and its
			// closest floating-point neighbors. Any number strictly between
			// boundary_minus and boundary_plus will round to v when converted to a double.
			// Grisu3 will never output representations that lie exactly on a boundary.
			F2S boundary_minus, boundary_plus;
			w.normalizedBoundaries(boundary_minus, boundary_plus);
			std::cout << to_triple(boundary_minus) << '\n';
			std::cout << to_triple(boundary_plus) << '\n';
			std::cout << to_triple(w) << '\n';			
			assert(boundary_plus.e() == w.e());

			buffer[0] = 0;
			length = 0;
			decimal_exponent = 0;
			bool result{ false };

			F2S ten_mk;  // Cached power of ten: 10^-k
			int mk;        // -k
			int precision = static_cast<int>(F2S::sizeOfSignificant);
			int ten_mk_minimal_binary_exponent = kMinimalTargetExponent - (w.e() + precision);
			int ten_mk_maximal_binary_exponent = kMaximalTargetExponent - (w.e() + precision);

			GetCachedPowerForBinaryExponentRange(ten_mk_minimal_binary_exponent, ten_mk_maximal_binary_exponent, ten_mk, mk);
			int bla = w.e() + ten_mk.e() + static_cast<int>(F2S::sizeOfSignificant);
			assert((kMinimalTargetExponent <= bla) && (kMaximalTargetExponent >= bla));

			std::cout << "w.e()               : " << w.e() << '\n';
			std::cout << "ten_mk.e()          : " << ten_mk.e() << '\n';
			std::cout << "mk                  : " << mk << '\n';
			std::cout << "sizeOfSignificant   : " << precision << '\n';
			std::cout << "min target exponent : " << kMinimalTargetExponent << '\n';
			std::cout << "bla                 : " << bla << '\n';
			std::cout << "max target exponent : " << kMaximalTargetExponent << '\n';
			// Note that ten_mk is only an approximation of 10^-k. A DiyFp only contains a
			// 64 bit significand and ten_mk is thus only precise up to 64 bits.

			// operator*() rounds its result, and ten_mk is thus approximate.
			// The variable scaled_w (as well as scaled_boundary_minus/plus) are off by a small amount.
			// In fact: scaled_w - w*10^k < 1ulp (unit in the last place) of scaled_w.
			// In other words: let f = scaled_w.f() and e = scaled_w.e(), then
			//           (f-1) * 2^e < w*10^k < (f+1) * 2^e
			//F2S scaled_w = (w * ten_mk);
			//assert(scaled_w.e() == boundary_plus.e() + ten_mk.e() + precision);
			// In theory it would be possible to avoid some recomputations by computing
			// the difference between w and boundary_minus/plus (a power of 2) and to
			// compute scaled_boundary_minus/plus by subtracting/adding from
			// scaled_w. However the code becomes much less readable and the speed
			// enhancements are not terrific.
			//F2S scaled_boundary_minus = boundary_minus * ten_mk;
			//F2S scaled_boundary_plus = boundary_plus * ten_mk;

			// DigitGen will generate the digits of scaled_w. Therefore we have
			// v == (double) (scaled_w * 10^-mk).
			// Set decimal_exponent == -mk and pass it to DigitGen. If scaled_w is not an
			// integer than it will be updated. For instance if scaled_w == 1.23 then
			// the buffer will be filled with "123" and the decimal_exponent will be
			// decreased by 2.
#ifdef LATER
			int kappa;
			bool result = DigitGen(scaled_boundary_minus, scaled_w, scaled_boundary_plus,
				buffer, length, &kappa);
			*decimal_exponent = -mk + kappa;
#endif
			return result;
		}
#else
bool Grisu3(double v, char buffer[], int& length, int& decimal_exponent) { return false; }
#endif
	} // namespace universal
} // namespace sw

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "f2s API validation";
	std::string test_tag    = "API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////         construction

#ifdef LATER
	{
		f2s<uint32_t> a, b, c;
		a = 1.0e0f;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0f;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		f2s<uint64_t> a, b, c;
		a = 1.0e0;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}
	{
		float f{ 1024*2 };
		std::cout << to_binary(f) << " : " << f << " : " << scale(f) << '\n';
	}



	
	{
		half f{ 0.03125 };
		std::cout << "floating-point value : " << to_binary(f) << " : " << f << " : " << to_triple(f) << '\n';

		duble d{ 0.0312 };
		std::cout << "floating-point value : " << to_binary(d) << " : " << d << " : " << to_triple(d) << '\n';
	}

	{
		f2s<uint64_t> a;
		// print out the cached powers
		constexpr unsigned ub = sizeof(CachedPowers) / sizeof(CachedPowers[0]);
		for (int mk = 0; mk < ub; ++mk) {
			CachedPower cp = CachedPowers[mk];
			std::cout << mk << " : " << to_binary(cp.significand, 64, true) << " : ";
			a.set(false, cp.binary_exponent, cp.significand);
			std::cout << double(a) << '\n';

		}
	}
#endif

	{
		/*
			ten_mk  {f_=0x9c40000000000000 e_=0xffffffce }
			a       {f_=0x8000000000000000 e_=0xffffffc1 }

			boundary_minus  0x0000006290efe828 {f_=0x7ffffffffffffe00 e_=0xffffffc1 }
			boundary_plus   0x0000006290efe858 {f_=0x8000000000000400 e_=0xffffffc1 }
		*/
		std::cout << "normalizedBoundaries around 1.0\n";
		f2s<uint64_t> a, a_minus, a_plus;
		a = 1.0e0;
		std::cout << to_triple(a) << '\n';

		a.normalizedBoundaries(a_minus, a_plus);
		std::cout << to_triple(a_minus) << '\n';
		std::cout << to_triple(a_plus) << '\n';

		a.normalize();
		std::cout << to_triple(a) << '\n';
	}

	{

		std::cout << "grisu3\n";
		char buffer[128];
		int nrOfDigits{ 0 };
		int decimalExponent{ 0 };
		bool success = Grisu3(1.0, buffer, nrOfDigits, decimalExponent);
		if (success) {
			std::cout << std::string(buffer) << '\n';
		}

	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

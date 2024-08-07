// rounding.cpp: test suite runner for rounding of fixed-sized, arbitrary precision double-base logarithmic number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <algorithm>
#include <cmath>
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename DbnsType>
	struct KVpair {
		DbnsType dbns;
		double   d;
	};

	template<typename DbnsType>
	std::ostream& operator<<(std::ostream& ostr, KVpair<DbnsType>& p) {
		return ostr << to_binary(p.dbns) << ", " << p.d;
	}

	template<typename DbnsType>
	void generateOrderedSet(std::vector<KVpair<DbnsType>>& ordered) {
		constexpr unsigned nbits = DbnsType::nbits;
		unsigned NR_ENCODINGS = (1u << nbits);
		KVpair<DbnsType> sample{0, 0};
		DbnsType dbns{ 0 };
		for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
			dbns.setbits(i);
			if (!dbns.isnan()) {
				sample.dbns = dbns;
				sample.d = double(dbns);
				ordered.push_back(sample);
			}
		}

		struct {
			bool operator()(const KVpair<DbnsType>& a, const KVpair<DbnsType>& b) const {
				return a.d < b.d;
			}
		} KVpairCompare;
		std::sort(ordered.begin(), ordered.end(), KVpairCompare);
	}

	void create5_2() {
		using DBNS5_2_sat = dbns<5, 2, uint8_t, Behavior::Saturating>;
		std::vector<KVpair<DBNS5_2_sat>> ordered;
		generateOrderedSet<DBNS5_2_sat>(ordered);
		for (auto p : ordered) {
			std::cout << p << '\n';
		}
	}
	void create7_3() {
		using DBNS7_3_sat = dbns<7, 3, uint8_t, Behavior::Saturating>;
		std::vector<KVpair<DBNS7_3_sat>> ordered;
		generateOrderedSet<DBNS7_3_sat>(ordered);
		for (auto p : ordered) {
			std::cout << p << '\n';
		}

	}
	template<typename DbnsType, typename Real>
	DbnsType convert_ieee754(Real v) noexcept {
		using std::abs;
		using std::log2;
		using std::pow;
		using std::round;

		DbnsType doubleBaseNumber{ 0 };
		typedef typename DbnsType::BlockType BlockType;
		//constexpr unsigned nbits   = DbnsType::nbits;
		//constexpr unsigned fbbits  = DbnsType::fbbits;
		//constexpr unsigned sbbits  = DbnsType::sbbits;
		//constexpr uint64_t FB_MASK = DbnsType::FB_MASK;
		constexpr uint64_t SB_MASK = DbnsType::SB_MASK;
		constexpr uint64_t MAX_A   = DbnsType::MAX_A;
		constexpr uint64_t MAX_B   = DbnsType::MAX_B;
		constexpr double   log2of3 = 1.5849625007211561814537389439478;

		bool s{ false };
		uint64_t unbiasedExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(v, s, unbiasedExponent, rawFraction, bits);
		if (unbiasedExponent == ieee754_parameter<Real>::eallset) { // nan and inf need to be remapped
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::snanmask) ||
				rawFraction == (ieee754_parameter<Real>::fmask & (ieee754_parameter<Real>::qnanmask | ieee754_parameter<Real>::snanmask))) {
				// 1.11111111.00000000.......00000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000.......00000001 signalling nan
				// 0.11111111.10000000.......00000001 signalling nan
				doubleBaseNumber.setnan();
				return doubleBaseNumber;
			}
			if (rawFraction == (ieee754_parameter<Real>::fmask & ieee754_parameter<Real>::qnanmask)) {
				// 1.11111111.10000000.......00000000 quiet nan
				// 0.11111111.10000000.......00000000 quiet nan
				doubleBaseNumber.setnan();
				//setsign(s);  a cfloat encodes a signalling nan with sign = 1, and a quiet nan with sign = 0
				return doubleBaseNumber;
			}
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				doubleBaseNumber.setinf(s);
				return doubleBaseNumber;
			}
		}
		if (v == 0.0) {
			doubleBaseNumber.setzero();
			return doubleBaseNumber;
		}

		// it is too expensive to check if the value is in the representable range
		// the search below will end up at 0 or maxpos

		// we search for the a and b in v = 2^a * 3^b, with both a and b positive
		// in our representation we have 0.5^a * 3^b, which would be equivalent
		// to a being negative
		// 
		// v = 2^a * 3^b =>
		// v = 2^(a + b*log2of3) =>
		// scale of v = (a + b*log2of3)
		// we use this relationship to search among the second base exponents 
		// and find a first base exponent that minimizes the error
		// between the result and the value we are trying to approximate.
		constexpr bool bDebug = true;
		double scale = log2(abs(v));
		if constexpr (bDebug) std::cout << "input value : " << v << " scale : " << scale << '\n';
		double lowestError = 1.0e10;
		int best_a = 500;
		int best_b = 500;
		for (int b = 0; b <= static_cast<int>(SB_MASK); ++b) {
			int a = static_cast<int>(round((scale - b * log2of3))); // find the first base exponent that is closest to the value
			if (a > 0 || a > static_cast<int>(MAX_A)) continue;
			double err = abs(scale - (a + b * log2of3));
			if constexpr (bDebug) {
				double fb = pow(2.0, a);
				double sb = pow(3.0, b);
				double value = fb * sb;
				std::cout << "a : " <<std::setw(3) << a << " b : " << std::setw(3) << b << " err : " << std::setw(12) << err << " fb : " << std::setw(10) << fb << " sb : " << std::setw(10) << sb << " value : " << std::setw(10) << value;
			}
			if (err < lowestError) {
				if constexpr (bDebug) std::cout << " ACCEPTED\n";
				lowestError = err;
				best_a = a;
				best_b = b;
			}
			else {
				if constexpr (bDebug) std::cout << "                 REJECTED\n";
			}
		}
		if constexpr (bDebug) std::cout << "best a : " << best_a << " best b : " << best_b << " lowest err : " << lowestError << '\n';
		assert(best_b >= 0); // second exponent is negative
		int a = -best_a;
		int b = best_b;
		if (a < 0 || a > static_cast<int>(MAX_A) || b > static_cast<int>(MAX_B)) {
			// try to project the value back into valid pairs
			// the approximations of unity looks like (8,-5), (19,-12), (84,-53),... 
			// they grow too fast and in a rather irregular manner. There are more 
			// subtle number theoretic considerations, but the ones outlined above 
			// should be sufficient to figure out a good solution to the problem.
			// 2^3*3^-2 = 0.888  2^-3*3^2 = 1.125
			// 2^8*3^-5 = 1.053  2^-8*3^5 = 0.949
			// multiplier   0.5, 1.5, 0.6, 0.889, 1.125, 0.949, 1.053.....
			int first[] =  { 1, 1, -1, 3, -3, 5, -5, 8, -8, 19, -19, 84, -84 };
			int second[] = { 0, 1, -1, 2, -2, 3, -3, 5, -5, 12, -12, 53, -53 };
			bool unableToAdjust{ true };
			for (unsigned i = 0; i < 13; ++i) {
				int adjusted_a = a - first[i];
				int adjusted_b = b - second[i];
				if (adjusted_a >= 0 && adjusted_a < static_cast<int>(MAX_A) && adjusted_b >= 0 && adjusted_b < static_cast<int>(MAX_B)) {
					doubleBaseNumber.setexponent(0, static_cast<unsigned>(adjusted_a));
					doubleBaseNumber.setexponent(1, static_cast<unsigned>(adjusted_b));
					doubleBaseNumber.setsign(s);
					unableToAdjust = false;
					break;
				}
			}
			if (unableToAdjust) {
				//if (a > b) {
				if (best_a < 0 && best_b >= 0) {
					doubleBaseNumber.setexponent(0, MAX_A);
					doubleBaseNumber.setexponent(1, 0);
					doubleBaseNumber.setsign(false); // we need to avoid nan(ind)
				}
				else {   // we have maxed out
					doubleBaseNumber.setexponent(0, 0);
					doubleBaseNumber.setexponent(1, MAX_B);
					doubleBaseNumber.setsign(s);
				}
			}
		}
		else {
			doubleBaseNumber.setexponent(0, static_cast<BlockType>(a));
			doubleBaseNumber.setexponent(1, static_cast<BlockType>(b));
			doubleBaseNumber.setsign(s);

			//_block[MSU] = static_cast<bt>(static_cast<bt>(s ? SIGN_BIT_MASK : 0u) | static_cast<bt>(a) | static_cast<bt>(b));
		}
		// avoid assigning to nan(ind)
		if (doubleBaseNumber.isnan()) doubleBaseNumber.setzero();
		return doubleBaseNumber;
	}

} }

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "dbns rounding validation";
	std::string test_tag    = "rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// GenerateDbnsTable<4, 1>(std::cout);
	// GenerateDbnsTable<5, 2>(std::cout);
	// GenerateDbnsTable<7, 3>(std::cout);

	using DBNS5_2_sat = dbns<5, 2, uint8_t, Behavior::Saturating>;
	//using DBNS7_3_sat = dbns<7, 3, uint8_t, Behavior::Saturating>;

	float f = 4.5 * 3.375;
	DBNS5_2_sat d{ f }, d2{ 0 };
	d2 = convert_ieee754<DBNS5_2_sat>(f);
	std::cout << std::setw(10) << f << " : " << to_binary(d) << " : " << to_binary(d2) << '\n';
	return 0;

	for (unsigned i = 0; i < 32; ++i) {
		d.setbits(i);
		f = float(d);
		d2 = convert_ieee754<DBNS5_2_sat>(f);
		std::cout << std::setw(10) << f << " : " << to_binary(d) << " : " << to_binary(d2) << '\n';
		if (d == d2) std::cout << "   PASS\n"; else std::cout << "   FAIL\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	
#endif

#if REGRESSION_LEVEL_2
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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



/*

  Generate Value table for an DBNS<4,1> in TXT format
   #           Binary    sign   scale                         value          format
   0:         0b0.0.00       0       0                             1                1
   1:         0b0.0.01       0       1                             3                3
   2:         0b0.0.10       0       3                             9                9
   3:         0b0.0.11       0       4                            27               27
   4:         0b0.1.00       0       1                             0                0
   5:         0b0.1.01       0       2                           1.5              1.5
   6:         0b0.1.10       0       4                           4.5              4.5
   7:         0b0.1.11       0       5                          13.5             13.5
   8:         0b1.0.00       1       0                            -1               -1
   9:         0b1.0.01       1       1                            -3               -3
  10:         0b1.0.10       1       3                            -9               -9
  11:         0b1.0.11       1       4                           -27              -27
  12:         0b1.1.00       1       1                     -nan(ind)        -nan(ind)
  13:         0b1.1.01       1       2                          -1.5             -1.5
  14:         0b1.1.10       1       4                          -4.5             -4.5
  15:         0b1.1.11       1       5                         -13.5            -13.5

Generate Value table for an DBNS<5,2> in TXT format
   #           Binary    sign   scale                         value          format
   0:        0b0.00.00       0       0                             1                1
   1:        0b0.00.01       0       1                             3                3
   2:        0b0.00.10       0       3                             9                9
   3:        0b0.00.11       0       4                            27               27
   4:        0b0.01.00       0       1                           0.5              0.5
   5:        0b0.01.01       0       2                           1.5              1.5
   6:        0b0.01.10       0       4                           4.5              4.5
   7:        0b0.01.11       0       5                          13.5             13.5
   8:        0b0.10.00       0       2                          0.25             0.25
   9:        0b0.10.01       0       3                          0.75             0.75
  10:        0b0.10.10       0       5                          2.25             2.25
  11:        0b0.10.11       0       6                          6.75             6.75
  12:        0b0.11.00       0       3                             0                0
  13:        0b0.11.01       0       4                         0.375            0.375
  14:        0b0.11.10       0       6                         1.125            1.125
  15:        0b0.11.11       0       7                         3.375            3.375
  16:        0b1.00.00       1       0                            -1               -1
  17:        0b1.00.01       1       1                            -3               -3
  18:        0b1.00.10       1       3                            -9               -9
  19:        0b1.00.11       1       4                           -27              -27
  20:        0b1.01.00       1       1                          -0.5             -0.5
  21:        0b1.01.01       1       2                          -1.5             -1.5
  22:        0b1.01.10       1       4                          -4.5             -4.5
  23:        0b1.01.11       1       5                         -13.5            -13.5
  24:        0b1.10.00       1       2                         -0.25            -0.25
  25:        0b1.10.01       1       3                         -0.75            -0.75
  26:        0b1.10.10       1       5                         -2.25            -2.25
  27:        0b1.10.11       1       6                         -6.75            -6.75
  28:        0b1.11.00       1       3                     -nan(ind)        -nan(ind)
  29:        0b1.11.01       1       4                        -0.375           -0.375
  30:        0b1.11.10       1       6                        -1.125           -1.125
  31:        0b1.11.11       1       7                        -3.375           -3.375

  Generate Value table for an DBNS<7,3> in TXT format
   #           Binary    sign   scale                         value          format
   0:      0b0.000.000       0       0                             1                1
   1:      0b0.000.001       0       1                             3                3
   2:      0b0.000.010       0       3                             9                9
   3:      0b0.000.011       0       4                            27               27
   4:      0b0.000.100       0       6                            81               81
   5:      0b0.000.101       0       7                           243              243
   6:      0b0.000.110       0       9                           729              729
   7:      0b0.000.111       0      11                          2187             2187
   8:      0b0.001.000       0       1                           0.5              0.5
   9:      0b0.001.001       0       2                           1.5              1.5
  10:      0b0.001.010       0       4                           4.5              4.5
  11:      0b0.001.011       0       5                          13.5             13.5
  12:      0b0.001.100       0       7                          40.5             40.5
  13:      0b0.001.101       0       8                         121.5            121.5
  14:      0b0.001.110       0      10                         364.5            364.5
  15:      0b0.001.111       0      12                        1093.5           1093.5
  16:      0b0.010.000       0       2                          0.25             0.25
  17:      0b0.010.001       0       3                          0.75             0.75
  18:      0b0.010.010       0       5                          2.25             2.25
  19:      0b0.010.011       0       6                          6.75             6.75
  20:      0b0.010.100       0       8                         20.25            20.25
  21:      0b0.010.101       0       9                         60.75            60.75
  22:      0b0.010.110       0      11                        182.25           182.25
  23:      0b0.010.111       0      13                        546.75           546.75
  24:      0b0.011.000       0       3                         0.125            0.125
  25:      0b0.011.001       0       4                         0.375            0.375
  26:      0b0.011.010       0       6                         1.125            1.125
  27:      0b0.011.011       0       7                         3.375            3.375
  28:      0b0.011.100       0       9                        10.125           10.125
  29:      0b0.011.101       0      10                        30.375           30.375
  30:      0b0.011.110       0      12                        91.125           91.125
  31:      0b0.011.111       0      14                       273.375          273.375
  32:      0b0.100.000       0       4                        0.0625           0.0625
  33:      0b0.100.001       0       5                        0.1875           0.1875
  34:      0b0.100.010       0       7                        0.5625           0.5625
  35:      0b0.100.011       0       8                        1.6875           1.6875
  36:      0b0.100.100       0      10                        5.0625           5.0625
  37:      0b0.100.101       0      11                       15.1875          15.1875
  38:      0b0.100.110       0      13                       45.5625          45.5625
  39:      0b0.100.111       0      15                       136.688          136.688
  40:      0b0.101.000       0       5                       0.03125          0.03125
  41:      0b0.101.001       0       6                       0.09375          0.09375
  42:      0b0.101.010       0       8                       0.28125          0.28125
  43:      0b0.101.011       0       9                       0.84375          0.84375
  44:      0b0.101.100       0      11                       2.53125          2.53125
  45:      0b0.101.101       0      12                       7.59375          7.59375
  46:      0b0.101.110       0      14                       22.7812          22.7812
  47:      0b0.101.111       0      16                       68.3438          68.3438
  48:      0b0.110.000       0       6                      0.015625         0.015625
  49:      0b0.110.001       0       7                      0.046875         0.046875
  50:      0b0.110.010       0       9                      0.140625         0.140625
  51:      0b0.110.011       0      10                      0.421875         0.421875
  52:      0b0.110.100       0      12                       1.26562          1.26562
  53:      0b0.110.101       0      13                       3.79688          3.79688
  54:      0b0.110.110       0      15                       11.3906          11.3906
  55:      0b0.110.111       0      17                       34.1719          34.1719
  56:      0b0.111.000       0       7                             0                0
  57:      0b0.111.001       0       8                     0.0234375        0.0234375
  58:      0b0.111.010       0      10                     0.0703125        0.0703125
  59:      0b0.111.011       0      11                      0.210938         0.210938
  60:      0b0.111.100       0      13                      0.632812         0.632812
  61:      0b0.111.101       0      14                       1.89844          1.89844
  62:      0b0.111.110       0      16                       5.69531          5.69531
  63:      0b0.111.111       0      18                       17.0859          17.0859
  64:      0b1.000.000       1       0                            -1               -1
  65:      0b1.000.001       1       1                            -3               -3
  66:      0b1.000.010       1       3                            -9               -9
  67:      0b1.000.011       1       4                           -27              -27
  68:      0b1.000.100       1       6                           -81              -81
  69:      0b1.000.101       1       7                          -243             -243
  70:      0b1.000.110       1       9                          -729             -729
  71:      0b1.000.111       1      11                         -2187            -2187
  72:      0b1.001.000       1       1                          -0.5             -0.5
  73:      0b1.001.001       1       2                          -1.5             -1.5
  74:      0b1.001.010       1       4                          -4.5             -4.5
  75:      0b1.001.011       1       5                         -13.5            -13.5
  76:      0b1.001.100       1       7                         -40.5            -40.5
  77:      0b1.001.101       1       8                        -121.5           -121.5
  78:      0b1.001.110       1      10                        -364.5           -364.5
  79:      0b1.001.111       1      12                       -1093.5          -1093.5
  80:      0b1.010.000       1       2                         -0.25            -0.25
  81:      0b1.010.001       1       3                         -0.75            -0.75
  82:      0b1.010.010       1       5                         -2.25            -2.25
  83:      0b1.010.011       1       6                         -6.75            -6.75
  84:      0b1.010.100       1       8                        -20.25           -20.25
  85:      0b1.010.101       1       9                        -60.75           -60.75
  86:      0b1.010.110       1      11                       -182.25          -182.25
  87:      0b1.010.111       1      13                       -546.75          -546.75
  88:      0b1.011.000       1       3                        -0.125           -0.125
  89:      0b1.011.001       1       4                        -0.375           -0.375
  90:      0b1.011.010       1       6                        -1.125           -1.125
  91:      0b1.011.011       1       7                        -3.375           -3.375
  92:      0b1.011.100       1       9                       -10.125          -10.125
  93:      0b1.011.101       1      10                       -30.375          -30.375
  94:      0b1.011.110       1      12                       -91.125          -91.125
  95:      0b1.011.111       1      14                      -273.375         -273.375
  96:      0b1.100.000       1       4                       -0.0625          -0.0625
  97:      0b1.100.001       1       5                       -0.1875          -0.1875
  98:      0b1.100.010       1       7                       -0.5625          -0.5625
  99:      0b1.100.011       1       8                       -1.6875          -1.6875
 100:      0b1.100.100       1      10                       -5.0625          -5.0625
 101:      0b1.100.101       1      11                      -15.1875         -15.1875
 102:      0b1.100.110       1      13                      -45.5625         -45.5625
 103:      0b1.100.111       1      15                      -136.688         -136.688
 104:      0b1.101.000       1       5                      -0.03125         -0.03125
 105:      0b1.101.001       1       6                      -0.09375         -0.09375
 106:      0b1.101.010       1       8                      -0.28125         -0.28125
 107:      0b1.101.011       1       9                      -0.84375         -0.84375
 108:      0b1.101.100       1      11                      -2.53125         -2.53125
 109:      0b1.101.101       1      12                      -7.59375         -7.59375
 110:      0b1.101.110       1      14                      -22.7812         -22.7812
 111:      0b1.101.111       1      16                      -68.3438         -68.3438
 112:      0b1.110.000       1       6                     -0.015625        -0.015625
 113:      0b1.110.001       1       7                     -0.046875        -0.046875
 114:      0b1.110.010       1       9                     -0.140625        -0.140625
 115:      0b1.110.011       1      10                     -0.421875        -0.421875
 116:      0b1.110.100       1      12                      -1.26562         -1.26562
 117:      0b1.110.101       1      13                      -3.79688         -3.79688
 118:      0b1.110.110       1      15                      -11.3906         -11.3906
 119:      0b1.110.111       1      17                      -34.1719         -34.1719
 120:      0b1.111.000       1       7                     -nan(ind)        -nan(ind)
 121:      0b1.111.001       1       8                    -0.0234375       -0.0234375
 122:      0b1.111.010       1      10                    -0.0703125       -0.0703125
 123:      0b1.111.011       1      11                     -0.210938        -0.210938
 124:      0b1.111.100       1      13                     -0.632812        -0.632812
 125:      0b1.111.101       1      14                      -1.89844         -1.89844
 126:      0b1.111.110       1      16                      -5.69531         -5.69531
 127:      0b1.111.111       1      18                      -17.0859         -17.0859

 */
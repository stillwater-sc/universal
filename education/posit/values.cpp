//  values.cpp : tests on values in scientific notation (sign, scale, fraction)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERSION
#include <universal/number/posit/posit>

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Constants
#define DBL_DECIMAL_DIG  17                      // # of decimal digits of rounding precision
#define DBL_DIG          15                      // # of decimal digits of precision
#define DBL_EPSILON      2.2204460492503131e-016 // smallest such that 1.0+DBL_EPSILON != 1.0
#define DBL_HAS_SUBNORM  1                       // type does support subnormal numbers
#define DBL_MANT_DIG     53                      // # of bits in mantissa
#define DBL_MAX          1.7976931348623158e+308 // max value
#define DBL_MAX_10_EXP   308                     // max decimal exponent
#define DBL_MAX_EXP      1024                    // max binary exponent
#define DBL_MIN          2.2250738585072014e-308 // min positive value
#define DBL_MIN_10_EXP   (-307)                  // min decimal exponent
#define DBL_MIN_EXP      (-1021)                 // min binary exponent
#define _DBL_RADIX       2                       // exponent radix
#define DBL_TRUE_MIN     4.9406564584124654e-324 // min positive value

#define FLT_DECIMAL_DIG  9                       // # of decimal digits of rounding precision
#define FLT_DIG          6                       // # of decimal digits of precision
#define FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0
#define FLT_HAS_SUBNORM  1                       // type does support subnormal numbers
#define FLT_GUARD        0
#define FLT_MANT_DIG     24                      // # of bits in mantissa
#define FLT_MAX          3.402823466e+38F        // max value
#define FLT_MAX_10_EXP   38                      // max decimal exponent
#define FLT_MAX_EXP      128                     // max binary exponent
#define FLT_MIN          1.175494351e-38F        // min normalized positive value
#define FLT_MIN_10_EXP   (-37)                   // min decimal exponent
#define FLT_MIN_EXP      (-125)                  // min binary exponent
#define FLT_NORMALIZE    0
#define FLT_RADIX        2                       // exponent radix
#define FLT_TRUE_MIN     1.401298464e-45F        // min positive value

#define LDBL_DIG         DBL_DIG                 // # of decimal digits of precision
#define LDBL_EPSILON     DBL_EPSILON             // smallest such that 1.0+LDBL_EPSILON != 1.0
#define LDBL_HAS_SUBNORM DBL_HAS_SUBNORM         // type does support subnormal numbers
#define LDBL_MANT_DIG    DBL_MANT_DIG            // # of bits in mantissa
#define LDBL_MAX         DBL_MAX                 // max value
#define LDBL_MAX_10_EXP  DBL_MAX_10_EXP          // max decimal exponent
#define LDBL_MAX_EXP     DBL_MAX_EXP             // max binary exponent
#define LDBL_MIN         DBL_MIN                 // min normalized positive value
#define LDBL_MIN_10_EXP  DBL_MIN_10_EXP          // min decimal exponent
#define LDBL_MIN_EXP     DBL_MIN_EXP             // min binary exponent
#define _LDBL_RADIX      _DBL_RADIX              // exponent radix
#define LDBL_TRUE_MIN    DBL_TRUE_MIN            // min positive value
-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

void TestConversionResult(bool bValid, const std::string& descriptor) {
	if (!bValid) {
		std::cout << descriptor << " conversions FAIL" << std::endl;
	}
	else {
		std::cout << descriptor << " conversions PASS" << std::endl;
	}
}

template<size_t fbits>
bool ValidateValue() {
	using namespace std;
	using namespace sw::universal;

	const int NR_TEST_CASES = 12;
	float input[NR_TEST_CASES] = {
		0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
	};
#if 0
	float golden_scales[NR_TEST_CASES] = {
		0, 0, 1, 2, 3,  4,  5,  6,   7,   8,   9,   10
	};
#endif 
	float golden_answer[NR_TEST_CASES] = {
		0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
	};

	bool bValid = true;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		internal::value<fbits> v;
		v = input[i];
		if (fabs(double(v) - golden_answer[i]) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << i << "] input " << input[i] << " ref = " << golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	for (int i = 2; i < NR_TEST_CASES; i++) {
		internal::value<fbits> v;
		v = 1.0 / input[i];
		if (fabs(double(v) - (1.0 / golden_answer[i])) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << NR_TEST_CASES + i << "] input " << 1.0 / input[i] << " ref = " << 1.0 / golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	return bValid;
}

/*
minimum normalized positive value of float, double and long double respectively
FLT_MIN
DBL_MIN
LDBL_MIN

minimum positive value of float, double and long double respectively
(macro constant)
FLT_TRUE_MIN
DBL_TRUE_MIN    (C++17)
LDBL_TRUE_MIN
 */
template<size_t fbits>
bool ValidateSubnormalFloats() {
	using namespace std;
	using namespace sw::universal;

	constexpr float flt_min = std::numeric_limits<float>::min();
	constexpr float flt_max = std::numeric_limits<float>::max();
	constexpr float flt_true_min = 1.401298464e-45F;

	bool bSuccess = false;

	cout << flt_min << " " << flt_max << endl;
	cout << flt_true_min << endl;
	cout << hexfloat << flt_min << defaultfloat << endl;

	internal::value<23> v;
	float flt = flt_min;
	std::cout << to_triple(v) << std::endl;
	for (size_t i = 0; i < 24; ++i) {
		flt /= 2.0;
		v = flt;
		cout << hexfloat << flt << defaultfloat << " " << flt << " " << to_triple(v) << " " << v << endl;
	}

	flt = flt_min + 3*flt_true_min;
	v = flt;
	cout << hexfloat << flt << defaultfloat << " " << flt << " " << to_triple(v) << " " << v << endl;

	return bSuccess;
}

template<size_t fbits>
void PrintValue(float f, const sw::universal::internal::value<fbits>& v) {
	std::cout << "float: " << std::setw(fbits) << f << sw::universal::internal::to_triple(v) << std::endl;
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	cout << "Validate subnormal floats" << endl;
	ValidateSubnormalFloats<std::numeric_limits<float>::digits>();

	cout << "Value configuration validation" << endl;
	TestConversionResult(ValidateValue<8>(), "value<8>");

	cout << "Conversion values of importance" << endl;
	/*
	no exp left : geo-dw d          0.125  result          0.0625  scale = -4  k = -2  exp = -  0001 00010          0.0625     PASS
	no rounding alltaken u          0.125  result             0.5  scale = -1  k = -1  exp = 1  0011 00100            0.25 FAIL
	no rounding alltaken u           0.25  result               1  scale =  0  k = -1  exp = 0  0100 00100            0.25 FAIL
	no rounding alltaken d           0.25  result            0.25  scale = -2  k = -1  exp = 0  0010 00100            0.25     PASS
	no rounding alltaken u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  1110 11100           -0.25     PASS
	no rounding alltaken d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  1100 11100           -0.25 FAIL
	no rounding alltaken d         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  1101 11100           -0.25 FAIL
	no exp left:  geo-dw u         -0.125  result         -0.0625  scale=  -4  k=  -2  exp=   -  1111 11110         -0.0625     PASS
	*/
	float f;
	internal::value<23> v;
	f =  0.12499f; v = f; PrintValue(f, v);
	f =  0.12500f; v = f; PrintValue(f, v);
	f =  0.12501f; v = f; PrintValue(f, v);
	f =  0.24999f; v = f; PrintValue(f, v);
	f =  0.25000f; v = f; PrintValue(f, v);
	f =  0.25001f; v = f; PrintValue(f, v);
	f = -0.25001f; v = f; PrintValue(f, v);
	f = -0.25000f; v = f; PrintValue(f, v);
	f = -0.24999f; v = f; PrintValue(f, v);
	f = -0.12501f; v = f; PrintValue(f, v);
	f = -0.12500f; v = f; PrintValue(f, v);
	f = -0.12499f; v = f; PrintValue(f, v);

	cout << "Rounding" << endl;
	internal::bitblock<8> fraction;
	fraction = 0x55;
	internal::value<8> r8(false, 0, fraction, false, false);
	cout << "Value is " << r8 << " components are " << to_triple(r8) << endl;
	internal::value<7> r7 = r8.round_to<7>();
	cout << "Value is " << r7 << " components are " << to_triple(r7) << endl;
	internal::value<6> r6 = r8.round_to<6>();
	cout << "Value is " << r6 << " components are " << to_triple(r6) << endl;
	internal::value<5> r5 = r8.round_to<5>();
	cout << "Value is " << r5 << " components are " << to_triple(r5) << endl;
	internal::value<4> r4 = r8.round_to<4>();
	cout << "Value is " << r4 << " components are " << to_triple(r4) << endl;
	internal::value<3> r3 = r8.round_to<3>();
	cout << "Value is " << r3 << " components are " << to_triple(r3) << endl;
	internal::value<2> r2 = r8.round_to<2>();
	cout << "Value is " << r2 << " components are " << to_triple(r2) << endl;
	internal::value<1> r1 = r8.round_to<1>();
	cout << "Value is " << r1 << " components are " << to_triple(r1) << endl;
	internal::value<0> r0 = r8.round_to<0>();
	cout << "Value is " << r0 << " components are " << to_triple(r0) << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

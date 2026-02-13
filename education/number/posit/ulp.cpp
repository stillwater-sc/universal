// ulp.cpp: examples of unit in the last place
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>


// generate the maximum value unit in the last place
template<unsigned nbits, unsigned es>
void GenerateMaxRangeUlp(int64_t startValue = 0, int precision = 20) {
	using Posit = sw::universal::posit<nbits, es>;
	Posit p = 0;
	Posit pn = startValue;  // just to speed up the loop as we are doing sw emulation
	while (p != pn) {
		p = pn;
		pn += 1.0f;
	}
	std::streamsize old_precision = std::cout.precision();
	std::cout << std::setprecision(precision);
	std::cout << "posit<" << nbits << "," << es << ">\n";
	std::cout << "p       = " << p << '\n';
	std::cout << "p - 1   = " << p - 1 << '\n'; // 8.388607e6
	std::cout << "p + 1   = " << p + 1 << '\n'; // 8.388608e6
	std::cout << "ulp(p)  = " << ulp(p) << '\n';
	std::cout << std::setprecision(old_precision);
}

// examples of Unit in the Last Place (ULP) measurements
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "Unit in the last place experiments\n";

/*
	The IEEE 754 specification requires that the result of an 
	elementary arithmetic operation: 
		addition, subtraction, multiplication, division, and 
		square root since 1985, and 
		FMA since 2008,
	be correctly rounded, which implies that in rounding to nearest, 
	the rounded result is within 0.5 ULP of the mathematically exact result, 
	using John Harrison's definition; conversely, this property implies that 
	the distance between the rounded result and the mathematically exact result 
	is minimized.

	For the halfway cases, it is satisfied by two consecutive floating-point numbers.

	Reputable numeric libraries compute the basic transcendental functions 
	to between 0.5 and about 1 ULP. 
	Only a few libraries compute them within 0.5 ULP, this problem being complex due to the Table-maker's dilemma.
*/


/* Example 1
   Let x be a positive floating-point number and assume that the active rounding attribute 
   is round to nearest, ties to even, denoted RN. 
   If ULP(x) is less than or equal to 1, then RN(x + 1) > x. 
   Otherwise, RN(x + 1) = x or RN(x + 1) = x + ULP(x), 
   depending on the value of the least significant digit and the exponent of x. 
*/

	// Here we start with 0 in single precision and repeatedly add 1 until the operation does not change the value. 
	// Since the significand for a single-precision number contains 24 bits, the first integer that is not exactly representable is 2^24+1, 
	// and this value rounds to 2^24 in round to nearest, ties to even. Thus the result is equal to 2^24.

	float f = 0;
	float n = 1;
	while (f != n) {
		f = n;
		n += 1.0f;
	}
	std::cout << std::setprecision(8);
	std::cout << "float 32-bit\n";
	std::cout << "f       = " << f << '\n';     // 1.6777216e7
	std::cout << "f - 1   = " << f - 1 << '\n'; // 1.6777215e7
	std::cout << "f + 1   = " << f + 1 << '\n'; // 1.6777216e7
	std::cout << "ulp(f-1)= " << ulp(f - 1) << '\n';
	std::cout << "ulp(f)  = " << ulp(f) << '\n';

	GenerateMaxRangeUlp<32, 2>(    8000000);//    8388608  ~ 8.3e6
	GenerateMaxRangeUlp<40, 2>(  536870000);//  536870912  ~ 5.36e8
	GenerateMaxRangeUlp<40, 3>( 4294960000);// 4294967296  ~ 4.294e9

	/*
		Example 2
		The following example in Java approximates π as a floating point value by finding the two double values bracketing π:

		p0 < π < p1
		// π with 20 decimal digits
		BigDecimal π = new BigDecimal("3.14159265358979323846");

		// truncate to a double floating point
		double p0 = π.doubleValue();
		// -> 3.141592653589793  (hex: 0x1.921fb54442d18p1)

		// p0 is smaller than π, so find next number representable as double
		double p1 = Math.nextUp(p0);
		// -> 3.1415926535897936 (hex: 0x1.921fb54442d19p1)
		Then ULP(π) is determined as

		ULP(π) = p1 - p0
		// ulp(π) is the difference between p1 and p0
		BigDecimal ulp = new BigDecimal(p1).subtract(new BigDecimal(p0));
		// -> 4.44089209850062616169452667236328125E-16
		// (this is precisely 2**(-51))

		// same result when using the standard library function
		double ulpMath = Math.ulp(p0);
		// -> 4.440892098500626E-16 (hex: 0x1.0p-51)
*/


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

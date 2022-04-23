#pragma once
// test_suite_mathlib.hpp : mathlib test suite for arbitrary universal number systems
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// CALLING ENVIRONMENT PREREQUISITE!!!!!
// We want the test suite to be used with different configurations of number systems
// so the calling environment needs to set the configuration
// This usually entails setting environment variables, such as
// #define POSIT_THOW_ARITHMETIC_EXCEPTIONS 1
// or
// #define AREAL_FAST_SPECIALIZATION 1

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw { namespace universal {


/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
///                             nextafter/towards TEST SUITES                         ///
/////////////////////////////////////////////////////////////////////////////////////////




/*
	 double nextafter (double x     , double y );
	  float nextafter (float x      , float y );
long double nextafter (long double x, long double y );
	 double nextafter (Type1 x      , Type2 y);        // additional overloads

	x
Base value.
y
Value toward which the return value is approximated.
If both parameters compare equal, the function returns y.

Return Value
The next representable value after x in the direction of y.

If x is the largest finite value representable in the type, and the result is infinite or not representable, an overflow range error occurs.

If an overflow range error occurs:
- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.

*/
	template<typename TestType>
	int VerifyNextafter(bool reportTestCases) {
		int nrOfFailedTests = 0;

		/*
		 next representable value after x in the direction of y
		 requires four quadrants to test:

		  x =  1.0 ->  2.0  = 1 + ULP
		  x =  1.0 ->  0.5  = 1 - ULP
		  x = -1.0 -> -2.0  = 1 - ULP
		  x = -1.0 -> -0.5  = 1 + ULP

		  plus all the boundary cases where x and/or y can be NaN
		 */

		TestType x{ 1.0f }, xpp{ 1.0f }, xmm{ 1.0f };
		++xpp;  // x + 1ULP
		--xmm;  // x - 1ULP
		TestType y;

		// positive quadrants
		y = nextafter(x, xpp);
		if (y != xpp) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << to_binary(x) << " -> " << to_binary(y) << " ref " << to_binary(xpp) << '\n';
		}
		y = nextafter(x, xmm);
		if (y != xmm) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << to_binary(x) << " -> " << to_binary(y) << " ref " << to_binary(xmm) << '\n';
		}

		// negative quadrants
		y = nextafter(-x, -xpp);
		if (y != -xpp) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << to_binary(-x) << " -> " << to_binary(y) << " ref " << to_binary(-xpp) << '\n';
		}
		y = nextafter(-x, -xmm);
		if (y != -xmm) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << to_binary(-x) << " -> " << to_binary(y) << " ref " << to_binary(-xmm) << '\n';
		}

		return nrOfFailedTests;
	}


	/*
	*
	* C++11
		 double nexttoward  (double x     , long double y);
		  float nexttowardf (float x      , long double y);
	long double nexttowardl (long double x, long double y);

	x
	Base value.
	y
	Value toward which the return value is approximated.

	If both parameters compare equal, the function returns y (converted to the return type).

	Return Value
	The next representable value after x in the direction of y.

	If x is the largest finite value representable in the type, and the result is infinite or not representable, an overflow range error occurs.

	If an overflow range error occurs:
	- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
	- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
	 */

	template<typename TestType>
	int VerifyNextoward() {
		int nrOfFailedTests = 0;

		// TODO: how do you set the target precision in a generic way?
		// targets:
		//   posit<256, 5>
		//   cfloat<128,15>
		// fixpnt? areal? valid?

		return nrOfFailedTests;
	}

}} // namespace sw::universal

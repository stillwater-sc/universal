// rump_equation.cpp: example program to show Rump computation requiring high-precision floats to work
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/number/integer/integer>
#include <universal/number/fixpnt/fixpnt>
#include <universal/number/areal/areal>
//#include <universal/number/bfloat/bfloat>
#include <universal/number/posit/posit>
#include <universal/number/lns/lns>

#include <universal/number/posit/numeric_limits.hpp>

template<typename Scalar>
Scalar Rump(const Scalar& a, const Scalar& b) {
	Scalar b2 = b * b;
	Scalar b3 = b * b * b;
	Scalar b4 = b2 * b2;
	Scalar b6 = b3 * b3;
	Scalar b8 = b4 * b4;
	Scalar a2 = a * a;
	// 333.75 * b ^ 6 + a ^ 2 * (11 * a ^ 2 * b ^ 2 - b ^ 6 - 121 * b ^ 4 - 2) + 5.5 * b ^ 8 + a / (2 * b);
	return 333.75 * b6 + a2 * (11 * a2 * b2 - b6 - 121 * b4 - 2) + 5.5 * b8 + a / (2 * b);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	cout << "Rump's equation\n";

/*
 * one off constant computation from "Handbook of Floating-Point Arithmetic":
     f(a,b) = 333.75*b^6 + a^2*(11*a^2*b^2 - b^6 - 121*b^4 - 2) + 5.5*b^8 + a/(2*b)
     for a=77617.0, b=33096.0
The exact result is -54767 / 66192 which starts with −0.8273960599...
Using pow vs repeated mutiplication for the power expressions usually yields the same answer, but not always
Running on x86 fp types we get (picking some interesting MPFR results)
Type        | Rep Mult    | Pow Func
------------+-------------+-------------
float       | -6.3383E+29 | -6.3383E+29
double      | -1.1806E+21 | -1.1806E+21
long double |  1.1726     |  5.7646E+17
quad        |  1.1726     |  1.1726
mpfr(26)    |  1.5846E+29 | -1.5846E+29
mpfr(37)    |  1.1726     | -1.5474E+26
mpfr(54)    |  1.1806e+21 |  1.1726
mpfr(76)    |  1.1726     |  1.1726
mpfr(98)    |  3.3554E+07 |  3.3554E+07
mpfr(121)   |  1.1726     |  1.1726
mpfr(122)   | -0.8274     | -0.8274
It requires 122 bits of mantissa under MPFR to get the correct answer, and is really erratic at "lower" precision.
There are a couple fpbench versions of it:  https://fpbench.org/benchmarks.html#Rump's%20example
*/

	streamsize precision = cout.precision();
	double da = 77617.0;
	double db = 33096.0;

	{
		posit<128, 2> a{ da }, b{ db };
		cout << double(Rump(a, b)) << endl;
	}
	{
		posit<156, 2> a{ da }, b{ db };
		cout << double(Rump(a, b)) << endl;
	}
	{
		posit<256, 2> a{ da }, b{ db };
		cout << double(Rump(a, b)) << endl;
	}

	cout << setprecision(precision);
	cout << endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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

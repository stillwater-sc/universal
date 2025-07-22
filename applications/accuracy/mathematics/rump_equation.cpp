// rump_equation.cpp: example program to show Rump computation requiring high-precision floats to work
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/number/erational/erational.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>

/*
In 1988, Siegfried Rump published an example in which numerical evaluation of an expression
gave a misleading result, even though use of increasing arithmetic precision suggested reliable
computation. 

Rump's example is to compute the expression:
    f(a,b) =  333.75 * b ^ 6 + a ^ 2 * (11 * a ^ 2 * b ^ 2 - b ^ 6 - 121 * b ^ 4 - 2) + 5.5 * b ^ 8 + a / (2 * b)
with a = 77617 and b 33096.

On an IBM S/370 main frame the function evaluates to the following values given the labeled
precision:
   single precision           f = 1.172603...
   double precision           f = 1.1726039400531...
   extended precision         f = 1.172603949953178...
This creates the illusion of a reliable result of approximately 1.172603. But in fact, the
correct  result is:
   correct result             f = -0.827396059946821368141165095479816...

   Using IEEE-754, we get the following results:
			    type  |                 Rump1  |                 Rump2  |                 Rump3  |
			   float  |           2.80149e+29  |                1.1726  |             -0.827396  |
			  double  |          -1.18059e+21  |                1.1726  |             -0.827396  |
		 long double  |          -1.18059e+21  |                1.1726  |             -0.827396  |

The root cause of this behavior is catastrophic cancellation due to the large scale of the exponentiation terms.
The value of a and b satisfy the equation:
           a ^ 2 = 5.5 * b ^ 2  +  1
Simple algebraic manipulation yields the more transparent form of the computation

    f(a,b) = 5.5 * b ^ 8 - 1 - 5.5 * b ^ 8 + a / ( 2 * b)

In this form it is easy to see where the cancellation occurs. The large term 5.5b^8 cancels out leaving
the quation:

    f(a,b) = -2 + a / (2 * b), which yields the correct value of -0.827396... for most formats.

For any arithmetic to evaluate this function in its raw form requires enough precision bits to
represent the value 1.0 in the ULP. 5.5*b^8 at b = 33096 is of the order of 8e+36, which requires 
122 bits of precision to capture the -2.0 while still representing 8e+36 and thus avoiding
catastrophic cancellation of this -2.0 during the computation.

*/
// original f(a,b) =  333.75 * b ^ 6 + a ^ 2 * (11 * a ^ 2 * b ^ 2 - b ^ 6 - 121 * b ^ 4 - 2) + 5.5 * b ^ 8 + a / (2 * b);
template<typename Scalar>
Scalar Rump1(double _a, double _b) {
	Scalar a, b;
	a = Scalar(_a);
	b = Scalar(_b);
	Scalar b2 = b * b;
	Scalar b3 = b * b * b;
	Scalar b4 = b2 * b2;
	Scalar b6 = b3 * b3;
	Scalar b8 = b4 * b4;
	Scalar a2 = a * a;
	// 333.75 * b ^ 6 + a ^ 2 * (11 * a ^ 2 * b ^ 2 - b ^ 6 - 121 * b ^ 4 - 2) + 5.5 * b ^ 8 + a / (2 * b);
	return 333.75 * b6 + a2 * (11 * a2 * b2 - b6 - 121 * b4 - 2) + 5.5 * b8 + a / (2 * b);
}

template<typename Scalar>
Scalar TraceRump1(double _a, double _b) {
	using namespace sw::universal;
	Scalar a{}, b{};
	std::cout << "+-----------------------------------------------------------------------------\n" << sw::universal::type_tag(a) << '\n';
	a = Scalar(_a);
	b = Scalar(_b);
	Scalar b2 = b * b;
	std::cout << "b * b                              : " << to_binary(b2) << " : " << b2 << '\n';
	Scalar b3 = b * b * b;
	std::cout << "b * b * b                          : " << to_binary(b3) << " : " << b3 << '\n';
	Scalar b4 = b2 * b2;
	std::cout << "b * b * b * b                      : " << to_binary(b4) << " : " << b4 << '\n';
	Scalar b6 = b3 * b3;
	std::cout << "b3 * b3                            : " << to_binary(b6) << " : " << b6 << '\n';
	Scalar b8 = b4 * b4;
	std::cout << "b4 * b4                            : " << to_binary(b8) << " : " << b8 << '\n';
	Scalar a2 = a * a;
	std::cout << "a * a                              : " << to_binary(a2) << " : " << a2 << '\n';

	Scalar term1 = 333.75 * b6;
	std::cout << "333.75 * b6                  term1 : " << to_binary(term1) << " : " << term1 << '\n';
	Scalar term2 = 11 * a2 * b2;
	std::cout << "11 * a2 * b2                       : " << to_binary(term2) << " : " << term2 << '\n';
	Scalar term3 = (11 * a2 * b2 - b6 - 121 * b4 - 2);
	std::cout << "(11 * a2 * b2 - b6 - 121 * b4 - 2) : " << to_binary(term3) << " : " << term3 << '\n';

	Scalar term4 = a2 * term3;
	std::cout << "a2 * previous_term           term4 : " << to_binary(term4) << " : " << term4 << '\n';
	Scalar term5 = 5.5 * b8;
	std::cout << "5.5 * b8                     term5 : " << to_binary(term5) << " : " << term5 << '\n';
	Scalar diff = term4 + term5;
	std::cout << "term4 + term5                diff  : " << to_binary(diff) << " : " << diff << '\n';

	Scalar term6 = a / (2 * b);
	std::cout << "a / (2 * b)                  term6 : " << to_binary(term6) << " : " << term6 << '\n';

	Scalar result = term1 + term4 + term5 + term6;
	std::cout << "term1 + term4 + term5 + term6      : " << to_binary(result) << " : " << result << '\n';

	// 333.75 * b ^ 6 + a ^ 2 * (11 * a ^ 2 * b ^ 2 - b ^ 6 - 121 * b ^ 4 - 2) + 5.5 * b ^ 8 + a / (2 * b);
	return 333.75 * b6 + a2 * (11 * a2 * b2 - b6 - 121 * b4 - 2) + 5.5 * b8 + a / (2 * b);
}

// rewrite f(a,b) = 5.5 * b ^ 8 - 2 - 5.5 * b ^ 8 + a / (2 * b)
template<typename Scalar>
Scalar Rump2(double _a, double _b) {
	Scalar a, b;
	a = Scalar(_a);
	b = Scalar(_b);
	Scalar b2 = b * b;
	Scalar b4 = b2 * b2;
	Scalar b8 = b4 * b4;
	// 5.5 * b ^ 8 - 2 - 5.5 * b ^ 8 + a / (2 * b)
	return 5.5 * b8 - 2.0 - 5.5 * b8 + (a / (2 * b));
}

// f(a,b) = -2 + a / 2b  
template<typename Scalar>
Scalar Rump3(double _a, double _b) {
	Scalar a, b;
	a = Scalar(_a);
	b = Scalar(_b);
	return -2.0 + a / (2 * b);
}

template<typename Scalar>
void GenerateRow(double a, double b, sw::universal::blas::matrix<double>& table, size_t rowNr) {
	table(rowNr, 0) = double(Rump1<Scalar>(a, b));
	table(rowNr, 1) = double(Rump2<Scalar>(a, b));
	table(rowNr, 2) = double(Rump3<Scalar>(a, b));
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "Rump's equation\n";

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

	std::streamsize precision = std::cout.precision();
	double a = 77617.0;
	double b = 33096.0;

	using Labels = std::vector<std::string>;
	using Data = blas::matrix<double>;

	Labels rowLbls = {
		"float",
		"double",
		"long double",
		"quad",
		"posit16",
		"posit32",
		"posit48",
		"posit64",
		"posit80",
		"posit128",
		"posit156",
		"cfloat16",
		"cfloat32",
		"cfloat64",
		"cfloat80",
		"dd",
		"qd"
	};
	Labels column = {
		"Rump1",
		"Rump2",
		"Rump3"
	};

//	using quad = bfloat<128, 11>;
	Data table(20, 5);
	GenerateRow<float>(a, b, table, 0);
	GenerateRow<double>(a, b, table, 1);
	GenerateRow<long double>(a, b, table, 2);
//	GenerateRow<quad>(a, b, table, 3);
	GenerateRow<posit<16, 2>>(a, b, table, 4);
	GenerateRow<posit<32, 2>>(a, b, table, 5);
	GenerateRow<posit<48, 2>>(a, b, table, 6);
	GenerateRow<posit<64, 2>>(a, b, table, 7);
	GenerateRow<posit<80, 2>>(a, b, table, 8);
	GenerateRow<posit<128, 2>>(a, b, table, 9);
	GenerateRow<posit<156, 2>>(a, b, table, 10);
	GenerateRow<cfloat<16, 11, std::uint16_t, true>>(a, b, table, 11);
	GenerateRow<cfloat<32, 11, std::uint32_t, true>>(a, b, table, 12);
	GenerateRow<cfloat<64, 11, std::uint32_t, true>>(a, b, table, 13);
	GenerateRow<cfloat<80, 11, std::uint32_t, true>>(a, b, table, 14);
	GenerateRow<dd>(a, b, table, 15);
	GenerateRow<qd>(a, b, table, 16);

	// print the table
	constexpr size_t COLUMN_WIDTH = 20;
	std::cout << std::setw(COLUMN_WIDTH) << "type" << "  |  ";
	for (size_t j = 0; j < size(column); ++j) {
		std::cout << std::setw(COLUMN_WIDTH) << column[j] << "  |  ";
	}
	std::cout << '\n';
	for (size_t i = 0; i < size(rowLbls); ++i) {
		std::cout << std::setw(COLUMN_WIDTH) << rowLbls[i] << "  |  ";
		for (size_t j = 0; j < 3; ++j) {
			std::cout << std::setw(COLUMN_WIDTH) << table(i, j) << "  |  ";
		}
		std::cout << '\n';
	}
	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
	// trace out the original Rump equation with different number systems
	TraceRump1<double>(a, b);
	TraceRump1<posit<63, 2>>(a, b);
	TraceRump1<posit<64, 2>>(a, b);
	TraceRump1<qd>(a, b);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

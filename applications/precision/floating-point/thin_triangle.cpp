// goldberg_thin_triangle.cpp: example program showing the Goldberg thin triangle example
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the posit number system
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

/* 
* Based on the discussion of rounding error of Golberg's thin triangle
* 
* We are following the exposition described in http://marc-b-reynolds.github.io/math/2019/02/06/Posit1.html
*/

/*

Introduction:

Goldberg’s long thin triangle
Kahan presented this problem in or prior to 1986 and the Goldberg paper (from 1991) was inspired from attended a conference by Kahan.


Compute the area A of a thin triangle using the classic form of Heron’s equation.

                 ^
	   b = c   /   \  c = 7/2 + 3*ulp(a)
	          /     \
			 +-------+
			     a

s = (a+b+c) / 2
A = SQRT(s(s−a)(s−b)(s−c))

The lengths are set to:
a=7
b=c= 0.5(a+3*ulp(a))   ulp is "unit in last position"

exact  = 1000000001001111111001110×2−31
posit  = 1000000001001111111001111×2−31
IEEE   = 1001010000101001011111110×2−31


This is an example of loss of significance due to catastrophic cancellation

For both posits & IEEE we have the upper 3 bits set for all three inputs with exponent of b and c smaller by 1. 
For IEEE’s b and c values the bottom two bits are set. 
However posits values in this range have two more bits available, so the same numeric value has the bottom two clear. 
Let’s look what happens when we start to compute s. First we perform a+b:

           IEEE                                 POSITS

t0=a+b:    111.000000000000000000000            111.00000000000000000000000
         +  11.1000000000000000000011         +  11.100000000000000000001100
          ----------------------------         ------------------------------
          1010.1000000000000000000011          1010.100000000000000000001100
round:    1010.10000000000000000001            1010.10000000000000000000110


The upper bits cause a carry (both increase exp by one) and that trailing bit of b means we need one more bit (now 25) to represent exactly. 
IEEE has to round to 24 bits and the posits version still has one zero bit at the bottom. 

Let’s complete the computation of s.


t1=t0+c:  1010.10000000000000000001            1010.10000000000000000000110
         +  11.1000000000000000000011         +  11.10000000000000000000110
         ----------------------------         -----------------------------
          1110.0000000000000000000111          1110.00000000000000000001100
round:    1110.00000000000000000010            1110.00000000000000000001100

.5f*t1:   111.000000000000000000010            111.000000000000000000001100

s:        7.000000954                          7.000000715


This time we still need 25 bits to be exact since t0 had to adjust the exp and IEEE must round again. 
Posits are still good with the padding bits we gave them. 
The multiply by half introduces no error for either. 
Also shown is the decimal values of each to 10 digits and the binary32 relative error is a tiny  3.40598×10−8.

Now for the (s−a) and (s−b) terms:

s-a:      111.000000000000000000010            111.000000000000000000001100
         -111.000000000000000000000           -111.000000000000000000000000
         ----------------------------         -----------------------------
            0.000000000000000000010              0.000000000000000000001100

s-b:      111.000000000000000000010            111.000000000000000000001100
          -11.1000000000000000000011           -11.100000000000000000001100
         ----------------------------         -----------------------------
           11.100000000000000000001             11.100000000000000000000000


Again posits don’t have any rounding error. The binary32 (s−b) has a tiny relative error of 6.81196×10−8 
and the performed (s−a) subtraction was exact, but the total relative error is a massive 0.3333¯¯¯. 
The tiny error in s was magnified by a subtraction of a nearby number. 
This is an example of catastrophic cancellation or loss of significance. 

John D. Cook’s version of a common rule of thumb:
    Cardinal rule of floating point arithmetic:
       If x and y agree to n bits, then you can lose up to n bits of precision computing x-y.


Nothing interesting happens in the remaining operations. All of the error is from a 
contrived set of numbers where IEEE is in a catastrophic cancellation case and posits are not. 
There is a noteworthy observation: the final relative error of the IEEE result is  0.154701. 
This is an example of the incorrect notion that errors grow without bound as a computation progresses.

Background references on the thin triangle problem: as previously mentioned this problem was original introduced by Kahan2 (as early as 1986) followed by Goldberg3 performing a pen-and-paper analysis and most recently Boldo4 provides a formal proof and a tighter error bound.

*/

// Naive application of Heron's Formula without any regard of rounding error
template<typename Scalar>
Scalar HeronFormulaNaive(const Scalar& a, const Scalar& b, const Scalar& c, bool verbose = false) {
	using namespace sw::universal;

	Scalar s = (a + b + c) / 2;
	Scalar A = sqrt(s * (s - a)*(s - b)*(s - c));

	if (verbose) {
		std::cout << "Thin triangle area calculation using Heran's formula\n";
		std::cout << "    a  = " << to_binary(a) << " " << to_base2_scientific(a) << " : " << std::showpos << a << std::noshowpos << '\n';
		std::cout << "    b  = " << to_binary(b) << " " << to_base2_scientific(b) << " : " << std::showpos << std::setprecision(8) << b << std::noshowpos << '\n';
		std::cout << "    c  = " << to_binary(c) << " " << to_base2_scientific(c) << " : " << std::showpos << c << std::noshowpos << '\n';
		std::cout << "    s  = " << to_binary(s) << " " << to_base2_scientific(s) << " : " << std::showpos << s << std::noshowpos << '\n';
		std::cout << "    A  = " << to_binary(A) << " " << to_base2_scientific(A) << " : " << std::showpos << A << std::noshowpos << '\n';

		Scalar p1, p2, p3;
		p1 = (s - a);
		p2 = (s - b);
		p3 = (s - c);
		std::cout << "    s      = " << to_binary(s) << '\n';
		std::cout << "        a  = " << to_binary(a) << '\n';
		std::cout << "   (s - a) = " << to_binary(p1) << '\n';
		std::cout << "   (s - a) = " << to_base2_scientific(p1) << '\n';
		std::cout << "   (s - b) = " << to_base2_scientific(p2) << '\n';
		std::cout << "   (s - c) = " << to_base2_scientific(p3) << '\n';
	}

	return A;
}

/*
“Miscalculating Area and Angles of a Needle-like Triangle”, W. Kahan, 2014
The Boldo paper4 details Kahan’s solution (for double input) which is an example of using option two. 
This is going to be left as a black box for now and it cost about one more issue vs. Heron’s (godbolt):

This list of requirements simply are: sorted largest first, valid triangle (including degenerates to line). 
Taking the original set of inputs and using Kahan’s method with 32-bit operations gives:

exact   = 1.000000001001111111001110111110×2−7   ≈0.007831550660
posit   = 1.000000001001111111001110110000×2−7   ≈0.007831550553
IEEE    = 1.000000001001111111010000000000×2−7   ≈0.007831551135

An interesting question is then: Does the error bound of Kahan’s method hold for posits? Well we’ll have to de-black-box it at some point I guess. 
An aside here: the complexity of Kahan’s method as shown is about the same as Heron’s (godbolt). 
The real cost is the ordering requirement in the cases where it’s not known nor otherwise required.
*/
template<typename Scalar>
Scalar HeronFormulaKahanRewrite(const Scalar& a, const Scalar& b, const Scalar& c, bool verbose = false) {
	using namespace sw::universal;

	// requires: a >= b >= c && a <= b+c && a <= 0x1.0p255
	Scalar s = (a + b + c) / 2;
	Scalar A = Scalar(0.25)*sqrt((a + (b + c))*(a + (b - c))*(c + (a - b))*(c - (a - b)));

	if (verbose) {
		std::cout << "Thin triangle area calculation using Kahan rewrite\n";
		std::cout << "    a  = " << to_binary(a) << " " << to_base2_scientific(a) << " : " << std::showpos << a << std::noshowpos << '\n';
		std::cout << "    b  = " << to_binary(b) << " " << to_base2_scientific(b) << " : " << std::showpos << b << std::noshowpos << '\n';
		std::cout << "    c  = " << to_binary(c) << " " << to_base2_scientific(c) << " : " << std::showpos << c << std::noshowpos << '\n';
		std::cout << "    s  = " << to_binary(s) << " " << to_base2_scientific(s) << " : " << std::showpos << s << std::noshowpos << '\n';
		std::cout << "    A  = " << to_binary(A) << " " << to_base2_scientific(A) << " : " << std::showpos << A << std::noshowpos << '\n';

		Scalar p1, p2, p3, p4;
		p1 = a + (b + c);
		p2 = a + (b - c);
		p3 = c + (a - b);
		p4 = c - (a - b);
		std::cout << "(a + (b + c)) = " << to_base2_scientific(p1) << '\n';
		std::cout << "(a + (b - c)) = " << to_base2_scientific(p2) << '\n';
		std::cout << "(c + (a - b)) = " << to_base2_scientific(p3) << '\n';
		std::cout << "(c - (a - b)) = " << to_base2_scientific(p4) << '\n';
	}

	return A;
}

/*
Let's assume the default behavior of a program is to run as quickly as possible.
However, with a significant comment, you can specify that certain values are to be computed "safely,"
using the XSC methods and the quire. For example, suppose a line of code is

x = a * b * c;

With both floats and posits, multiplication does not follow the associative law.
Floats can overflow from one of the multiply operations even if the mathematical value of x
is perfectly representable; posits can get very inaccurate if one of the products lands
in the large-magnitude or small-magnitude regions. Without knowing any numerical analysis or
how it works, a programmer could precede this with something like

//$ safe(x)
x = a * b * c;

indicating that the next evaluation of x is to be performed as if there is infinite precision,
then rounded to the nearest posit. The compiler sets up the sparse lower-triangular linear system

| 1         | | t1 |   | a |
|           | |    |   |   |
| b  -1     |•| t2 | = | 0 |
|           | |    |   |   |
|     c  -1 | | t3 |   | 0 |
*/
template<typename Scalar>
Scalar HeronFormulaKarlsruheAccurateArithmetic(const Scalar& a, const Scalar& b, const Scalar& c, bool verbose = false) {
	return HeronFormulaKahanRewrite(a, b, c, verbose);  // TODO
}

template<typename Scalar>
void printTriangleConfiguration(std::ostream& ostr, const Scalar& a, const Scalar& b, const Scalar& c) {
	ostr << "    a  = " << sw::universal::to_binary(a) << " " << sw::universal::to_base2_scientific(a) << " : " << std::showpos << a << std::noshowpos << '\n';
	ostr << "    b  = " << sw::universal::to_binary(b) << " " << sw::universal::to_base2_scientific(b) << " : " << std::showpos << b << std::noshowpos << '\n';
	ostr << "    c  = " << sw::universal::to_binary(c) << " " << sw::universal::to_base2_scientific(c) << " : " << std::showpos << c << std::noshowpos << '\n';
}

int main()
try {
	using namespace sw::universal;
	using std::abs;

	/*
	{
		// single precision floats
		std::cout << "0.125f = " << to_base2_scientific(0.125f) << '\n';
		std::cout << "0.25f  = " << to_base2_scientific(0.25f) << '\n';
		std::cout << "0.5f   = " << to_base2_scientific(0.5f) << '\n';
		std::cout << "1.0f   = " << to_base2_scientific(1.0f) << '\n';
		std::cout << "2.0f   = " << to_base2_scientific(2.0f) << '\n';
		std::cout << "4.0f   = " << to_base2_scientific(4.0f) << '\n';
		std::cout << "8.0f   = " << to_base2_scientific(8.0f) << '\n';

		std::cout << "ulp(1) = " << to_base2_scientific(ulp(1.0f)) << '\n';
	}

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 1;
		std::cout << "0.0625 = " << to_base2_scientific(posit<nbits, es>(0.0625f)) << '\n';
		std::cout << "0.125f = " << to_base2_scientific(posit<nbits, es>(0.125f)) << '\n';
		std::cout << "0.25f  = " << to_base2_scientific(posit<nbits, es>(0.25f)) << '\n';
		std::cout << "0.5f   = " << to_base2_scientific(posit<nbits, es>(0.5f)) << '\n';
		std::cout << "1.0f   = " << to_base2_scientific(posit<nbits, es>(1.0f)) << '\n';
		std::cout << "2.0f   = " << to_base2_scientific(posit<nbits, es>(2.0f)) << '\n';
		std::cout << "4.0f   = " << to_base2_scientific(posit<nbits, es>(4.0f)) << '\n';
		std::cout << "8.0f   = " << to_base2_scientific(posit<nbits, es>(8.0f)) << '\n';
		std::cout << "16.0f  = " << to_base2_scientific(posit<nbits, es>(16.0f)) << '\n';
		std::cout << "32.0f  = " << to_base2_scientific(posit<nbits, es>(32.0f)) << '\n';
		std::cout << "64.0f  = " << to_base2_scientific(posit<nbits, es>(64.0f)) << '\n';
		std::cout << "ulp(1) = " << to_base2_scientific(ulp(posit<nbits, es>(1.0f))) << '\n';
		std::cout << "       = " << to_binary(posit<nbits, es>(1.0f)) << '\n';

	}
*/
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	bool verbose = false;

	// build the triangle in double precision representation
	double a, b, c;
	double Aexact;
	float  Aieee32b;
	Posit  Aposit32;

	// create the thin triangle
	a = 7.0;
	double delta = ulp<float>(float(a));
	b = 0.5 * (a + 3.0 * delta);
	c = b;
	if (verbose) {
		printTriangleConfiguration<float>(std::cout, float(a), float(b), float(c));
		printTriangleConfiguration<Posit>(std::cout, a, b, c);
	}


	// demonstrate the rounding issues of calculating the area of this thin triangle
	std::cout << "Area calculation of a thin triangle\n";
	Aexact = HeronFormulaKahanRewrite(a, b, c);	

	std::cout << "Using Heron's Formula with disregard to catastrophic cancellation\n";
	std::cout << "exact                 = " << std::showpos << Aexact << std::noshowpos << '\n';
	Aieee32b = HeronFormulaNaive<float>(float(a), float(b), float(c));
	std::cout << "IEEE single precision = " << std::showpos << Aieee32b << std::noshowpos << "  relative error : " << abs(Aexact - Aieee32b) << '\n';
	Aposit32 = HeronFormulaNaive<Posit>(Posit(a), Posit(b), Posit(c));
	std::cout << type_tag(Aposit32) << "          = " << Aposit32 << "  relative error : " << abs(Aexact - double(Aposit32)) << '\n';	

	std::cout << '\n';

	std::cout << "Using Kahan rewrite to avoid catastrophic cancellation\n";
	std::cout << "exact                 = " << std::showpos << Aexact << std::noshowpos << '\n';
	Aieee32b = HeronFormulaKahanRewrite<float>(float(a), float(b), float(c));
	std::cout << "IEEE single precision = " << std::showpos << Aieee32b << std::noshowpos << "  relative error : " << abs(Aexact - Aieee32b) << '\n';
	Aposit32 = HeronFormulaKahanRewrite<Posit>(Posit(a), Posit(b), Posit(c));
	std::cout << type_tag(Aposit32) << "          = " << Aposit32 << "  relative error : " << abs(Aexact - double(Aposit32)) << '\n';

	std::cout << '\n';

	std::cout << "Using Karlsruhe Accurate Arithmetic\n";
	std::cout << "exact                 = " << std::showpos << Aexact << std::noshowpos << '\n';
//	Aieee32b = HeronFormulaKahanRewrite<float>(float(a), float(b), float(c));
//	std::cout << "IEEE single precision = " << std::showpos << Aieee32b << std::noshowpos << "  relative error : " << abs(Aexact - Aieee32b) << '\n';
	Aposit32 = HeronFormulaKarlsruheAccurateArithmetic<Posit>(Posit(a), Posit(b), Posit(c));
	std::cout << type_tag(Aposit32) << "          = " << Aposit32 << "  relative error : " << abs(Aexact - double(Aposit32)) << '\n';

	std::cout << std::setprecision(precision);

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

// arbitrary_precision_pi.cpp: generating a 'perfect' approximation of pi for a given number system
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>

/*
Traditionally, we define the PI as the ratio of the circumference and its diameter.
Historically, however, was not always so.

It is known that this irrational number arose on the calculations of geometers
over time as a proportionality constant for at least 4 relationships, not necessarily in this order:

 - Between the circumference of a circle to its diameter;
 - Between the area of a circle and the square of its diameter;
 - Between the area of a sphere and the square of its diameter;
 - Between the volume of a sphere and the cube of its diameter;

The earliest known written references of the PI come from Babylon around 2000 BC.
Since then, their approximations have gone through several transformations until
they reach the billions of digits obtained today with the aid of the computer.

Historically, one of the best approximations of PI and interestingly also one of the oldest,
was used by the Chinese mathematician Zu Chongzhi (Sec.450 DC), which related the PI
as "something" between 3.1415926 and 3.1415927.

The calculation of PI has been revolutionized by the development of techniques of
infinite series, especially by mathematicians from europe in the 16th and 17th centuries.
An infinite series is the sum (or product) of the terms of an infinite sequence.
That approach was first discovered in India sometime between 1400 and 1500 AD.

Now let's look at the main discoveries in this area:

Viete's Series
The first infinite sequence discovered in Europe was an infinite product,
found by French mathematician François Viète in 1593:

2    sqrt(2)   sqrt(2 + sqrt(2))   sqrt(2 + sqrt(2 + sqrt(2)))
-  = ------- * ----------------- * --------------------------- * ...
pi      2             2                         2

Wallis's Series
The second infinite sequence, found in Europe by John Wallis in 1655, was also an infinite product:

pi   2   2   4   4   6   6   8   8
-- = - * - * - * - * - * - * - * - * ...
2    1   3   3   5   5   7   7   9

Leibniz's Series
Madhava of Sangamagrama, a Indian mathematician, formulated a series that was rediscovered
by scottish mathematician James Gregory in 1671, and by Leibniz in 1674:

	 4   4   4   4   4   4    4
pi = - - - + - - - + - - -- + -- ...
	 1   3   5   7   9   11   13

Nilakantha's Series
An infinite series for PI published by Nilakantha in the 15th century is:

		   4       4       4       4
pi = 3 + ----- - ----- + ----- - ------ + ...
		 2*3*4   4*5*6   6*7*8   8*9*10

*/

// best practice for C++ is to assign a literal
// but even this literal gets rounded in a double assignment
constexpr 
double pi = 3.14159265358979323846;  
//    pi  = 3.141592653589793115997963    value of above literal
//    ref = 3.14159265358979323846264338327950288419716939937510

// first 50 digits of pi
static std::string pi50 = "3.\
14159265358979323846264338327950288419716939937510";
// first 1000 digits of pi
static std::string pi1000 = "3.\
14159265358979323846264338327950288419716939937510\
58209749445923078164062862089986280348253421170679\
82148086513282306647093844609550582231725359408128\
48111745028410270193852110555964462294895493038196\
44288109756659334461284756482337867831652712019091\
45648566923460348610454326648213393607260249141273\
72458700660631558817488152092096282925409171536436\
78925903600113305305488204665213841469519415116094\
33057270365759591953092186117381932611793105118548\
07446237996274956735188575272489122793818301194912\
98336733624406566430860213949463952247371907021798\
60943702770539217176293176752384674818467669405132\
00056812714526356082778577134275778960917363717872\
14684409012249534301465495853710507922796892589235\
42019956112129021960864034418159813629774771309960\
51870721134999999837297804995105973173281609631859\
50244594553469083026425223082533446850352619311881\
71010003137838752886587533208381420617177669147303\
59825349042875546873115956286388235378759375195778\
18577805321712268066130019278766111959092164201989";

template<typename Real>
Real MethodOfViete(size_t N) {
	using namespace sw::unum;
	Real pi = Real(1);
	for (size_t i = N; i > 1; --i) {
		Real repeatingFactor = Real(2);
		for (size_t j = 1; j < i; ++j) {
			repeatingFactor = Real(2) + sqrt(repeatingFactor);
		}
		repeatingFactor = sqrt(repeatingFactor);
		pi = pi * repeatingFactor / Real(2);
	}
	pi *= sqrt(Real(2)) / Real(2);
	pi = Real(2) / pi;
	return pi;
}
 template<typename Real>
 Real MethodOfWallis(size_t N) {
	 using namespace sw::unum;
	 Real pi = Real(4);
	 for (size_t i = 3; i <= (N + 2); i += 2) {
		 pi = pi * (Real(i - 1) / Real(i)) * (Real(i + 1) / Real(i));
	 }
	 return pi;
 }

 template<typename Real>
 Real MethodOfMadhavaOfSangamagrama(size_t N) {
	 using namespace sw::unum;
	 Real pi = Real(0);
	 Real s = Real(1); // sign for the next iteration
	 for (size_t i = 1; i <= (2 * N); i += 2) {
		 pi = pi + s * (Real(4) / Real(i));
		 s = -s;
	 }
	 return pi;
 }

 template<typename Real>
 Real MethodOfNilakantha(size_t N) {
	 using namespace sw::unum;
	 Real pi = Real(3);
	 Real s = Real(1); // sign for the next iteration
	 for (size_t i = 2; i <= (2 * N); i += 2) {
		 pi = pi + s * (Real(4) / Real(i * (i + 1) * (i + 2)) );
		 s = -s;
	 }
	 return pi;
 }

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	cout << "Perfect approximations of PI for different number systems" << endl;

	cout << pi1000 << endl;
	cout << "pi  = " << setprecision(25) << pi << endl;
	cout << "ref = " << pi50 << endl;

	using Real = posit<64,3>; 
	size_t N = 100;
	cout << "Viete Series using " << N << " iteration" << endl; // doesn't really work for floats as rounding error accumulates to quickly
	cout << "pi  = " << setprecision(20) << MethodOfViete<float>(N) << endl;
	cout << "pi  = " << setprecision(20) << MethodOfViete<double>(N) << endl;
	cout << "ref = " << pi50 << endl;
	cout << "pi  = " << setprecision(20) << MethodOfViete<Real>(N) << endl;

	N = 1000;
	cout << "Wallis Series using " << N << " iteration" << endl; // doesn't really work for floats as rounding error accumulates to quickly
	cout << "pi  = " << setprecision(20) << MethodOfWallis<float>(N) << endl;
	cout << "pi  = " << setprecision(20) << MethodOfWallis<double>(N) << endl;
	cout << "ref = " << pi50 << endl;
	cout << "pi  = " << setprecision(20) << MethodOfWallis<Real>(N) << endl;

	N = 1000;
	cout << "Madhava of Sangamagrama (or Leibniz) Series using " << N << " iteration" << endl; // doesn't really work for floats as rounding error accumulates to quickly
	cout << "pi  = " << setprecision(20) << MethodOfMadhavaOfSangamagrama<float>(N) << endl;
	cout << "pi  = " << setprecision(20) << MethodOfMadhavaOfSangamagrama<double>(N) << endl;
	cout << "ref = " << pi50 << endl;
	cout << "pi  = " << setprecision(20) << MethodOfMadhavaOfSangamagrama<Real>(N) << endl;

	N = 1000;
	cout << "Nilakantha Series using " << N << " iteration" << endl; // doesn't really work for floats as rounding error accumulates to quickly
	cout << "pi  = " << setprecision(20) << MethodOfNilakantha<float>(N) << endl;
	cout << "pi  = " << setprecision(20) << MethodOfNilakantha<double>(N) << endl;
	cout << "ref = " << pi50 << endl;
	cout << "pi  = " << setprecision(20) << MethodOfNilakantha<Real>(N) << endl;


	// 1000 digits -> 1.e1000 -> 2^3322 -> 1.051103774764883380737596422798e+1000 -> you will need 3322 bits to represent 1000 digits of pi

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

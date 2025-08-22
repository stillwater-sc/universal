// factorial.cpp: evaluation of factorials in the posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
// enable conversion between posits and integers
#include <universal/adapters/adapt_integer_and_posit.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>

// TODO: this should be writable as a single generic function taking an Integer concept and a Posit concept
// the simple 
//   template<typename Integer, typename Posit>
//   void GenerateFactorialTableComparison(...) 
// did not work due to ambiguity to resolve the arguments uniquely to resolve the statement
//   integerRef = positRef; 
// in the body of the function: ETLO 1/19/2020

// generate factorials in an Integer and a Posit number system to compare
template<unsigned pbits, unsigned pes>
void GenerateFactorialTableComparison(unsigned upperbound, unsigned long long factorialValue = 1, sw::universal::posit<pbits,pes> positRef = 1, int columnWidth = 30) {
	std::cout << "\n+---------------\n" << typeid(factorialValue).name() << " and " << typeid(positRef).name() << '\n';
	std::cout << "  i    " << std::setw(columnWidth) << "integer(N!)" << "  " << std::setw(columnWidth) << "posit(N!)" << std::setw(columnWidth) << "abs(error)\n";
	for (unsigned i = 2; i < upperbound; ++i) {
		factorialValue *= i;
		positRef *= i;
		unsigned long long integerRef;
		integerRef = (unsigned long long)positRef;
		unsigned long long error = (factorialValue > integerRef ? factorialValue - integerRef : integerRef - factorialValue);
		std::cout << std::setw(5) << i << "  " << std::setw(columnWidth) << factorialValue << "  " << std::setw(columnWidth) << positRef << std::setw(columnWidth) << error << '\n';
	}
}

template<unsigned ibits, unsigned pbits, unsigned pes>
void GenerateFactorialTableComparison(unsigned upperbound, sw::universal::integer<ibits> factorialValue = 1, sw::universal::posit<pbits, pes> positRef = 1, int columnWidth = 30) {
	std::cout << "\n+---------------\n" << typeid(factorialValue).name() << " and " << typeid(positRef).name() << '\n';
	std::cout << "  i    " << std::setw(columnWidth) << "integer(N!)" << "  " << std::setw(columnWidth) << "posit(N!)" << std::setw(columnWidth) << "abs(error)\n";
	for (unsigned i = 2; i < upperbound; ++i) {
		factorialValue *= i;
		positRef *= i;
		sw::universal::integer<ibits> integerRef;
		integerRef = positRef;
		sw::universal::integer<ibits> error = (factorialValue > integerRef ? factorialValue - integerRef : integerRef - factorialValue);
		std::cout << std::setw(5) << i << "  " << std::setw(columnWidth) << factorialValue << "  " << std::setw(columnWidth) << positRef << std::setw(columnWidth) << error << '\n';
	}
}


// generate factorial tables using different number types
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	unsigned upperbound = 22;   
	{
		// 20! can still be represented by a 64-bit integer
		// 21! can not be represented by a 64-bit integer
		// 13! and up generate integers that a 32-bit posit can't represent
		using Integer = unsigned long long;
		constexpr unsigned nbits = 32;
		constexpr unsigned es = 2;
		using Posit = posit<nbits, es>;
		int columnWidth = 40;
		GenerateFactorialTableComparison(upperbound, Integer(1), Posit(1), columnWidth);
 
		   /*
+---------------
unsigned __int64 and class sw::universal::posit<32,2>
  i                                 integer(N!)                                 posit(N!)                             abs(error)
	2                                         2                                         2                                       0
	3                                         6                                         6                                       0
	4                                        24                                        24                                       0
	5                                       120                                       120                                       0
	6                                       720                                       720                                       0
	7                                      5040                                      5040                                       0
	8                                     40320                                     40320                                       0
	9                                    362880                                    362880                                       0
   10                                   3628800                                   3628800                                       0
   11                                  39916800                                  39916800                                       0
   12                                 479001600                                 479001600                                       0
   13                                6227020800                                6227017728                                    3072   <- the 32-bit posit can't represent these integers
   14                               87178291200                               87178346496                                   55296
   15                             1307674368000                          1.3076749353e+12                                  567296
   16                            20922789888000                         2.09229331825e+13                               143294464
   17                           355687428096000                         3.55692011586e+14                              4583489536
   18                          6402373705728000                         6.40245620854e+15                             82502811648
   19                        121645100408832000                         1.21649966497e+17                           4866088304640
   20                       2432902008176640000                         2.43306969869e+18                         167690510270464
   21                      14197454024290336768                         5.10888341729e+19                     4974081987435560960  <- 21! cannot be represented by a 64-bit unsigned long long integer

		   */
	}

	{
		// 20! can still be represented by a 64-bit integer
        // 21! can not be represented by a 64-bit integer
		using Integer = unsigned long long;
		constexpr unsigned nbits = 64;
		constexpr unsigned es = 2;
		using Posit = posit<nbits, es>;
		int columnWidth = 40;
		GenerateFactorialTableComparison(upperbound, Integer(1), Posit(1), columnWidth);
		/*
+---------------
unsigned __int64 and class sw::universal::posit<64,2>
  i                                 integer(N!)                                 posit(N!)                             abs(error)
	2                                         2                                         2                                       0
	3                                         6                                         6                                       0
	4                                        24                                        24                                       0
	5                                       120                                       120                                       0
	6                                       720                                       720                                       0
	7                                      5040                                      5040                                       0
	8                                     40320                                     40320                                       0
	9                                    362880                                    362880                                       0
   10                                   3628800                                   3628800                                       0
   11                                  39916800                                  39916800                                       0
   12                                 479001600                                 479001600                                       0
   13                                6227020800                                6227020800                                       0
   14                               87178291200                               87178291200                                       0
   15                             1307674368000                           1.307674368e+12                                       0
   16                            20922789888000                          2.0922789888e+13                                       0
   17                           355687428096000                         3.55687428096e+14                                       0
   18                          6402373705728000                         6.40237370573e+15                                       0
   19                        121645100408832000                         1.21645100409e+17                                       0
   20                       2432902008176640000                         2.43290200818e+18                                       0
   21                      14197454024290336768                         5.10909421717e+19                     4974081987435560960  <- 21! cannot be represented by a 64-bit unsigned long long integer

		*/
	}

	upperbound = 30;
	{
		using Integer = sw::universal::integer<128>;
		constexpr unsigned nbits = 64;
		constexpr unsigned es = 3;
		using Posit = posit<nbits, es>;
		int columnWidth = 40;
		GenerateFactorialTableComparison(upperbound, Integer(1), Posit(1), columnWidth);
		/*
+---------------
class sw::universal::integer<128> and class sw::universal::posit<64,3>
  i                                 integer(N!)                                 posit(N!)                             abs(error)
	2                                         2                                         2                                       0
	3                                         6                                         6                                       0
	4                                        24                                        24                                       0
	5                                       120                                       120                                       0
	6                                       720                                       720                                       0
	7                                      5040                                      5040                                       0
	8                                     40320                                     40320                                       0
	9                                    362880                                    362880                                       0
   10                                   3628800                                   3628800                                       0
   11                                  39916800                                  39916800                                       0
   12                                 479001600                                 479001600                                       0
   13                                6227020800                                6227020800                                       0
   14                               87178291200                               87178291200                                       0
   15                             1307674368000                           1.307674368e+12                                       0
   16                            20922789888000                          2.0922789888e+13                                       0
   17                           355687428096000                         3.55687428096e+14                                       0
   18                          6402373705728000                         6.40237370573e+15                                       0
   19                        121645100408832000                         1.21645100409e+17                                       0
   20                       2432902008176640000                         2.43290200818e+18                                       0
   21                      51090942171709440000                         5.10909421717e+19                                       0
   22                    1124000727777607680000                         1.12400072778e+21                                       0
   23                   25852016738884976640000                         2.58520167389e+22                                 9961472  <- not enough precision with a posit<64,3>
   24                  620448401733239439360000                         6.20448401733e+23                               775946240
   25                15511210043330985984000000                         1.55112100433e+25                             32283557888
   26               403291461126605635584000000                         4.03291461127e+26                           1457847795712
   27             10888869450418352160768000000                         1.08888694504e+28                          21769704439808
   28            304888344611713860501504000000                         3.04888344612e+29                         891026701025280
   29           8841761993739701954543616000000                         8.84176199374e+30                        5685423061860352
		*/
	}

	{
		using Integer = sw::universal::integer<128>;
		constexpr unsigned nbits = 128;
		constexpr unsigned es = 4;
		using Posit = posit<nbits, es>;
		int columnWidth = 40;
		GenerateFactorialTableComparison(upperbound, Integer(1), Posit(1), columnWidth);
		/*
+---------------
class sw::universal::integer<128> and class sw::universal::posit<128,4>
  i                                 integer(N!)                                 posit(N!)                             abs(error)
	2                                         2                                         2                                       0
	3                                         6                                         6                                       0
	4                                        24                                        24                                       0
	5                                       120                                       120                                       0
	6                                       720                                       720                                       0
	7                                      5040                                      5040                                       0
	8                                     40320                                     40320                                       0
	9                                    362880                                    362880                                       0
   10                                   3628800                                   3628800                                       0
   11                                  39916800                                  39916800                                       0
   12                                 479001600                                 479001600                                       0
   13                                6227020800                                6227020800                                       0
   14                               87178291200                               87178291200                                       0
   15                             1307674368000                           1.307674368e+12                                       0
   16                            20922789888000                          2.0922789888e+13                                       0
   17                           355687428096000                         3.55687428096e+14                                       0
   18                          6402373705728000                         6.40237370573e+15                                       0
   19                        121645100408832000                         1.21645100409e+17                                       0
   20                       2432902008176640000                         2.43290200818e+18                                       0
   21                      51090942171709440000                         5.10909421717e+19                                       0
   22                    1124000727777607680000                         1.12400072778e+21                                       0
   23                   25852016738884976640000                         2.58520167389e+22                                       0
   24                  620448401733239439360000                         6.20448401733e+23                                       0
   25                15511210043330985984000000                         1.55112100433e+25                                       0
   26               403291461126605635584000000                         4.03291461127e+26                                       0
   27             10888869450418352160768000000                         1.08888694504e+28                                       0
   28            304888344611713860501504000000                         3.04888344612e+29                                       0
   29           8841761993739701954543616000000                         8.84176199374e+30                                       0 <-- enough precision with a posit<128,4>
		*/
	}

	// restore the previous ostream precision
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

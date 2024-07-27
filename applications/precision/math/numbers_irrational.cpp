// numbers_irrational.cpp: experiments with irrational numbers and their approximations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/sequences/fibonacci.hpp>

/*
The most 'irrational' number of all is the golden ration, phi: phi = 1 + 1/phi
The second most is sqrt(2), which has a continued expansion of 1 + 1/(2 + 1/(2 + 1/(2 + ...)))
Pi is not that rational, like to find out what the 1 + 1/(3 + ... continued fraction yields
*/

/*
 we can generate the golden ratio by different means: 
 direct eval: phi = 1/2 + sqrt(5)/2
 continued fraction: pick x, calc 1/x, add 1, repeat
 evaluate the ration of the last two numbers of a Fibonacci sequence

 phi at 156 digits 
 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748475408807538689175212663386222353693179318006076672635
 */

template<typename Ty>
Ty PhiThroughFibonacciSequence(unsigned terms) {
	std::pair<Ty, Ty> fib = sw::sequences::GoldenRatio<Ty>(unsigned(terms));
	return fib.second / fib.first;
}

template<typename Ty>
void GoldenRatioTerms(unsigned terms) {
	auto p = sw::sequences::GoldenRatio<Ty>(terms);
	std::cout << p.first << " " << p.second << " : approximation to phi " << (1.0 + double(p.first / p. second)) << std::endl;
}

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	using int128 = sw::universal::integer<128>;
	auto v = sw::sequences::Fibonacci<int128>(10);
	for (auto e : v) {
		std::cout << e << ' ';
	}
	std::cout << std::endl;
	for (unsigned t = 2; t < 10; ++t) {
		auto p = sw::sequences::GoldenRatio<int128>(t);
		std::cout << p.first << " " << p.second << '\n';
	}

	std::string phi = "1.618033988749894848204586834365638117720309179805762862135448622705260462818902449707207204189391137484754088075";

	// use an adaptive precision floating point oracle as reference
	std::cout << "oracle                       : " << phi << '\n';
	std::cout << "posit<128,4>     golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< posit<128, 4> >(100) << '\n';
	std::cout << "posit<256,5>     golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< posit<256, 5> >(100) << '\n';
	
	GoldenRatioTerms<sw::universal::integer<256>>(100);
	GoldenRatioTerms<sw::universal::posit<128,4>>(100);
	// why are we not seeing an improvement in the approximation for Fib pair for increasing n?
	GoldenRatioTerms<sw::universal::posit<256,5>>(200);
	GoldenRatioTerms<sw::universal::posit<512,6>>(300);

	std::cout << "\n\nEnumerating increasingly accurate approximations\n";
	using int256 = sw::universal::integer<256>;
	std::streamsize precision = std::cout.precision();
	for (unsigned i = 40; i < 50; i++) {
		auto p = sw::sequences::GoldenRatio<int256>(i);
		std::cout << "Using " << p.first << " " << p.second << '\n';
		std::cout << "approximation to golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< float >(i) << '\n';
		std::cout << "approximation to golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< double >(i) << '\n';
		std::cout << "approximation to golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< posit<32, 2> >(i) << '\n';
		std::cout << "approximation to golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< posit<64, 3> >(i) << '\n';
		std::cout << "approximation to golden ratio: " << std::setprecision(27) << PhiThroughFibonacciSequence< posit<128, 4> >(i) << '\n';
	}
	std::cout << std::setprecision(precision);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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

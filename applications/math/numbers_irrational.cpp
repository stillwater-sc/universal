// numbers_irrational.cpp: experiments with irrational numbers and their approximations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/integer/integer>
#include <universal/posit/posit>
#include <universal/sequences/sequences.hpp>

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
	std::cout << p.first << " " << p.second << std::endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	using int128 = sw::unum::integer<128>;
	auto v = sw::sequences::Fibonacci<int128>(10);
	for (auto e : v) {
		cout << e << ' ';
	}
	cout << endl;
	for (unsigned t = 2; t < 10; ++t) {
		auto p = sw::sequences::GoldenRatio<int128>(t);
		cout << p.first << " " << p.second << endl;
	}

	std::string phi = "1.618033988749894848204586834365638117720309179805762862135448622705260462818902449707207204189391137484754088075";

	// use an adaptive precision floating point oracle as reference
	cout << "oracle                       : " << phi << endl;
	cout << "posit<128,4>     golden ratio: " << setprecision(27) << PhiThroughFibonacciSequence< posit<128, 4> >(100) << endl;
	cout << "posit<256,5>     golden ratio: " << setprecision(27) << PhiThroughFibonacciSequence< posit<256, 5> >(100) << endl;
	
	GoldenRatioTerms<sw::unum::integer<256>>(100);
	GoldenRatioTerms<sw::unum::posit<128,4>>(100);
	GoldenRatioTerms<sw::unum::posit<256,5>>(100);
	GoldenRatioTerms<sw::unum::posit<512,6>>(100);

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

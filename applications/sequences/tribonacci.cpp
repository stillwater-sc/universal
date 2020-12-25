// fibonacci.cpp: experiments with representing Fibonacci sequences
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/integer/integer>
#include <universal/sequences/tribonacci.hpp>
#include <universal/integer/integer.hpp>
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::sequences;

	int nrOfFailedTestCases = 0;

	{
		using Scalar = sw::unum::integer<64>;
		constexpr unsigned N = 10;
		auto v = Tribonacci<Scalar>(N);
		cout << "Tribonacci Sequence: " << v.size() << endl;
		for (auto e: v) { cout << e << '\n'; }

		for (unsigned n = 1; n <= N; ++n) {
			cout << setw(3) << n << " : " << TribonacciNumber<Scalar>(n) << endl;
		}
	}

	// enumerate till we exhaust the number system
	{
		constexpr size_t N = 256;
		using Scalar = sw::unum::integer<N,uint32_t>;

		unsigned next = 20;
		Scalar tri_n_minus_1 = TribonacciNumber<Scalar>(next++);
		Scalar tri_n         = TribonacciNumber<Scalar>(next);
		while (tri_n > tri_n_minus_1) {
			tri_n_minus_1 = tri_n;
			tri_n = TribonacciNumber<Scalar>(++next);
		}
		cout << "Largest Tribonacci number that can be represented by " << typeid(Scalar).name() << " is\n";
		cout << tri_n_minus_1 << endl;
		std::string rep = to_string(tri_n_minus_1);
		cout << "Number of digits: " << rep.size() << "    binary size relates to decimal size as " << N << "/3.3 ~ " << int(float(N)/3.3f) << " digits\n";
		cout << tri_n << endl;
		rep = to_string(tri_n);
		cout << "Number of digits: " << rep.size() << endl;
	}
	//streamsize precision = cout.precision();
	// ...
	//cout << setprecision(precision);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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

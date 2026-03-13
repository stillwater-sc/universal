//  quires.cpp : test suite for quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// type definitions for the important types, posit<> and quire<>
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/fdp_generalized.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "Quire use cases\n";

	// generate table of quire sizes for standard posit configurations
	quire<posit<  8, 0>,   7>  quire8  ; std::cout << "quire<posit<  8,0>,  7>  " << quire8.total_bits() << " bits\n";
	quire<posit< 16, 1>,  15>  quire16 ; std::cout << "quire<posit< 16,1>, 15>  " << quire16.total_bits() << " bits\n";
	quire<posit< 32, 2>,  31>  quire32 ; std::cout << "quire<posit< 32,2>, 31>  " << quire32.total_bits() << " bits\n";
	quire<posit< 64, 3>,  63>  quire64 ; std::cout << "quire<posit< 64,3>, 63>  " << quire64.total_bits() << " bits\n";
	quire<posit<128, 4>, 127>  quire128; std::cout << "quire<posit<128,4>,127>  " << quire128.total_bits() << " bits\n";
	quire<posit<256, 5>,   7>  quire256; std::cout << "quire<posit<256,5>,  7>  " << quire256.total_bits() << " bits\n";

#if MANUAL_TESTING

	{
		std::cout << "Compare value and quire content\n";

		float v = 2.6226e-05f;
		using Posit = posit<16, 1>;
		quire<Posit, 2> q;
		Posit p1, p2, argA, argB;

		p1 = v;
		q = p1;
		p2 = quire_resolve(q);
		argA = -0.016571;
		argB = 0.000999451;
		float diff = v - float(p1);
		std::cout << "diff       = " << std::setprecision(17) << diff << '\n';

		std::cout << "quire      = " << q << '\n';
		std::cout << "v as posit = " << pretty_print(p1) << '\n';
		std::cout << "q as posit = " << p2 << '\n';
		q += quire_mul(argA, argB);
		std::cout << "quire      = " << q << '\n';
		p2 = quire_resolve(q);
		std::cout << "q as posit = " << p2 << '\n';
	}

	{
		std::cout << "\nNothing prohibiting us from creating quires for float and double arithmetic\n";
		float f = 1.555555555555e-10f;
		blocktriple<23, BlockTripleOperator::REP, uint32_t> vf(f);
		quire<posit<10, 2>, 2> fquire;
		fquire += vf;
		std::cout << "float:  " << std::setw(15) << f << " " << fquire << '\n';

		double d = 1.555555555555e16;
		blocktriple<52, BlockTripleOperator::REP, uint32_t> vd(d);
		quire<posit<10, 2>, 2> dquire;
		dquire += vd;
		std::cout << "double: " << std::setw(15) << d << " " << dquire << std::endl;
	}

	std::cout << std::endl;

	{
		std::cout << "testing carry/borrow propagation\n";
		constexpr unsigned nbits = 4;
		constexpr unsigned es = 1;
		constexpr unsigned capacity = 2;
		using Posit = posit<nbits, es>;
		quire<Posit, capacity> q;

		long double dmax = (long double)Posit(SpecificValue::maxpos);
		long double dmin = (long double)Posit(SpecificValue::minpos);
		std::cout << "maxpos = " << dmax << " maxpos^2 = " << dmax*dmax << std::endl;
		std::cout << "minpos = " << dmin << " minpos^2 = " << dmin*dmin << std::endl;

		std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
		q.clear();
		blocktriple<23, BlockTripleOperator::REP, uint32_t> v(64.0f);
		blocktriple<23, BlockTripleOperator::REP, uint32_t> c(static_cast<float>(dmax*dmax));
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << " <- entering capacity bits\n";
		q += c;		std::cout << q << " <- adding maxpos^2\n";
		q += c;     std::cout << q << " <- flipping another capacity bit\n";
		blocktriple<23, BlockTripleOperator::REP, uint32_t> neg_c(c);
		neg_c.setsign(true);
		q += neg_c;	std::cout << q << " <- subtracting maxpos^2\n";
		q += neg_c;	std::cout << q << " <- subtracting maxpos^2\n";
		blocktriple<23, BlockTripleOperator::REP, uint32_t> neg_v(v);
		neg_v.setsign(true);
		q += neg_v;	std::cout << q << " <- removing the capacity bit\n";
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << '\n';
		q += neg_v;	std::cout << q << " <- should be zero\n";

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators\n";
		q.clear();
		blocktriple<23, BlockTripleOperator::REP, uint32_t> half(0.5f);
		blocktriple<23, BlockTripleOperator::REP, uint32_t> neg_half(0.5f);
		neg_half.setsign(true);
		q += half;		std::cout << q << '\n';
		q += half;		std::cout << q << '\n';
		q += half;		std::cout << q << '\n';
		q += half;		std::cout << q << '\n';
		q += neg_half;	std::cout << q << '\n';
		q += neg_half;	std::cout << q << '\n';
		q += neg_half;	std::cout << q << '\n';
		q += neg_half;	std::cout << q << " <- should be zero\n";

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators\n";
		q.clear();
		blocktriple<23, BlockTripleOperator::REP, uint32_t> w(3.9375f);  // 3.875 + 0.0625 = 3.9375
		blocktriple<23, BlockTripleOperator::REP, uint32_t> neg_w(3.9375f);
		neg_w.setsign(true);
		q += w;		std::cout << q << '\n';
		q += w;		std::cout << q << '\n';
		q += w;		std::cout << q << '\n';
		q += neg_w;	std::cout << q << '\n';
		q += neg_w;	std::cout << q << '\n';
		q += neg_w;	std::cout << q << " <- should be zero\n";
	}

	std::cout << std::endl;

#else

	std::cout << "Quire validation\n";
	// TODO: add generalized quire validation tests

#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

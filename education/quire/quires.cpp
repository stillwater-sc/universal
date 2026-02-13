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
#include <universal/number/posit/quire.hpp>
#include <universal/verification/quire_test_suite.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::cout << "Quire use cases\n";
	std::string tag = "Quire Accumulation failed";

	// generate table of quire sizes for standard posit configurations
	quire<  8, 0, 7>   quire8  ; std::cout << "quire<  8,0,7>   " << quire8.total_bits() << " bits\n";
	quire< 16, 1, 15>  quire16 ; std::cout << "quire< 16,1,15>  " << quire16.total_bits() << " bits\n";
	quire< 32, 2, 31>  quire32 ; std::cout << "quire< 32,2,31>  " << quire32.total_bits() << " bits\n";
	quire< 64, 3, 63>  quire64 ; std::cout << "quire< 64,3,63>  " << quire64.total_bits() << " bits\n";
	quire<128, 4, 127> quire128; std::cout << "quire<128,4,127> " << quire128.total_bits() << " bits\n";
	quire<256, 5, 7>   quire256; std::cout << "quire<256,5,7>   " << quire256.total_bits() << " bits\n";

	/*
		quire<  8, 0, 0>   25 bits
		quire< 16, 1, 0>   113 bits
		quire< 32, 2, 0>   481 bits
		quire< 64, 3, 0>   1985 bits
		quire<128, 4, 0>   8065 bits
		quire<256, 5, 0>   32513 bits
	*/

#if MANUAL_TESTING

	{
		std::cout << "Compare value and quire content\n";

		float v = 2.6226e-05f;
		sw::universal::quire<16, 1, 2> q;
		sw::universal::posit<16, 1> p1, p2, argA, argB;

		p1 = v;
		q = posit_to_value(p1);
		convert(q.to_value(), p2);
		argA = -0.016571;
		argB = 0.000999451;
		float diff = v - float(p1);
		std::cout << "diff       = " << std::setprecision(17) << diff << '\n';

		std::cout << "quire      = " << q << '\n';
		std::cout << "v as posit = " << pretty_print(p1) << '\n';
		std::cout << "q as posit = " << p2 << '\n';
		q += quire_mul(argA, argB);
		std::cout << "quire      = " << q << '\n';
		convert(q.to_value(), p2);
		std::cout << "q as posit = " << p2 << '\n';
	}

	{
		std::cout << "Generate value assignments\n";
		const size_t nbits = 4;
		const size_t es = 1;
		const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
		const size_t fbits = 5;

		//GenerateUnsignedIntAssignments<nbits, es, capacity>();
		//GenerateSignedIntAssignments<nbits, es, capacity>();
		//GenerateUnsignedIntAssignments<8, 2, capacity>();

		GenerateValueAssignments<nbits, es, capacity, fbits>();
	}

	std::cout << '\n';

	{
		std::cout << "Nothing prohibiting us from creating quires for float and double arithmetic\n";
		float f = 1.555555555555e-10f;
		internal::value<23> vf(f);
		quire<10, 2, 2> fquire;
		fquire += vf;
		std::cout << "float:  " << std::setw(15) << f << " " << fquire << '\n';

		double d = 1.555555555555e16;
		internal::value<52> vd(d);
		quire<10, 2, 2> dquire;
		dquire += vd;
		std::cout << "double: " << std::setw(15) << d << " " << dquire << std::endl;
	}


	/* pattern to use posits with a quire
	posit<10, 2> p = 1.555555555555e16;
	quire<10, 2, 2> pquire(p.convert_to_scientific_notation());
	std::cout << "posit:  " << setw(15) << d << " " << dquire << std::endl;
	*/
	std::cout << std::endl;

	{
		std::cout << "testing carry/borrow propagation\n";
		const size_t nbits = 4;
		const size_t es = 1;
		const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
		// nbits = 4, es = 1, capacity = 2
		//  17 16   15 14 13 12 11 10  9  8    7  6  5  4  3  2  1  0
		// [ 0  0    0  0  0  0  0  0  0  0    0  0  0  0  0  0  0  0 ]
		quire<nbits, es, capacity> q;
		internal::value<5> maxpos, maxpos_squared, minpos, minpos_squared;
		long double dmax = (long double)sw::universal::posit<nbits, es>(sw::universal::SpecificValue::maxpos);
		maxpos = dmax;
		maxpos_squared = dmax*dmax;
		std::cout << "maxpos * maxpos = " << to_triple(maxpos_squared) << std::endl;
		long double dmin = (long double)sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos);
		minpos = dmin;
		minpos_squared = dmin*dmin;
		std::cout << "minpos * minpos = " << to_triple(minpos_squared) << std::endl;
		internal::value<5> c(maxpos_squared);

		std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
		q.clear();
		internal::value<5> v(64);
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
		q += -c;	std::cout << q << " <- subtracting maxpos^2\n";
		q += -c;	std::cout << q << " <- subtracting maxpos^2\n";
		q += -v;	std::cout << q << " <- removing the capacity bit\n";
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << " <- should be zero\n";

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators\n";
		q.clear();
		v = 0.5;
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << " <- should be zero\n";

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators\n";
		q.clear();  // equivalent to q = 0, but more articulate/informative
		v = 3.875 + 0.0625; std::cout << "v " << to_triple(v) << '\n'; // the input value is 11.1111 so hidden + 5 fraction bits
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += v;		std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << '\n';
		q += -v;	std::cout << q << " <- should be zero\n";
	}

	std::cout << std::endl;

#else

	std::cout << "Quire validation\n";
	std::vector< posit<8, 0> > v;
	TestQuireAccumulationResult(ValidateQuireAccumulation<8,0,5>(true, v), "quire<8,0,5>");  // <-- this is segfaulting

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


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

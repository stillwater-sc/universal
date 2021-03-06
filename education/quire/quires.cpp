//  quires.cpp : test suite for quires
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// type definitions for the important types, posit<> and quire<>
#include <universal/number/posit/posit>
#include <universal/verification/quire_test_suite.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Quire use cases" << endl;
	std::string tag = "Quire Accumulation failed";

	// generate table of quire sizes for standard posit configurations
	quire<  8, 0, 7>   quire8  ; cout << "quire<  8,0,7>   " << quire8.total_bits() << " bits\n";
	quire< 16, 1, 15>  quire16 ; cout << "quire< 16,1,15>  " << quire16.total_bits() << " bits\n";
	quire< 32, 2, 31>  quire32 ; cout << "quire< 32,2,31>  " << quire32.total_bits() << " bits\n";
	quire< 64, 3, 63>  quire64 ; cout << "quire< 64,3,63>  " << quire64.total_bits() << " bits\n";
	quire<128, 4, 127> quire128; cout << "quire<128,4,127> " << quire128.total_bits() << " bits\n";
	quire<256, 5, 7>   quire256; cout << "quire<256,5,7>   " << quire256.total_bits() << " bits\n";

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
		cout << "Compare value and quire content\n";

		float v = 2.6226e-05f;
		sw::universal::quire<16, 1, 2> q;
		sw::universal::posit<16, 1> p1, p2, argA, argB;

		p1 = v;
		q = p1.to_value();
		convert(q.to_value(), p2);
		argA = -0.016571;
		argB = 0.000999451;
		float diff = v - float(p1);
		std::cout << "diff       = " << setprecision(17) << diff << std::endl;

		std::cout << "quire      = " << q << std::endl;
		std::cout << "v as posit = " << pretty_print(p1) << std::endl;
		std::cout << "q as posit = " << p2 << std::endl;
		q += quire_mul(argA, argB);
		std::cout << "quire      = " << q << std::endl;
		convert(q.to_value(), p2);
		std::cout << "q as posit = " << p2 << std::endl;
	}

	{
		cout << "Generate value assignments\n";
		const size_t nbits = 4;
		const size_t es = 1;
		const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
		const size_t fbits = 5;

		//GenerateUnsignedIntAssignments<nbits, es, capacity>();
		//GenerateSignedIntAssignments<nbits, es, capacity>();
		//GenerateUnsignedIntAssignments<8, 2, capacity>();

		GenerateValueAssignments<nbits, es, capacity, fbits>();
	}

	std::cout << endl;

	{
		std::cout << "Nothing prohibiting us from creating quires for float and double arithmetic" << std::endl;
		float f = 1.555555555555e-10f;
		internal::value<23> vf(f);
		quire<10, 2, 2> fquire;
		fquire += vf;
		std::cout << "float:  " << setw(15) << f << " " << fquire << std::endl;

		double d = 1.555555555555e16;
		internal::value<52> vd(d);
		quire<10, 2, 2> dquire;
		dquire += vd;
		std::cout << "double: " << setw(15) << d << " " << dquire << std::endl;
	}


	/* pattern to use posits with a quire
	posit<10, 2> p = 1.555555555555e16;
	quire<10, 2, 2> pquire(p.convert_to_scientific_notation());
	std::cout << "posit:  " << setw(15) << d << " " << dquire << std::endl;
	*/
	std::cout << std::endl;

	{
		cout << "testing carry/borrow propagation\n";
		const size_t nbits = 4;
		const size_t es = 1;
		const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
		// nbits = 4, es = 1, capacity = 2
		//  17 16   15 14 13 12 11 10  9  8    7  6  5  4  3  2  1  0
		// [ 0  0    0  0  0  0  0  0  0  0    0  0  0  0  0  0  0  0 ]
		quire<nbits, es, capacity> q;
		internal::value<5> maxpos, maxpos_squared, minpos, minpos_squared;
		long double dmax = sw::universal::maxpos_value<nbits, es>();
		maxpos = dmax;
		maxpos_squared = dmax*dmax;
		std::cout << "maxpos * maxpos = " << to_triple(maxpos_squared) << std::endl;
		long double dmin = sw::universal::minpos_value<nbits, es>();
		minpos = dmin;
		minpos_squared = dmin*dmin;
		std::cout << "minpos * minpos = " << to_triple(minpos_squared) << std::endl;
		internal::value<5> c(maxpos_squared);

		std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
		q.clear();
		internal::value<5> v(64);
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << " <- entering capacity bits" << std::endl;
		q += c;		std::cout << q << " <- adding maxpos^2" << std::endl;
		q += c;     std::cout << q << " <- flipping another capacity bit" << std::endl;
		q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
		q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
		q += -v;	std::cout << q << " <- removing the capacity bit" << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << " <- should be zero" << std::endl;

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
		q.clear();
		v = 0.5;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << " <- should be zero" << std::endl;

		std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
		q.clear();  // equivalent to q = 0, but more articulate/informative
		v = 3.875 + 0.0625; std::cout << "v " << to_triple(v) << std::endl; // the input value is 11.1111 so hidden + 5 fraction bits
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += v;		std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << std::endl;
		q += -v;	std::cout << q << " <- should be zero" << std::endl;
	}

	std::cout << std::endl;

#else

	std::cout << "Quire validation" << std::endl;
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

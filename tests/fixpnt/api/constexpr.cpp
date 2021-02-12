// constexpr.cpp: compile time tests for fixed-point constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/math_functions.hpp>

/*
fixpnt& operator=(double rhs) noexcept {
		clear();
		if (rhs == 0.0) {
			return *this;
		}
		if (arithmetic == Saturating) {	// check if the value is in the representable range
			fixpnt<nbits, rbits, arithmetic, BlockType> a;
			a.setmaxpos();
			if (rhs >= float(a)) { return *this = a; } // set to max pos value
			a.setmaxneg();
			if (rhs <= float(a)) { return *this = a; } // set to max neg value
		}

		double_decoder decoder;
		decoder.d = rhs;
		uint64_t raw = (uint64_t(1) << 52) | decoder.parts.fraction;
		int radixPoint = 52 - (int(decoder.parts.exponent) - 1023);  // move radix point to the right if scale > 0, left if scale < 0
		// our fixed-point has its radixPoint at rbits
		int shiftRight = radixPoint - int(rbits);
		// do we need to round?
		if (shiftRight > 0) {
			// yes, round the raw bits
			// collect guard, round, and sticky bits
			// this same logic will work for the case where
			// we only have a guard bit  and no round and sticky bits
			// because the mask logic will make round and sticky both 0
			uint64_t mask = (uint64_t(1) << (shiftRight - 1));
			bool guard = (mask & raw);
			mask >>= 1;
			bool round = (mask & raw);
			if (shiftRight > 1) {
				mask = (0xFFFFFFFFFFFFFFFF << (shiftRight - 2));
				mask = ~mask;
			}
			else {
				mask = 0;
			}
			bool sticky = (mask & raw);

			raw >>= shiftRight;  // shift out the bits we are rounding away
			bool lsb = (raw & 0x1);
			//  ... lsb | guard  round sticky   round
			//       x     0       x     x       down
			//       0     1       0     0       down  round to even
			//       1     1       0     0        up   round to even
			//       x     1       0     1        up
			//       x     1       1     0        up
			//       x     1       1     1        up
			if (guard) {
				if (lsb && (!round && !sticky)) ++raw; // round to even
				if (round || sticky) ++raw;
			}
		}
		raw = (decoder.parts.sign == 0) ? raw : (~raw + 1); // map to two's complement
		set_raw_bits(raw);
		return *this;
	}
 */
// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

constexpr double pi = 3.14159265358979323846;

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	cout << "fixed-point constexpr tests" << endl;
	
	{
		fixpnt<8, 4> a(pi);
		cout << a << endl;
	}
#ifdef CONSTEXPR
	{
		// decorated constructors
		{
			constexpr fixpnt<8, 4> a(1l);  // signed long
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a(1ul);  // unsigned long
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a(1.0f);  // float
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a(1.0);   // double
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a(1.0l);  // long double
			cout << a << endl;
		}
	}
	{
		// assignment operators
		{
			constexpr fixpnt<8, 4> a = 1l;  // signed long
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a = 1ul;  // unsigned long
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a = 1.0f;  // float
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a = 1.0;   // double
			cout << a << endl;
		}
		{
			constexpr fixpnt<8, 4> a = 1.0l;  // long double
			cout << a << endl;
		}
	}
#endif


	if (nrOfFailedTestCases > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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

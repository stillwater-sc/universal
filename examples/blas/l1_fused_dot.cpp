// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "blas_operators.hpp"

template<typename Ty>
Ty minValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
	return *it;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//constexpr size_t nbits = 16;
	//constexpr size_t es = 1;
	//constexpr size_t capacity = 6;   // 2^3 accumulations of maxpos^2
	//constexpr size_t vecSizePwr = 5;
	//constexpr size_t vecSize = (size_t(1) << vecSizePwr);

	// generate an interesting vector x with 0.5 ULP round-off errors in each product
	// that the fused-dot product will be able to resolve
	// by progressively adding smaller values, a regular dot product loses these bits due to canceleation.
	// but a fused dot product leveraging a quire will be able to resolve these.
	//float eps      = std::numeric_limits<float>::epsilon();
	//float epsminus = 1.0f - eps;
	//float epsplus  = 1.0f + eps;


	{
		using IEEEType = float;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		vector<IEEEType> xieee = { a1, a2, a3, a4 };
		vector<IEEEType> yieee = { b1, b2, b3, b4 };

		printVector(cout, "a: ", xieee);
		printVector(cout, "b: ", yieee);

		cout << endl << endl;

		cout << setprecision(17);
		cout << "IEEE float   BLAS dot(x,y)  : " << dot(xieee.size(), xieee, 1, yieee, 1) << endl;
		cout << setprecision(5);
	}

	{
		using IEEEType = double;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		vector<IEEEType> xieee = { a1, a2, a3, a4 };
		vector<IEEEType> yieee = { b1, b2, b3, b4 };

		cout << setprecision(17);
		cout << "IEEE double  BLAS dot(x,y)  : " << dot(xieee.size(), xieee, 1, yieee, 1) << endl;
		cout << setprecision(5);
	}

	{
		// a little verbose but enabling different precisions to be injected
		// float, double, long double
		// so that you can convince yourself that this is a property of posits and quires
		// and not some input precision shenanigans. The magic is all in the quire
		// accumulating UNROUNDED multiplies: that gives you in affect double the 
		// fraction bits.
		using IEEEType = float;
		IEEEType a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;
		IEEEType b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;

		using PositType = posit<32, 2>;
		vector<PositType> xposit = { a1, a2, a3, a4 };
		vector<PositType> yposit = { b1, b2, b3, b4 };

		cout << "posit<32,2> fused dot(x,y)  : " << fused_dot(xposit.size(), xposit, 1, yposit, 1) << "           <----- correct answer is 2" << endl;
		cout << setprecision(5);
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::posit_internal_exception& err) {
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

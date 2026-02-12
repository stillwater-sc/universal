// exception.cpp : special posit/quire arithmetic exceptions to be used by applications
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit library to enable arithmetic exceptions
// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>

int main()
try {
	using namespace sw::universal;

	constexpr unsigned nbits = 16;
	constexpr unsigned es = 1;
	constexpr unsigned capacity = 2;

	posit<nbits, es> pa, pb, pc;

	try {
		pa = 1.0f;
		pb = 0;
		pc = pa / pb;
		std::cout << "Incorrect: division by zero exception didn't fire\n"; // just in case the exception doesn't fire
	}
	catch (const posit_divide_by_zero& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	try {
		pa = 1.0f;
		pb.setnar();
		pc = pa / pb;
		std::cout << "Incorrect: division by nar exception didn't fire\n";
	}
	catch (const posit_divide_by_nar& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setnar();
		pb = 1.0f;
		pc = pa / pb;
		std::cout << "Incorrect: numerator is nar exception didn't fire\n";
	}
	catch (const posit_numerator_is_nar& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setnar();
		pb = 1.0f;
		pc = pa + pb;
		std::cout << "Incorrect: operand is nar exception didn't fire\n";
	}
	catch (const posit_operand_is_nar& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setnar();
		pb = 1.0f;
		pc = pa - pb;
		std::cout << "Incorrect: operand is nar exception didn't fire\n";
	}
	catch (const posit_operand_is_nar& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setnar();
		pb = 1.0f;
		pc = pa * pb;	// TODO: operator *= throws the same exception, but for some reason we can't catch it here
		std::cout << "Incorrect: operand is nar exception didn't fire\n";
	}
	catch (const posit_operand_is_nar& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}
	catch (const posit_arithmetic_exception& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Why can't I catch operand_is_nar exception for multiply?\n";
	}

	quire<nbits, es, capacity> q1, q2, q3;
	internal::value<pa.mbits> v;
	// report some parameters about the posit and quire configuration
	int max_scale = q1.max_scale();
	v = std::pow(2.0, max_scale+1);
	try {
		q1 += v; // v is outside the max scale of the quire
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	int min_scale = q1.min_scale();
	v = std::pow(2.0, min_scale - 1);
	try {
		q1 += v; // v is outside the max scale of the quire
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Correctly caught exception: " << err.what() << std::endl;
	}

	//value<pa.mbits> unrounded = sw::universal::quire_mul(minpos<nbits, es>(), minpos<nbits, es>());

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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

// signalling_nar.cpp : all arithmetic errors become silent signalling NaRs
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <posit>

int main()
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	constexpr size_t capacity = 2;

	posit<nbits, es> pa, pb, pc;

	try {
		pa = 1.0f;
		pb = 0;
		pc = pa / pb;
		cout << "Correct: division by zero exception didn't fire as it is not enabled" << endl;
	}
	catch (const divide_by_zero& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	try {
		pa = 1.0f;
		pb.setToNaR();
		pc = pa / pb;
		cout << "Correct: division by nar exception didn't fire as it is not enabled" << endl;
	}
	catch (const divide_by_nar& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setToNaR();
		pb = 1.0f;
		pc = pa / pb;
		cout << "Correct: numerator is nar exception didn't fire as it is not enabled" << endl;
	}
	catch (const numerator_is_nar& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setToNaR();
		pb = 1.0f;
		pc = pa + pb;
		cout << "Correct: operand is nar exception didn't fire as it is not enabled" << endl;
	}
	catch (const operand_is_nar& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setToNaR();
		pb = 1.0f;
		pc = pa - pb;
		cout << "Correct: operand is nar exception didn't fire as it is not enabled" << endl;
	}
	catch (const operand_is_nar& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	try {
		pa.setToNaR();
		pb = 1.0f;
		pc = pa * pb;
		cout << "Correct: operand is nar exception didn't fire as it is not enabled" << endl;
	}
	catch (const operand_is_nar& err) {
		std::cerr << "Incorrectly caught exception: " << err.what() << std::endl;
	}

	quire<nbits, es, capacity> q1, q2, q3;
	value<pa.mbits> v;
	// report some parameters about the posit and quire configuration
	int max_scale = q1.max_scale();
	v = std::pow(2.0, max_scale+1);
	try {
		q1 += v; // v is outside the max scale of the quire
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Correct: caught exception: " << err.what() << std::endl;
	}

	int min_scale = q1.min_scale();
	v = std::pow(2.0, min_scale - 1);
	try {
		q1 += v; // v is outside the max scale of the quire
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Correct: caught exception: " << err.what() << std::endl;
	}

	// value<pa.mbits> unrounded = sw::unum::quire_mul(minpos<nbits, es>(), minpos<nbits, es>());

	return EXIT_SUCCESS;
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

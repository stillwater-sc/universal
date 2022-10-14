// reverse_view.cpp: tester for the reverse container view for range based loops
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <list>
#include <array>
#include <universal/number/integer/integer.hpp>
#include <universal/utility/reverse_view.hpp>

// receive a float and print the components of a double representation
int main()
try {
	using namespace sw::universal;
	using Int = integer<10>;

	auto list = std::list<Int>{1,2,3,4,5};
	for (auto& element: list) {
		std::cout << element++ << ' ';
	}
	std::cout << std::endl;
	for (auto& element : reverse(list)) {
		std::cout << --element << ' ';
	}
	std::cout << std::endl;
	for (auto& element : list) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;

	// const containers
	const auto const_list = { 1, 2, 3, 4, 5 };
	for (auto& element : const_list) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;
	for (auto& element : reverse(const_list)) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;

	// temporary sequences, they need more machinery to apply a reverse view as
	// you need to copy the contents of the temporary into the reverse view
	// otherwise the sequence will have gone out of scope by the time we
	// apply the begin/end methods.
	for (auto& element : std::list<Int>{10, 20, 30, 40, 50}) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;
	for (auto& element : reverse(std::list<Int>{10, 20, 30, 40, 50})) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;

	// arrays
	std::array<Int, 5> array = { 100, 200, 300, 400, 500 };
	for (auto& element : array) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;
	for (auto& element : reverse(array)) {
		std::cout << element << ' ';
	}
	std::cout << std::endl;

	return EXIT_SUCCESS;
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

// reverse_view.cpp: tester for the reverse container view for range based loops
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <list>
#include <array>
#include <universal/utility/reverse_view.hpp>

// receive a float and print the components of a double representation
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	auto list = std::list<int>{1,2,3,4,5};
	for (auto& element: list) {
		cout << element++ << ' ';
	}
	cout << endl;
	for (auto& element : reverse(list)) {
		cout << --element << ' ';
	}
	cout << endl;
	for (auto& element : list) {
		cout << element << ' ';
	}
	cout << endl;

	// const containers
	const auto const_list = { 1, 2, 3, 4, 5 };
	for (auto& element : const_list) {
		cout << element << ' ';
	}
	cout << endl;
	for (auto& element : reverse(const_list)) {
		cout << element << ' ';
	}
	cout << endl;

	// temporary sequences, they need more machinery to apply a reverse view as
	// you need to copy the contents of the temporary into the reverse view
	// otherwise the sequence will have gone out of scope by the time we
	// apply the begin/end methods.
	for (auto& element : std::list<int>{10, 20, 30, 40, 50}) {
		cout << element << ' ';
	}
	cout << endl;
	for (auto& element : reverse(std::list<int>{10, 20, 30, 40, 50})) {
		cout << element << ' ';
	}
	cout << endl;

	// arrays
	array<int, 5> array = { 100, 200, 300, 400, 500 };
	for (auto& element : array) {
		cout << element << ' ';
	}
	cout << endl;
	for (auto& element : reverse(array)) {
		cout << element << ' ';
	}
	cout << endl;

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

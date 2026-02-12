// type_test.cpp: type testing 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>
#include <iostream>
#include <typeinfo>

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>

typedef sw::universal::posit<32,2> posit_32_2;

template<typename T>
void test(const std::string& message)
{
    std::cout << message << std::endl;
    std::cout << sw::universal::type_tag<T>() << '\n';

    // Constructor and assignment
    T v = 8;
    posit_32_2 p(1.0);

    // Basis operators
    p += v;
    p -= v;
    p *= v;
    p /= v;

    // Logical operators
    bool b(false);
    b = (p == v); 
    std::cout << "(p == v) : " << (b ? "true" : "false") << std::endl;
    b = (p != v);
    std::cout << "(p != v) : " << (b ? "true" : "false") << std::endl;
    b = (p <  v);
    std::cout << "(p < v) : " << (b ? "true" : "false") << std::endl;
    b = (p >  v);
    std::cout << "(p > v) : " << (b ? "true" : "false") << std::endl;
    b = (p <= v);
    std::cout << "(p <= v) : " << (b ? "true" : "false") << std::endl;
    b = (p >= v);
    std::cout << "(p >= v) : " << (b ? "true" : "false") << std::endl;

    // pretty print
    std::cout << color_print(p) << " : " << p << std::endl;
}

int main()
try {
    using namespace sw::universal;

    test<size_t>("size_t");

    test<char>("char");
    test<short>("short");
    test<int>("int");
    test<long>("long");
    test<long long>("long long");

//    test<unsigned char>("unsigned char");
    test<unsigned short>("unsigned short");
    test<unsigned int>("unsigned int");
    test<unsigned long>("unsigned long");
    test<unsigned long long>("unsigned long long");
        
    test<float>("float");
    test<double>("double");
    test<long double>("long double");

    return 0;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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

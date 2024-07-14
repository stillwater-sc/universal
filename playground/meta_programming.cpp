// meta_programming.cpp: playground to experiment with meta programming techniques to generalize functions and algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

template<typename T>
struct hasSerialize {
	typedef char yes[1];
	typedef yes no[2];

	// helper to determine if serialize is a function
	template<typename U, U function>
	struct reallyHas;

	template<typename S> static yes& test(reallyHas<std::string(S::*)(), &S::serialize>* /*unused*/) { yes y{0}; return y; }
	template<typename S> static yes& test(reallyHas<std::string(S::*)() const, &S::serialize>* /*unused*/) { yes y{0}; return y; } 

	template<typename> static no& test(...) { no n{0}; return n; }

	// constant used as return value for the test
	static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

struct A {};

std::string to_string(const A& a) {
	return "I am an A";
}

struct B {
	std::string serialize() const {
		return "I am a B";
	}
};

struct C {
	std::string serialize;
};

std::string to_string(const C& c) {
	return "I am a C";
}

namespace sw {
	// typeless struct, will always fail substitution
	template<bool b, typename T = void>
	struct enable_if {};

	// specialization for type T
	template<typename T>
	struct enable_if<true, T> {
		using type = T;
	};

	template<typename T>
	typename enable_if<hasSerialize<T>::value, std::string>::type serialize(const T& obj) {
		return obj.serialize();
	}
	template<typename T>
	typename enable_if<!hasSerialize<T>::value, std::string>::type serialize(const T& obj) {
		return to_string(obj);
	}
}

int main(int argc, char** argv)
try {
	bool bSuccess = true;

	A a;
	B b;
	C c;

	/* goal
	cout << serialize(a) << endl;
	cout << serialize(b) << endl;
	cout << serialize(c) << endl;
	*/

	std::cout << hasSerialize<A>::value << '\n';
	std::cout << hasSerialize<B>::value << '\n';
	std::cout << hasSerialize<C>::value << '\n';

	// pedantic
	sw::enable_if<true, int>::type t1; // type t1 is an int
	sw::enable_if<hasSerialize<B>::value, int>::type t2; // t2 is an int
	// to get rid of warnings in g++
	t1 = t2 = 1;
	std::cout << t1 << t2 << '\n';
	// enable_if<false, int>::type t3;  doesn't compile as enable_if<false, ...> doesn't have a type type
	// enable_if<hasSerialize<A>::value, int>::type t4; doesn't compile as enable_if<false, ...> doesn't have a type type

	// with enable_if we have the indirection to dispatch the right function: serialize or to_string
	std::cout << sw::serialize(a) << '\n';
	std::cout << sw::serialize(b) << '\n';
	std::cout << sw::serialize(c) << '\n';

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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

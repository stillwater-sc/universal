// api.cpp: class interface usage patterns for blocktriple
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <typeinfo>

// BIT_CAST_SUPPORT is compiler env dependent and drives the algorith selection of ieee-754 decode
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */

#define BIT_CAST_SUPPORT 0

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

#define BIT_CAST_SUPPORT 0

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */

#define BIT_CAST_SUPPORT 1

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_ADD 1
#include <universal/internal/blocktriple/blocktriple.hpp>

/*
 BlockTriple is the unifying compute engine for any of the
 floating-point number systems, linear, tapered, compressed, etc.

 The use case of blocktriple is as an ephemeral input/operator/round/output
 data structure through the computational pipeline.
 The blocktriple enables a uniform machine to go from source number system,
 through different arithmetic operators, such as,
 add/sub/mul/div/sqrt/special function, back to the source number encoding,
 or a new target number system

 To make this fast, we need to avoid any unnecessary copies.
 This will be particularly important for precise numbers, that is,
 numbers with many fraction bits, as the cost of the copy grows
 linearly with the size of the fraction bits.

 The input step is a normalization from number system to a triple.
 A triple is (sign, scale, significant).
 The blocktriple uses a 2's complement encoded significant, that is,
 the fraction bits including the hidden bit, and additionally extended
 for the specific use case, such as inputs to ALUs or Special Function Units (SFU).

 TODO: is there an optimization that can be applied that makes this
 even faster? What about moves? Need to ping Peter Gottschling.

 The significant is the input to the ALUs and SFUs.
 For addition and subtraction the significant needs to be aligned,
 which involves a shift operation, which is expensive for multi-block
 representations.
*/

template<typename Real>
void TestConversionRounding(Real f = 511.5f)
{
	using namespace std;
	using namespace sw::universal;
	cout << "\n " << typeid(Real).name() << " conversion use case and result\n";
	cout << to_binary(f, true) << '\n';
	CONSTEXPRESSION blocktriple<6> a = f;
	cout << to_triple(a) << " : " << a << '\n';
	CONSTEXPRESSION blocktriple<7> b = f;
	cout << to_triple(b) << " : " << b << '\n';
	CONSTEXPRESSION blocktriple<8> c = f;
	cout << to_triple(c) << " : " << c << '\n';
	CONSTEXPRESSION blocktriple<9> d = f;
	cout << to_triple(d) << " : " << d << '\n';
	CONSTEXPRESSION blocktriple<10> e = f;
	cout << to_triple(e) << " : " << e << '\n';
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	std::cout << "blocktriple<> class interface tests" << std::endl;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// pick a value that rounds up to even between 6 to 10 bits of fraction
	TestConversionRounding(511.5f);
	TestConversionRounding(511.5);

	{
		cout << "\nblocktriple add\n";
		constexpr size_t abits = 7;
		blocktriple<abits> a, b, c;
		a = 1.03125f;
		b = -1.03125f;
		cout << to_triple(a) << '\n' << to_triple(b) << '\n';
		c.add(a, b);
		cout << to_triple(c) << " : " << c << '\n';
	}

#else // !MANUAL_TESTING



#endif // MANUAL_TESTING

	std::cout << "\nblocktriple API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

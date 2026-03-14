// quire_traits.cpp: compile-time validation of quire_traits for all supported number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Include the number types (forward declarations needed for traits)
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
// Include the quire_traits
#include <universal/traits/quire_traits.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "quire_traits validation";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	// ========================================================================
	// posit quire traits -- validate against known formulas
	// ========================================================================
	{
		using Scalar = posit<32, 2>;
		using QT = quire_traits<Scalar>;

		static_assert(QT::escale == 4, "posit<32,2> escale should be 4");
		static_assert(QT::range == 4 * (4 * 32 - 8), "posit<32,2> range should be 480");
		static_assert(QT::half_range == 240, "posit<32,2> half_range should be 240");
		static_assert(QT::upper_range == 241, "posit<32,2> upper_range should be 241");
		static_assert(QT::fbits == 27, "posit<32,2> fbits should be 27");
		static_assert(QT::qbits == 510, "posit<32,2> qbits with default capacity should be 510");

		std::cout << "posit<32,2> quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  half_range  = " << QT::half_range << '\n';
		std::cout << "  upper_range = " << QT::upper_range << '\n';
		std::cout << "  fbits       = " << QT::fbits << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}
	{
		using Scalar = posit<16, 1>;
		using QT = quire_traits<Scalar>;

		static_assert(QT::escale == 2, "posit<16,1> escale should be 2");
		static_assert(QT::range == 2 * (4 * 16 - 8), "posit<16,1> range should be 112");
		static_assert(QT::half_range == 56, "posit<16,1> half_range should be 56");
		static_assert(QT::qbits == 142, "posit<16,1> qbits should be 142");

		std::cout << "posit<16,1> quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}
	{
		using Scalar = posit<8, 0>;
		using QT = quire_traits<Scalar>;

		static_assert(QT::escale == 1, "posit<8,0> escale should be 1");
		static_assert(QT::range == 1 * (4 * 8 - 8), "posit<8,0> range should be 24");
		static_assert(QT::qbits == 54, "posit<8,0> qbits should be 54");

		std::cout << "posit<8,0> quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}

	// ========================================================================
	// cfloat quire traits -- validate against IEEE-754 formula
	// ========================================================================
	{
		// IEEE-754 single precision equivalent
		using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
		using QT = quire_traits<Scalar>;

		// mbits=24, fbits=23, bias=127, max_scale=127, abs_min_scale=149
		// radix_point=298, upper_range=256, range=554, half_range=298, qbits=584
		static_assert(QT::mbits == 24, "cfloat<32,8> mbits should be 24");
		static_assert(QT::range == 554, "cfloat<32,8> range should be 554");
		static_assert(QT::half_range == 298, "cfloat<32,8> half_range should be 298");
		static_assert(QT::qbits == 584, "cfloat<32,8> qbits should be 584");

		std::cout << "cfloat<32,8> (single) quire traits:\n";
		std::cout << "  mbits       = " << QT::mbits << '\n';
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  half_range  = " << QT::half_range << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}
	{
		// half precision equivalent
		using Scalar = cfloat<16, 5, uint16_t, true, false, false>;
		using QT = quire_traits<Scalar>;

		// mbits=11, fbits=10, bias=15, max_scale=15, abs_min_scale=24
		// radix_point=48, upper_range=32, range=80, half_range=48, qbits=110
		static_assert(QT::mbits == 11, "cfloat<16,5> mbits should be 11");
		static_assert(QT::range == 80, "cfloat<16,5> range should be 80");
		static_assert(QT::half_range == 48, "cfloat<16,5> half_range should be 48");
		static_assert(QT::qbits == 110, "cfloat<16,5> qbits should be 110");

		std::cout << "cfloat<16,5> (half) quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}

	// ========================================================================
	// fixpnt quire traits
	// ========================================================================
	{
		using Scalar = fixpnt<16, 8, Modulo, uint16_t>;
		using QT = quire_traits<Scalar>;

		static_assert(QT::range == 32, "fixpnt<16,8> range should be 32");
		static_assert(QT::half_range == 16, "fixpnt<16,8> half_range should be 16");
		static_assert(QT::radix_point == 16, "fixpnt<16,8> radix_point should be 16");
		static_assert(QT::qbits == 62, "fixpnt<16,8> qbits should be 62");

		std::cout << "fixpnt<16,8> quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  radix_point = " << QT::radix_point << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}

	// ========================================================================
	// lns quire traits
	// ========================================================================
	{
		using Scalar = lns<16, 8, uint16_t>;
		using QT = quire_traits<Scalar>;

		// integer_bits = 16 - 1 - 8 = 7, max_exponent = 128, range = 256
		static_assert(QT::integer_bits == 7, "lns<16,8> integer_bits should be 7");
		static_assert(QT::max_exponent == 128, "lns<16,8> max_exponent should be 128");
		static_assert(QT::range == 256, "lns<16,8> range should be 256");
		static_assert(QT::qbits == 286, "lns<16,8> qbits should be 286");

		std::cout << "lns<16,8> quire traits:\n";
		std::cout << "  integer_bits = " << QT::integer_bits << '\n';
		std::cout << "  range        = " << QT::range << '\n';
		std::cout << "  qbits        = " << QT::qbits << '\n';
		std::cout << '\n';
	}

	// ========================================================================
	// dbns quire traits
	// ========================================================================
	{
		using Scalar = dbns<16, 5, uint16_t>;
		using QT = quire_traits<Scalar>;

		std::cout << "dbns<16,5> quire traits:\n";
		std::cout << "  range       = " << QT::range << '\n';
		std::cout << "  half_range  = " << QT::half_range << '\n';
		std::cout << "  qbits       = " << QT::qbits << '\n';
		std::cout << '\n';
	}

	// ========================================================================
	// Cross-type comparison: show quire sizes for common configurations
	// ========================================================================
	{
		std::cout << "Quire size comparison (default capacity=30):\n";
		std::cout << "  posit<8,0>   : " << quire_traits<posit<8, 0>>::qbits << " bits\n";
		std::cout << "  posit<16,1>  : " << quire_traits<posit<16, 1>>::qbits << " bits\n";
		std::cout << "  posit<32,2>  : " << quire_traits<posit<32, 2>>::qbits << " bits\n";
		std::cout << "  cfloat<16,5> : " << quire_traits<cfloat<16, 5, uint16_t, true, false, false>>::qbits << " bits\n";
		std::cout << "  cfloat<32,8> : " << quire_traits<cfloat<32, 8, uint32_t, true, false, false>>::qbits << " bits\n";
		std::cout << "  fixpnt<16,8> : " << quire_traits<fixpnt<16, 8, Modulo, uint16_t>>::qbits << " bits\n";
		std::cout << "  fixpnt<32,16>: " << quire_traits<fixpnt<32, 16, Modulo, uint32_t>>::qbits << " bits\n";
		std::cout << "  lns<16,8>    : " << quire_traits<lns<16, 8, uint16_t>>::qbits << " bits\n";
		std::cout << '\n';
	}

	// ========================================================================
	// Custom capacity: demonstrate computing quire size with non-default capacity
	// ========================================================================
	{
		using QT = quire_traits<posit<32, 2>>;
		static_assert(QT::range + 10 == 490, "posit<32,2> with capacity=10 should be 490");
		static_assert(QT::range + 50 == 530, "posit<32,2> with capacity=50 should be 530");

		std::cout << "Custom capacity for posit<32,2>:\n";
		std::cout << "  capacity=10 : " << QT::range + 10 << " bits\n";
		std::cout << "  capacity=30 : " << QT::qbits << " bits (default)\n";
		std::cout << "  capacity=50 : " << QT::range + 50 << " bits\n";
		std::cout << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

// string_parse.cpp: tests for integer parse() migrated to string_parse primitives
//                  (Phase B1 of issue #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "integer<> parse() string-parsing test suite";
	std::string test_tag    = "integer<> parse()";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using I8  = integer< 8, std::uint8_t,  IntegerNumberType::IntegerNumber>;
	using I16 = integer<16, std::uint8_t,  IntegerNumberType::IntegerNumber>;
	using I32 = integer<32, std::uint32_t, IntegerNumberType::IntegerNumber>;
	using I64 = integer<64, std::uint32_t, IntegerNumberType::IntegerNumber>;

	// ----- decimal -----
	{
		int start = nrOfFailedTestCases;
		I8 a;
		if (!parse(std::string("0"),    a) || a != I8(0))    ++nrOfFailedTestCases;
		if (!parse(std::string("5"),    a) || a != I8(5))    ++nrOfFailedTestCases;
		if (!parse(std::string("127"),  a) || a != I8(127))  ++nrOfFailedTestCases;
		if (!parse(std::string("-1"),   a) || a != I8(-1))   ++nrOfFailedTestCases;
		if (!parse(std::string("-128"), a) || a != I8(-128)) ++nrOfFailedTestCases;
		if (!parse(std::string("+42"),  a) || a != I8(42))   ++nrOfFailedTestCases;
		I32 b;
		if (!parse(std::string("1000000"), b) || b != I32(1000000)) ++nrOfFailedTestCases;
		I64 c;
		if (!parse(std::string("1234567890123"), c) || c != I64(1234567890123LL)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: decimal parsing\n";
	}

	// ----- binary -----
	{
		int start = nrOfFailedTestCases;
		I8 a;
		if (!parse(std::string("0b1010"),     a) || a != I8(10)) ++nrOfFailedTestCases;
		if (!parse(std::string("0b0"),        a) || a != I8(0))  ++nrOfFailedTestCases;
		if (!parse(std::string("0b11111111"), a) || a != I8(-1)) ++nrOfFailedTestCases;
		if (!parse(std::string("0B1010"),     a) || a != I8(10)) ++nrOfFailedTestCases;
		if (!parse(std::string("-0b1010"),    a) || a != I8(-10)) ++nrOfFailedTestCases;
		I32 b;
		if (!parse(std::string("0b10101010101010101010101010101010"), b)
		    || b != I32(0xAAAAAAAA)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: binary parsing\n";
	}

	// ----- octal -----
	{
		int start = nrOfFailedTestCases;
		I16 a;
		if (!parse(std::string("0o17"),  a) || a != I16(15))  ++nrOfFailedTestCases;
		if (!parse(std::string("0o100"), a) || a != I16(64))  ++nrOfFailedTestCases;
		I8 b;
		if (!parse(std::string("0o0"),   b) || b != I8(0))    ++nrOfFailedTestCases;
		if (!parse(std::string("0O17"),  b) || b != I8(15))   ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: octal parsing\n";
	}

	// ----- hex -----
	{
		int start = nrOfFailedTestCases;
		I8 a;
		if (!parse(std::string("0xFF"), a) || a != I8(-1))    ++nrOfFailedTestCases;
		if (!parse(std::string("0X1F"), a) || a != I8(31))    ++nrOfFailedTestCases;
		if (!parse(std::string("0xaB"), a) || a != I8(0xAB))  ++nrOfFailedTestCases;
		if (!parse(std::string("-0x05"), a) || a != I8(-5))   ++nrOfFailedTestCases;
		I16 b;
		if (!parse(std::string("0xFF"), b) || b != I16(255))  ++nrOfFailedTestCases;
		I32 c;
		if (!parse(std::string("0xDEADBEEF"),  c) || c != I32(0xDEADBEEF)) ++nrOfFailedTestCases;
		if (!parse(std::string("0xDEAD'BEEF"), c) || c != I32(0xDEADBEEF)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hex parsing\n";
	}

	// ----- invalid inputs must be rejected -----
	{
		int start = nrOfFailedTestCases;
		I8 a;
		if (parse(std::string(""),      a)) ++nrOfFailedTestCases;
		if (parse(std::string("-"),     a)) ++nrOfFailedTestCases;
		if (parse(std::string("12a"),   a)) ++nrOfFailedTestCases;
		if (parse(std::string("0b102"), a)) ++nrOfFailedTestCases;
		if (parse(std::string("0o18"),  a)) ++nrOfFailedTestCases;
		if (parse(std::string("0xZZ"),  a)) ++nrOfFailedTestCases;
		if (parse(std::string("0b"),    a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x"),    a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x'"),   a)) ++nrOfFailedTestCases;
		if (parse(std::string("0x''"),  a)) ++nrOfFailedTestCases;
		I32 b;
		if (parse(std::string("0x'''"), b)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: invalid-input rejection\n";
	}

	// ----- commit-on-success: a failed parse must leave value untouched -----
	{
		int start = nrOfFailedTestCases;
		I32 v(42);
		if (parse(std::string("0x1G"), v))  ++nrOfFailedTestCases;  // should fail
		if (v != I32(42))                   ++nrOfFailedTestCases;  // unchanged

		v = I32(-7);
		if (parse(std::string("123abc"), v)) ++nrOfFailedTestCases;
		if (v != I32(-7))                    ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: commit-on-success\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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

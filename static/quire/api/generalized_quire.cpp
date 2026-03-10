// generalized_quire.cpp: tests for the generalized quire (limb-based super-accumulator)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Test basic construction and properties for cfloat
int TestCfloatQuireConstruction() {
	int nrOfFailedTestCases = 0;

	// IEEE single-precision equivalent
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	using QT = quire_traits<Scalar>;

	quire<Scalar> q;
	std::cout << "quire<cfloat<32,8>> properties:\n";
	std::cout << "  total bits:  " << q.total_bits() << '\n';
	std::cout << "  max scale:   " << q.max_scale() << '\n';
	std::cout << "  min scale:   " << q.min_scale() << '\n';
	std::cout << "  capacity:    " << q.capacity_range() << '\n';
	std::cout << "  is zero:     " << (q.iszero() ? "yes" : "no") << '\n';

	if (q.total_bits() != QT::qbits + 1) {
		std::cerr << "FAIL: total_bits mismatch\n";
		++nrOfFailedTestCases;
	}
	if (!q.iszero()) {
		std::cerr << "FAIL: default constructed quire should be zero\n";
		++nrOfFailedTestCases;
	}
	std::cout << '\n';
	return nrOfFailedTestCases;
}

// Test integer assignment
int TestIntegerAssignment() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q;

	q = int64_t(42);
	if (q.iszero() || q.sign()) {
		std::cerr << "FAIL: q = 42 should be positive non-zero\n";
		++nrOfFailedTestCases;
	}
	if (q.scale() != 5) {  // 42 = 101010, MSB at position 5
		std::cerr << "FAIL: q = 42 should have scale 5, got " << q.scale() << '\n';
		++nrOfFailedTestCases;
	}

	q = int64_t(-7);
	if (!q.sign()) {
		std::cerr << "FAIL: q = -7 should be negative\n";
		++nrOfFailedTestCases;
	}

	q = int64_t(0);
	if (!q.iszero()) {
		std::cerr << "FAIL: q = 0 should be zero\n";
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test blocktriple accumulation using quire_mul products
// quire_mul produces full-precision MUL blocktriples with correct radix
int TestBlocktripleAccumulation() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q;
	Scalar one(1.0f), two(2.0f);

	// Accumulate via quire_mul(x, 1.0) to test single-value accumulation
	q += quire_mul(one, one);  // 1.0 * 1.0 = 1.0
	double result = q.convert_to<double>();
	if (result != 1.0) {
		std::cerr << "FAIL: q += 1*1 should give 1.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	q += quire_mul(two, one);  // 2.0 * 1.0 = 2.0
	result = q.convert_to<double>();
	if (result != 3.0) {
		std::cerr << "FAIL: q += 2*1 should give 3.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	q += quire_mul(one, one);  // 1.0 * 1.0 = 1.0
	result = q.convert_to<double>();
	if (result != 4.0) {
		std::cerr << "FAIL: q += 1*1 should give 4.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test accumulation with subtraction
int TestAccumulationWithSubtraction() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q;
	Scalar one(1.0f);

	// Use quire_mul(x, 1.0) to accumulate individual values
	q += quire_mul(Scalar(10.0f), one);
	q -= quire_mul(Scalar(3.0f), one);
	double result = q.convert_to<double>();
	if (result != 7.0) {
		std::cerr << "FAIL: 10 - 3 should be 7.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	// subtract more than current value: should flip sign
	q -= quire_mul(Scalar(20.0f), one);
	result = q.convert_to<double>();
	if (result != -13.0) {
		std::cerr << "FAIL: 7 - 20 should be -13.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test dot product accumulation pattern using quire_mul
int TestDotProductPattern() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q;

	// Simulate dot product: sum of a[i]*b[i] using quire_mul
	Scalar a[] = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
	Scalar b[] = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };
	// expected: 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70

	for (int i = 0; i < 4; ++i) {
		q += quire_mul(a[i], b[i]);
	}

	double result = q.convert_to<double>();
	if (result != 70.0) {
		std::cerr << "FAIL: dot product should be 70.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test with posit type
int TestPositQuire() {
	int nrOfFailedTestCases = 0;

	using Scalar = posit<32, 2>;
	quire<Scalar> q;

	std::cout << "quire<posit<32,2>> properties:\n";
	std::cout << "  total bits:  " << q.total_bits() << '\n';
	std::cout << "  max scale:   " << q.max_scale() << '\n';
	std::cout << "  min scale:   " << q.min_scale() << '\n';
	std::cout << '\n';

	// posit<32,2> quire should have 511 total bits (510 + 1 sign)
	if (q.total_bits() != 511) {
		std::cerr << "FAIL: posit<32,2> quire should have 511 total bits, got " << q.total_bits() << '\n';
		++nrOfFailedTestCases;
	}

	// accumulate via integer assignment
	q = int64_t(100);
	double result = q.convert_to<double>();
	if (result != 100.0) {
		std::cerr << "FAIL: posit quire = 100 should give 100.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test with fixpnt type
int TestFixpntQuire() {
	int nrOfFailedTestCases = 0;

	using Scalar = fixpnt<16, 8, Modulo, uint16_t>;
	quire<Scalar> q;

	std::cout << "quire<fixpnt<16,8>> properties:\n";
	std::cout << "  total bits:  " << q.total_bits() << '\n';
	std::cout << "  range:       " << q.dynamic_range() << '\n';
	std::cout << '\n';

	if (!q.iszero()) {
		std::cerr << "FAIL: default fixpnt quire should be zero\n";
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test quire-quire addition
int TestQuireQuireAddition() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q1, q2;

	q1 = int64_t(100);
	q2 = int64_t(200);
	q1 += q2;
	double result = q1.convert_to<double>();
	if (result != 300.0) {
		std::cerr << "FAIL: 100 + 200 should be 300.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test exception handling
int TestExceptions() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
	quire<Scalar> q;

	// Test NaR/NaN assignment
	blocktriple<23, BlockTripleOperator::REP, uint32_t> nan_val;
	nan_val.setnan();
	try {
		q = nan_val;
		std::cerr << "FAIL: assigning NaN should throw operand_is_nar\n";
		++nrOfFailedTestCases;
	}
	catch (const operand_is_nar&) {
		// expected
	}

	// Test inf assignment
	blocktriple<23, BlockTripleOperator::REP, uint32_t> inf_val;
	inf_val.setinf();
	try {
		q += inf_val;
		std::cerr << "FAIL: adding inf should throw operand_is_nar\n";
		++nrOfFailedTestCases;
	}
	catch (const operand_is_nar&) {
		// expected
	}

	return nrOfFailedTestCases;
}

// Test quire properties display
int TestQuireProperties() {
	int nrOfFailedTestCases = 0;

	using CFloat = cfloat<32, 8, uint32_t, true, false, false>;
	using Posit  = posit<32, 2>;

	std::cout << quire_properties<CFloat>();
	std::cout << quire_properties<Posit>();
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "generalized quire (limb-based)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += TestQuireProperties();
	nrOfFailedTestCases += ReportTestResult(TestCfloatQuireConstruction(), "quire<cfloat<32,8>>", "construction");
	nrOfFailedTestCases += ReportTestResult(TestIntegerAssignment(), "quire<cfloat<32,8>>", "integer assignment");
	nrOfFailedTestCases += ReportTestResult(TestBlocktripleAccumulation(), "quire<cfloat<32,8>>", "blocktriple accumulation");
	nrOfFailedTestCases += ReportTestResult(TestAccumulationWithSubtraction(), "quire<cfloat<32,8>>", "accumulation with subtraction");
	nrOfFailedTestCases += ReportTestResult(TestDotProductPattern(), "quire<cfloat<32,8>>", "dot product pattern");
	nrOfFailedTestCases += ReportTestResult(TestPositQuire(), "quire<posit<32,2>>", "posit quire");
	nrOfFailedTestCases += ReportTestResult(TestFixpntQuire(), "quire<fixpnt<16,8>>", "fixpnt quire");
	nrOfFailedTestCases += ReportTestResult(TestQuireQuireAddition(), "quire<cfloat<32,8>>", "quire-quire addition");
	nrOfFailedTestCases += ReportTestResult(TestExceptions(), "quire<cfloat<32,8>>", "exception handling");

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

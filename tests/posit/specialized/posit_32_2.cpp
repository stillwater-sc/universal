// posit_32_2.cpp: Functionality tests for fast specialized posit<32,2>
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<32,2>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_32_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#include <universal/posit/posit_parse.hpp>
// test helpers, such as, ReportTestResults
#include "../../utils/test_helpers.hpp"
#include "../../utils/posit_test_randoms.hpp"

// Standard posit with nbits = 32 have es = 2 exponent bits.

template<size_t nbits, size_t es>
void CheckAddition() {
	using namespace std;
	using namespace sw::unum;

	posit<nbits, es> pa, pb, pc;
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa.set_raw_bits(a);
		for (int b = 0; b < 256; ++b) {
			pb.set_raw_bits(b);
			pc = pa + pb;

			double da, db, dref;
			da = double(pa);
			db = double(pb);
			dref = da + db;

			posit<nbits, es> pref = dref;
			if (pref != pc) {
				cout << "FAIL: " << posit_format(pa) << " + " << posit_format(pb) << " produced " << posit_format(pc) << " instead of " << posit_format(pref) << endl;
				++fails;
				break;
			}
		}
	}
	if (fails) {
		printf("addition        FAIL\n");
	}
	else {
		printf("addition        PASS\n");
	}
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t RND_TEST_CASES = 500000;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
#if POSIT_FAST_POSIT_32_2
	cout << "Fast specialization posit<32,2> configuration tests" << endl;
#else
	cout << "Standard posit<32,2> configuration tests" << endl;

#endif

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<32,2>";

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl << endl;

#if MANUAL_TESTING

	posit<nbits, es> a, b, c;
	a.set_raw_bits(0x0aa99eea);
	b.set_raw_bits(0xf97fcf40);
	cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << endl;
	c = a + b;
	cout << a << " + " << b << " = " << c << endl;
	cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << endl;
	c = a;
	c += b;
	cout << a << " + " << b << " = " << c << endl;
	cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << endl;

#if 1
	std::string testVector[] = {
		"32.2x0a2f641dp",
		"32.2x06e8eb35p",
		"32.2xf97fcf40p",
		"32.2x03812f3fp",
		"32.2xf57e2aa8p",
		"32.2xf88b7e2fp",
		"32.2x04cd9168p",
		"32.2xfa843f6bp",
		"32.2x05a36e2ep",
		"32.2xf4e89c21p",
		"32.2x05080d4cp",
		"32.2x05a36e2ep"
	};

	std::string golden[] = {
		"32.2x0a2f641dp",
		"32.2x0aa99eeap",
		"32.2x0a4992bap",
		"32.2x0a51a5aep",
		"32.2xfa7e82b0p",
		"32.2xf82b1edbp",
		"32.2xf864d108p",
		"32.2xf805e0e3p",
		"32.2xf86ebc6ep",
		"32.2xf41ffa58p",
		"32.2xf440fc02p",
		"32.2xf47569c8p"
	};

	posit<nbits, es> accu1(0), accu2(0);
	int i = 0;
	for (auto v : testVector) {
		if (!parse(v, p)) {
			std::cerr << "unable to parse -" << v << "- into a posit value\n";
		}
		else {
			cout << hex_format(accu1) << " + " << hex_format(p) << endl;
			accu1 = accu1 + p;
			accu2 += p;
			cout << hex_format(accu1) << " vs " << golden[i++] << " " << accu2 << endl;
		}
	}
#endif
#else

	// special cases
	cout << "Special case tests " << endl;
	string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(tag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessThan          <nbits, es>(), tag, "    <           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=          (native)  ");

	// conversion tests
	// internally this generators are clamped as the state space 2^33 is too big
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion           <nbits, es>(tag, bReportIndividualTestCases), tag, "sint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateUintConversion              <nbits, es>(tag, bReportIndividualTestCases), tag, "uint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion                  <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign    (native)  ");
//	nrOfFailedTestCases += ReportTestResult( ValidateConversionThroughRandoms <nbits, es>(tag, true, 100), tag, "float assign   ");

	// arithmetic tests
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction     (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division        (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "+=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "-=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "*=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "/=              (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SQRT,  RND_TEST_CASES), tag, "sqrt            (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_EXP,   RND_TEST_CASES), tag, "exp                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_EXP2,  RND_TEST_CASES), tag, "exp2                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG,   RND_TEST_CASES), tag, "log                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG2,  RND_TEST_CASES), tag, "log2                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_LOG10, RND_TEST_CASES), tag, "log10                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SIN,   RND_TEST_CASES), tag, "sin                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_COS,   RND_TEST_CASES), tag, "cos                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_TAN,   RND_TEST_CASES), tag, "tan                       ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ASIN,  RND_TEST_CASES), tag, "asin                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ACOS,  RND_TEST_CASES), tag, "acos                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ATAN,  RND_TEST_CASES), tag, "atan                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_SINH,  RND_TEST_CASES), tag, "sinh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_COSH,  RND_TEST_CASES), tag, "cosh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_TANH,  RND_TEST_CASES), tag, "tanh                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ASINH, RND_TEST_CASES), tag, "asinh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ACOSH, RND_TEST_CASES), tag, "acosh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateUnaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_ATANH, RND_TEST_CASES), tag, "atanh                     ");
	// elementary functions with two operands
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<nbits, es>(tag, bReportIndividualTestCases, OPCODE_POW, RND_TEST_CASES),   tag, "pow                       ");
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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


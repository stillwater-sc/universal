// mod_conversion.cpp: test suite runner for fixed-point modulo conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

namespace sw {
	namespace universal {

		// generate specific test case that you can trace with the trace conditions in fixpnt.hpp
		// for most bugs they are traceable with _trace_conversion and _trace_add
		template<size_t nbits, size_t rbits, typename Ty>
		void GenerateTestCase(Ty _a, Ty _b) {
			Ty ref;
			sw::universal::fixpnt<nbits, rbits> a, b, cref, result;
			a = _a;
			b = _b;
			result = a + b;
			ref = _a + _b;
			cref = ref;
			std::streamsize oldPrecision = std::cout.precision();
			std::cout << std::setprecision(nbits - 2);
			std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
			std::cout << a << " + " << b << " = " << result << " (reference: " << cref << ")   " ;
			std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
			std::cout << std::dec << std::setprecision(oldPrecision);
		}

		template<size_t nbits, size_t rbits>
		void GenerateFixedPointComparisonTable() {
			using namespace sw::universal;
			constexpr size_t NR_VALUES = (size_t(1) << nbits);
			fixpnt<nbits, rbits> fp;
			fixpnt<nbits+1, rbits+1> fpnext;
			std::cout << "  fixpnt<" << nbits + 1 << "," << rbits + 1 << ">      |    fixpnt<" << nbits << ", " << rbits << ">" << '\n';
			for (size_t i = 0; i < NR_VALUES; ++i) {
				fp.set_raw_bits(i);
				fpnext.set_raw_bits(2*i);
				std::cout << to_binary(fpnext) << ' ' << std::setw(10) << fpnext << "  |  " << to_binary(fp) << ' ' << std::setw(15) << fp << '\n';
				fpnext.set_raw_bits(2 * i + 1);
				std::cout << to_binary(fpnext) << ' ' << std::setw(10) << fpnext << "  |  " << '\n';
			}
		}

		// verify that integer conversion picks up the correct integer encoding for the fixed-point
		template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
		int VerifySignedIntegerProgressions(bool reportTestCases) {
			using namespace sw::universal;
			int nrOfFailedTestCases = 0;

			// generate the integer progression for this fixpnt, which is represented by a marching MSB
			constexpr size_t ibits = nbits - rbits;  // <8,4> has 8-4 = 4 ibits in 2's complement form, and 4 rbits
			static_assert(ibits > 2, "test requires at least 3 bits of integer bits");
			// assume that we have maximally 64 integer bits
			static_assert(ibits < 65, "test assumes we have at most 64 integer bits");

			// largest negative integer    is 100...000
			// largest positive integer    is 011111111
			// largest positive power of 2 is 010000000

		//	Fixed maxneg(SpecificValue::maxneg);
			std::uint64_t maxneg{0xFFFFFFFFFFFFFFFFull};
			maxneg <<= (ibits - 1);
			int64_t marchingOne = (long long)maxneg;
			std::cout << "ibits - 1 = " << (ibits - 1) << '\n';
			std::cout << "maxneg      " << to_binary(maxneg) << '\n';
			std::cout << "marchingOne " << to_binary(marchingOne) << '\n'; 
			for (int i = static_cast<int>(ibits - 1); i >= 0; --i) {
				fixpnt<nbits, rbits, arithmetic, bt> a = marchingOne;
				if (i == 0) {
					if (reportTestCases) std::cout << "i = " << std::setw(3) << 0 << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << " : " << a << '\n';
				}
				else {
					if (reportTestCases) std::cout << "i = " << std::setw(3) << -i << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << " : " << a << '\n';
				}
				if (a != marchingOne) ++nrOfFailedTestCases;
				marchingOne /= 2;
			}
				uint64_t marchingOnePos = 1;
				for (size_t i = 1; i < ibits; ++i) {
					fixpnt<nbits, rbits, arithmetic, bt> a = marchingOnePos;
					if (reportTestCases) std::cout << "i = " << std::setw(3) << i << " bit pattern: " << to_binary(marchingOnePos) << " : " << to_binary(a) << " : " << a << '\n';
					if (a != marchingOnePos) ++nrOfFailedTestCases;
					marchingOnePos *= 2;
				}
			return nrOfFailedTestCases;
		}

		template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
		int VerifyUnsignedIntegerProgressions(bool reportTestCases) {
			using namespace sw::universal;
			int nrOfFailedTestCases = 0;

			using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
			// generate the integer progression for this fixpnt, which is represented by a marching MSB
			constexpr size_t ibits = nbits - rbits;  // <8,4> has 8-4 = 4 ibits in 2's complement form, and 4 rbits
			static_assert(ibits > 2, "test requires at least 3 bits of integer bits");
			// assume that we have maximally 64 integer bits
			static_assert(ibits < 65, "test assumes we have at most 64 integer bits");

			Fixed a;
			uint64_t marchingOne = 1;
			for (size_t i = 1; i < ibits; ++i) {
				a = marchingOne;
				if (reportTestCases) std::cout << "i = " << std::setw(3) << i << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << '\n';
				if (a != marchingOne) {
					++nrOfFailedTestCases;
				}
				marchingOne *= 2;
			}
			return nrOfFailedTestCases;
		}

		template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
		int VerifySignedIntegerProgressionsFloat(bool reportTestCases) {
			using namespace sw::universal;
			int nrOfFailedTestCases = 0;

			using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
			// generate the integer progression for this fixpnt, which is represented by a marging MSB
			constexpr size_t ibits = nbits - rbits;  // <8,4> has 8-4 = 4 ibits in 2's complement form, and 4 rbits
			// assume that we have maximally 64 integer bits
			static_assert(ibits < 65, "test assumes we have at most 64 integer bits");

			// largest negative integer    is 100...000
			// largest positive integer    is 011111111
			// largest positive power of 2 is 010000000

			Fixed a, b;

			constexpr Fixed maxneg(SpecificValue::maxneg);
			int64_t marchingOne = (long long)maxneg;
			float f = float(marchingOne);
			std::cout << to_binary(f) << '\n';
			for (int i = static_cast<int>(ibits - 1); i >= 0; --i) {
				a = float(marchingOne);
				b = double(marchingOne);
				if (i == 0) {
					if (reportTestCases) std::cout << "i = " << std::setw(3) << 0 << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << " : " << to_binary(b) << '\n';
				}
				else {
					if (reportTestCases) std::cout << "i = " << std::setw(3) << -i << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << " : " << to_binary(b) << '\n';
				}
				if (a != marchingOne || b != marchingOne) {
					++nrOfFailedTestCases;
				}
				marchingOne /= 2;
			}
			marchingOne = 1;
			for (size_t i = 1; i < ibits; ++i) {
				a = float(marchingOne);
				b = double(marchingOne);
				if (reportTestCases) std::cout << "i = " << std::setw(3) << i << " bit pattern: " << to_binary(marchingOne) << " : " << to_binary(a) << " : " << to_binary(b) << '\n';
				if (a != marchingOne || b != marchingOne) {
					++nrOfFailedTestCases;
				}
				marchingOne *= 2;
			}
			return nrOfFailedTestCases;
		}


	}
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point modulo conversion ";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	fixpnt<8, 4, Modulo, uint16_t> f;
	f = 0.000001f;
	std::cout << to_binary(f) << " : " << f << std::endl;

//	fixpnt_range(fixpnt< 4, 0, Modulo, uint16_t>());
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 0, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 0,Modulo,uint8_t>");
//	fixpnt_range(fixpnt< 4, 1, Modulo, uint16_t>());
//	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 1,Modulo,uint8_t>");

//	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions<  8,  4, Modulo, uint8_t>(reportTestCases), "signed integer conversion", "fixpnt<8,4,Modulo,uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 16,  8, Modulo, uint8_t>(reportTestCases), "signed integer conversion", "fixpnt<16,8,Modulo,uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 32, 16, Modulo, uint8_t>(reportTestCases), "signed integer conversion", "fixpnt<32,16,Modulo,uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 64, 32, Modulo, uint8_t>(reportTestCases), "signed integer conversion", "fixpnt<64,32,Modulo,uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions<128, 64, Modulo, uint8_t>(reportTestCases), "signed integer conversion", "fixpnt<128,64,Modulo,uint8_t>");

	// single limb representations
	//nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat<  8,  4, Modulo, uint8_t>(reportTestCases);
	//nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 16,  8, Modulo, uint16_t>(reportTestCases);
	//nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 32, 16, Modulo, uint32_t>(reportTestCases);
	//nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 64, 32, Modulo, uint64_t>(reportTestCases);

	// multiple limb representations
	// nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 16,  8, Modulo, uint8_t>(reportTestCases);
	// nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 32, 16, Modulo, uint8_t>(reportTestCases);
	// nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 64, 32, Modulo, uint8_t>(reportTestCases);
	// nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat<128, 64, Modulo, uint8_t>(reportTestCases);

//	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions<  8, 4, Modulo, uint8_t>(reportTestCases),  test_tag, "fixpnt<  8, 4, Modulo, uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 16, 8, Modulo, uint8_t>(true),  test_tag, "fixpnt< 16, 8, Modulo, uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 32, 16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 32,16, Modulo, uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 64, 32, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 64,32, Modulo, uint8_t>");
//	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions<128, 64, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<128,64, Modulo, uint8_t>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 0, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 0,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 1,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 2, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 2,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 3, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 3,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 4, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 4, 4,Modulo,uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 0, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 0,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 1, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 1,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 2, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 2,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 3, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 3,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 4, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 4,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 5, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 5,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 6, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 6,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 7, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 7,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 8, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 8, 8,Modulo,uint8_t>");

#if defined(__GNUC__) || defined(__GNUG__)
		/* GNU GCC/G++. --------------------------------------------- */

	// TODO GCC 11 is exhibiting undefined behavior with these progressions when they are issued together as in the else clause
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions<128, 64, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<128,64, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 64, 32, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 64,32, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 32, 16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 32,16, Modulo, uint8_t>");

#else
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions<  8,  4, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<  8, 4, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 16,  8, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 16, 8, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 32, 16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 32,16, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions< 64, 32, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 64,32, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressions<128, 64, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<128,64, Modulo, uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat<  8, 4, Modulo, uint8_t>(reportTestCases),  test_tag, "fixpnt<  8, 4, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 16, 8, Modulo, uint8_t>(reportTestCases),  test_tag, "fixpnt< 16, 8, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 32, 16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 32,16, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat< 64, 32, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 64,32, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySignedIntegerProgressionsFloat<128, 64, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<128,64, Modulo, uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions<  8, 4, Modulo, uint8_t>(reportTestCases),  test_tag, "fixpnt<  8, 4, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 16, 8, Modulo, uint8_t>(reportTestCases),  test_tag, "fixpnt< 16, 8, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 32, 16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 32,16, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions< 64, 32, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt< 64,32, Modulo, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedIntegerProgressions<128, 64, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<128,64, Modulo, uint8_t>");
#endif

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 0, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 0,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 1, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 1,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 2, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 2,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 3,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 4, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 4,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 6, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 6,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 8, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12, 8,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12,10, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12,10,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12,12, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<12,12,Modulo,uint8_t>");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 0, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 0,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 1, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 1,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 2,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 3, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 3,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 4, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 4,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 8, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16, 8,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16,12, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16,12,Modulo,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16,16, Modulo, uint8_t>(reportTestCases), test_tag, "fixpnt<16,16,Modulo,uint8_t>");
#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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


/*
  Value relationships between fixpnt<nbits+1,rbits+1> and fixpnt<nbits,rbits> we'll use for validation

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

  fixpnt<5,1>      |    fixpnt<4, 0>
0000.0          0  |  0000.          0
0000.1        0.5  |
0001.0        1.0  |  0001.        1.0
0001.1        1.5  |
0010.0        2.0  |  0010.        2.0
0010.1        2.5  |
0011.0        3.0  |  0011.        3.0
0011.1        3.5  |
0100.0        4.0  |  0100.        4.0
0100.1        4.5  |
0101.0        5.0  |  0101.        5.0
0101.1        5.5  |
0110.0        6.0  |  0110.        6.0
0110.1        6.5  |
0111.0        7.0  |  0111.        7.0
0111.1        7.5  |
1000.0       -8.0  |  1000.       -8.0
1000.1       -7.5  |
1001.0       -7.0  |  1001.       -7.0
1001.1       -6.5  |
1010.0       -6.0  |  1010.       -6.0
1010.1       -5.5  |
1011.0       -5.0  |  1011.       -5.0
1011.1       -4.5  |
1100.0       -4.0  |  1100.       -4.0
1100.1       -3.5  |
1101.0       -3.0  |  1101.       -3.0
1101.1       -2.5  |
1110.0       -2.0  |  1110.       -2.0
1110.1       -1.5  |
1111.0       -1.0  |  1111.       -1.0
1111.1       -0.5  |



  fixpnt<5,2>      |    fixpnt<4, 1>
000.00          0  |  000.0          0
000.01       0.25  |
000.10       0.50  |  000.1        0.5
000.11       0.75  |
001.00        1.0  |  001.0        1.0
001.01       1.25  |
001.10       1.50  |  001.1        1.5
001.11       1.75  |
010.00        2.0  |  010.0        2.0
010.01       2.25  |
010.10       2.50  |  010.1        2.5
010.11       2.75  |
011.00        3.0  |  011.0        3.0
011.01       3.25  |
011.10       3.50  |  011.1        3.5
011.11       3.75  |
100.00       -4.0  |  100.0       -4.0
100.01      -3.75  |
100.10      -3.50  |  100.1       -3.5
100.11      -3.25  |
101.00       -3.0  |  101.0       -3.0
101.01      -2.75  |
101.10      -2.50  |  101.1       -2.5
101.11      -2.25  |
110.00       -2.0  |  110.0       -2.0
110.01      -1.75  |
110.10      -1.50  |  110.1       -1.5
110.11      -1.25  |
111.00       -1.0  |  111.0       -1.0
111.01      -0.75  |
111.10      -0.50  |  111.1       -0.5
111.11      -0.25  |



  fixpnt<5,3>      |    fixpnt<4, 2>
00.000          0  |  00.00          0
00.001      0.125  |
00.010      0.250  |  00.01       0.25
00.011      0.375  |
00.100      0.500  |  00.10       0.50
00.101      0.625  |
00.110      0.750  |  00.11       0.75
00.111      0.875  |
01.000        1.0  |  01.00        1.0
01.001      1.125  |
01.010      1.250  |  01.01       1.25
01.011      1.375  |
01.100      1.500  |  01.10       1.50
01.101      1.625  |
01.110      1.750  |  01.11       1.75
01.111      1.875  |
10.000       -2.0  |  10.00       -2.0
10.001     -1.875  |
10.010     -1.750  |  10.01      -1.75
10.011     -1.625  |
10.100     -1.500  |  10.10      -1.50
10.101     -1.375  |
10.110     -1.250  |  10.11      -1.25
10.111     -1.125  |
11.000       -1.0  |  11.00       -1.0
11.001     -0.875  |
11.010     -0.750  |  11.01      -0.75
11.011     -0.625  |
11.100     -0.500  |  11.10      -0.50
11.101     -0.375  |
11.110     -0.250  |  11.11      -0.25
11.111     -0.125  |
 */

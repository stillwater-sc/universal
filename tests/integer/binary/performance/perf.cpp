//  perf.cpp : baseline performance benchmarking for integer<>
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1;
		c = d;
		b = c;
		a = b;
		// we are doing two adds per loop, so half the NR_OPS
		NR_OPS >>= 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			b = c - a;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == (d + d)) std::cout << " "; else std::cout << "-";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1;
		c = d;
		b = 1;
		a = 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << " "; else std::cout << "-";
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1;
		c = d;
		b = 1;
		a = 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a / b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << " "; else std::cout << "-";
	}

	// Generic string conversion
	template<typename Scalar>
	void SerializationWorkload(size_t NR_OPS) {
		Scalar a(SpecificValue::maxpos);
		std::stringstream s;
		for (size_t i = 0; i < NR_OPS; ++i) {
			s << a;
			s.str(std::string()); // clear the string
		}
		std::cout << s.str();
	}

	/*
	April, 2022, NZXT Ryzen 7 2700X 3.7GHz desktop

	Arithmetic operator performance
	 integer< 8>   add/subtract      4194304 per       0.0043789sec -> 957 Mops/sec
	 integer<16>   add/subtract      4194304 per       0.0046774sec -> 896 Mops/sec
	 integer<32>   add/subtract      4194304 per       0.0045622sec -> 919 Mops/sec
	 integer<64>   add/subtract      4194304 per       0.0041442sec ->   1 Gops/sec
	 integer< 8>   multiplication    1048576 per       0.0010844sec -> 966 Mops/sec
	 integer<16>   multiplication    1048576 per       0.0010819sec -> 969 Mops/sec
	 integer<32>   multiplication     524288 per       0.0004136sec ->   1 Gops/sec
	 integer<64>   multiplication     524288 per       0.0004144sec ->   1 Gops/sec
	 integer< 8>   division          1048576 per       0.0910805sec ->  11 Mops/sec
	 integer<16>   division          1048576 per        0.102274sec ->  10 Mops/sec
	 integer<32>   division           524288 per       0.0648111sec ->   8 Mops/sec
	 integer<64>   division           524288 per       0.0614304sec ->   8 Mops/sec
	 */

	void TestArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("integer< 8>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<32, uint32_t> >, NR_OPS);
		PerformanceRunner("integer<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<64, uint64_t> >, NR_OPS); // <--- lucky as carry does not matter in modulo arithmetic

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer< 8>   multiplication", MultiplicationWorkload< sw::universal::integer< 8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<16>   multiplication", MultiplicationWorkload< sw::universal::integer<16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer<32>   multiplication", MultiplicationWorkload< sw::universal::integer<32, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer<64>   multiplication", MultiplicationWorkload< sw::universal::integer<64, uint64_t> >, NR_OPS / 2); // <---- TODO: support uint64_t

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer< 8>   division      ", DivisionWorkload< sw::universal::integer< 8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<16>   division      ", DivisionWorkload< sw::universal::integer<16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer<32>   division      ", DivisionWorkload< sw::universal::integer<32, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer<64>   division      ", DivisionWorkload< sw::universal::integer<64, uint32_t> >, NR_OPS / 2);    // <---- TODO: bug when BlockType is uint64_t divide fails
	}


	/*
	April, 2022, NZXT Ryzen 7 2700X 3.7GHz desktop

	Serialization operator performance
	integer<   8>  ostream              512 per       0.0022123sec -> 231 Kops/sec
	integer<  16>  ostream              512 per       0.0024522sec -> 208 Kops/sec
	integer<  32>  ostream              512 per       0.0044591sec -> 114 Kops/sec
	integer<  64>  ostream              512 per       0.0096226sec ->  53 Kops/sec
	integer< 128>  ostream              512 per       0.0242779sec ->  21 Kops/sec
	integer< 256>  ostream              512 per       0.0675558sec ->   7 Kops/sec
	integer< 512>  ostream              512 per        0.231855sec ->   2 Kops/sec
	integer<1024>  ostream              512 per        0.850865sec -> 601  ops/sec
	*/

	void TestSerializationOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nSerialization operator performance\n";

		size_t NR_OPS = 512ull;
		PerformanceRunner("integer<   8>  ostream      ", SerializationWorkload< sw::universal::integer<   8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  16>  ostream      ", SerializationWorkload< sw::universal::integer<  16, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  32>  ostream      ", SerializationWorkload< sw::universal::integer<  32, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  64>  ostream      ", SerializationWorkload< sw::universal::integer<  64, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 128>  ostream      ", SerializationWorkload< sw::universal::integer< 128, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 256>  ostream      ", SerializationWorkload< sw::universal::integer< 256, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 512>  ostream      ", SerializationWorkload< sw::universal::integer< 512, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<1024>  ostream      ", SerializationWorkload< sw::universal::integer<1024, uint8_t> >, NR_OPS);
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
	using namespace sw::universal::internal;

	std::string test_suite = "integer performance benchmarking";
//	std::string test_tag = "integer performance";
//	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance();
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	TestArithmeticOperatorPerformance();
	TestSerializationOperatorPerformance();
#endif

#if REGRESSION_LEVEL_2
	TestArithmeticOperatorPerformance();
#endif

#if REGRESSION_LEVEL_4
	TestArithmeticOperatorPerformance();
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

/*
ETLO
Date run : 3/01/2021
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/

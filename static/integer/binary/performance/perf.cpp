//  perf.cpp : baseline performance benchmarking for integer<>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

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

	// Generic set of remainders for a given number system type
	template<typename Scalar>
	void RemainderWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 2;
		c = d;
		b = c;
		a = 3;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a % b;
			b = ++c;
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

	 after enabling constexpr of single block configurations

	 Arithmetic operator performance
	 integer<  8>   add/subtract      4194304 per       0.0042346sec -> 990 Mops/sec
	 integer< 16>   add/subtract      4194304 per       0.0045856sec -> 914 Mops/sec
	 integer< 32>   add/subtract      4194304 per       0.0044825sec -> 935 Mops/sec
	 integer< 64>   add/subtract      4194304 per       0.0040611sec ->   1 Gops/sec
	 integer<128>   add/subtract      4194304 per       0.0609649sec ->  68 Mops/sec  <-- segmented algorithm
	 integer<  8>   multiplication    1048576 per       0.0010752sec -> 975 Mops/sec
	 integer< 16>   multiplication    1048576 per        0.001106sec -> 948 Mops/sec
	 integer< 32>   multiplication     524288 per       0.0004072sec ->   1 Gops/sec
	 integer< 64>   multiplication     524288 per       0.0004291sec ->   1 Gops/sec
	 integer<128>   multiplication     524288 per       0.0606596sec ->   8 Mops/sec  <-- segmented algorithm
	 integer<  8>   division          1048576 per       0.0044795sec -> 234 Mops/sec
	 integer< 16>   division          1048576 per       0.0035649sec -> 294 Mops/sec
	 integer< 32>   division           524288 per       0.0017663sec -> 296 Mops/sec
	 integer< 64>   division           524288 per         0.00179sec -> 292 Mops/sec
	 integer<128>   division           524288 per       0.0699141sec ->   7 Mops/sec  <-- segmented algorithm
	 integer<  8>   remainder         1048576 per       0.0049021sec -> 213 Mops/sec
	 integer< 16>   remainder         1048576 per       0.0036782sec -> 285 Mops/sec
	 integer< 32>   remainder          524288 per       0.0018578sec -> 282 Mops/sec
	 integer< 64>   remainder          524288 per       0.0018595sec -> 281 Mops/sec
	 integer<128>   remainder          524288 per       0.0759803sec ->   6 Mops/sec  <-- segmented algorithm
	 */

	void TestStandardArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("integer<  8>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<  8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer< 32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 32, uint32_t> >, NR_OPS);
		PerformanceRunner("integer< 64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 64, uint64_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<  8>   multiplication", MultiplicationWorkload< sw::universal::integer<  8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 16>   multiplication", MultiplicationWorkload< sw::universal::integer< 16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer< 32>   multiplication", MultiplicationWorkload< sw::universal::integer< 32, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer< 64>   multiplication", MultiplicationWorkload< sw::universal::integer< 64, uint64_t> >, NR_OPS / 2);	// uint64_t works because it is a single block

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<  8>   division      ", DivisionWorkload< sw::universal::integer<  8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 16>   division      ", DivisionWorkload< sw::universal::integer< 16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer< 24>   division      ", DivisionWorkload< sw::universal::integer< 24, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer< 32>   division      ", DivisionWorkload< sw::universal::integer< 32, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer< 48>   division      ", DivisionWorkload< sw::universal::integer< 48, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer< 64>   division      ", DivisionWorkload< sw::universal::integer< 64, uint64_t> >, NR_OPS / 2);	// uint64_t works because it is a single block

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<  8>   remainder     ", RemainderWorkload< sw::universal::integer<  8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 16>   remainder     ", RemainderWorkload< sw::universal::integer< 16, uint16_t> >, NR_OPS);
		PerformanceRunner("integer< 32>   remainder     ", RemainderWorkload< sw::universal::integer< 32, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("integer< 64>   remainder     ", RemainderWorkload< sw::universal::integer< 64, uint64_t> >, NR_OPS / 2);	// uint64_t works because it is a single block
	}

	void TestExtendedArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("integer<128>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<128, uint32_t> >, NR_OPS);	// need to drop down to uint32_t to catch carry prop
		PerformanceRunner("integer<128>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<128, uint8_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<128>   multiplication", MultiplicationWorkload< sw::universal::integer<128, uint32_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop
		PerformanceRunner("integer<128>   multiplication", MultiplicationWorkload< sw::universal::integer<128, uint8_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<128>   division      ", DivisionWorkload< sw::universal::integer<128, uint32_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop
		PerformanceRunner("integer<128>   division      ", DivisionWorkload< sw::universal::integer<128, uint8_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("integer<128>   remainder     ", RemainderWorkload< sw::universal::integer<128, uint32_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop
		PerformanceRunner("integer<128>   remainder     ", RemainderWorkload< sw::universal::integer<128, uint8_t> >, NR_OPS / 2);	// need to drop down to uint32_t to catch carry prop
	}

	/*
	April, 2022, NZXT Ryzen 7 2700X 3.7GHz desktop

	With convert_to_decimal_string() function that uses repeated addition
	Serialization operator performance
	integer<   8>  ostream              512 per       0.0022123sec -> 231 Kops/sec
	integer<  16>  ostream              512 per       0.0024522sec -> 208 Kops/sec
	integer<  32>  ostream              512 per       0.0044591sec -> 114 Kops/sec
	integer<  64>  ostream              512 per       0.0096226sec ->  53 Kops/sec
	integer< 128>  ostream              512 per       0.0242779sec ->  21 Kops/sec
	integer< 256>  ostream              512 per       0.0675558sec ->   7 Kops/sec
	integer< 512>  ostream              512 per        0.231855sec ->   2 Kops/sec
	integer<1024>  ostream              512 per        0.850865sec -> 601  ops/sec

	With new convert_to_string() function that relies on division
	Serialization operator performance
	integer<   8>  ostream              512 per       0.0002716sec ->   1 Mops/sec
	integer<  16>  ostream              512 per       0.0007282sec -> 703 Kops/sec
	integer<  32>  ostream              512 per       0.0031247sec -> 163 Kops/sec
	integer<  64>  ostream              512 per       0.0171648sec ->  29 Kops/sec
	integer< 128>  ostream              512 per       0.0931262sec ->   5 Kops/sec
	integer< 256>  ostream              512 per        0.558849sec -> 916  ops/sec
	integer< 512>  ostream              512 per         4.45766sec -> 114  ops/sec
	integer<1024>  ostream              512 per         33.6936sec ->  15  ops/sec

	-> TODO: need to replace current long division with fast segmented division 4/11/2022
	*/

	void TestStandardSerializationOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nSerialization operator performance\n";

		size_t NR_OPS = 512ull;
		PerformanceRunner("integer<   8>   ostream       ", SerializationWorkload< sw::universal::integer<   8, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  16>   ostream       ", SerializationWorkload< sw::universal::integer<  16, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  24>   ostream       ", SerializationWorkload< sw::universal::integer<  24, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  32>   ostream       ", SerializationWorkload< sw::universal::integer<  32, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  48>   ostream       ", SerializationWorkload< sw::universal::integer<  48, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<  64>   ostream       ", SerializationWorkload< sw::universal::integer<  64, uint8_t> >, NR_OPS);
	}
	void TestExtendedSerializationOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nSerialization operator performance\n";

		size_t NR_OPS = 512ull;
		PerformanceRunner("integer< 128>   ostream       ", SerializationWorkload< sw::universal::integer< 128, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 256>   ostream       ", SerializationWorkload< sw::universal::integer< 256, uint8_t> >, NR_OPS);
		PerformanceRunner("integer< 512>   ostream       ", SerializationWorkload< sw::universal::integer< 512, uint8_t> >, NR_OPS);
		PerformanceRunner("integer<1024>   ostream       ", SerializationWorkload< sw::universal::integer<1024, uint8_t> >, NR_OPS);
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
	TestStandardArithmeticOperatorPerformance();
	TestStandardSerializationOperatorPerformance();
#endif

#if REGRESSION_LEVEL_2
	TestExtendedArithmeticOperatorPerformance();
#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	TestExtendedSerializationOperatorPerformance();
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

/*
Date run : 3/01/2021
Processor: AMD Ryzen 7 2700x CPU @ 3.70GHz, 8 cores, 16 threads, 75W desktop processor
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

Arithmetic operator performance
 integer<  8>   add/subtract      4194304 per       0.0057685sec -> 727 Mops/sec
 integer< 16>   add/subtract      4194304 per       0.0052956sec -> 792 Mops/sec
 integer< 32>   add/subtract      4194304 per       0.0045509sec -> 921 Mops/sec
 integer< 64>   add/subtract      4194304 per       0.0047554sec -> 882 Mops/sec
 integer<128>   add/subtract      4194304 per       0.0643765sec ->  65 Mops/sec
 integer<  8>   multiplication    1048576 per       0.0011309sec -> 927 Mops/sec
 integer< 16>   multiplication    1048576 per       0.0011109sec -> 943 Mops/sec
 integer< 32>   multiplication     524288 per       0.0004292sec ->   1 Gops/sec
 integer< 64>   multiplication     524288 per       0.0004168sec ->   1 Gops/sec
 integer<128>   multiplication     524288 per       0.0633141sec ->   8 Mops/sec
 integer<  8>   division          1048576 per       0.0046351sec -> 226 Mops/sec
 integer< 16>   division          1048576 per       0.0039042sec -> 268 Mops/sec
 integer< 32>   division           524288 per       0.0017782sec -> 294 Mops/sec
 integer< 64>   division           524288 per       0.0017905sec -> 292 Mops/sec
 integer<128>   division           524288 per       0.0697476sec ->   7 Mops/sec
 integer<  8>   remainder         1048576 per       0.0036293sec -> 288 Mops/sec
 integer< 16>   remainder         1048576 per       0.0035849sec -> 292 Mops/sec
 integer< 32>   remainder          524288 per        0.001824sec -> 287 Mops/sec
 integer< 64>   remainder          524288 per       0.0017972sec -> 291 Mops/sec
 integer<128>   remainder          524288 per       0.0748564sec ->   7 Mops/sec

Serialization operator performance
integer<   8>  ostream                512 per       0.0015546sec -> 329 Kops/sec
integer<  16>  ostream                512 per       0.0028714sec -> 178 Kops/sec
integer<  32>  ostream                512 per       0.0048707sec -> 105 Kops/sec
integer<  64>  ostream                512 per       0.0097528sec ->  52 Kops/sec
integer< 128>  ostream                512 per       0.0227661sec ->  22 Kops/sec
integer< 256>  ostream                512 per       0.0931414sec ->   5 Kops/sec
integer< 512>  ostream                512 per        0.246753sec ->   2 Kops/sec
integer<1024>  ostream                512 per        0.862064sec -> 593  ops/sec
*/

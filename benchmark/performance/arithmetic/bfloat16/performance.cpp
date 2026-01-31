// performance.cpp : performance benchmarking for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

template<typename Bfloat>
void CopyWorkload(size_t NR_OPS) {
	using namespace sw::universal;
	Bfloat a,b,c;

	bool bFail = false;
	size_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i,++j) {
		a.setbits(i);
		b = a;
		c.setbits(j);
		if (b.sign() != c.sign()) {
			bFail = true;
		}
	}
	if (bFail) std::cout << "COPY FAIL\n"; // just a quick double check that all went well
}


/// <summary>
/// measure performance of copying numbers around
/// </summary>
void TestCopyPerformance() {
	using namespace sw::universal;
	std::cout << "bfloat16 copy performance\n";

	uint64_t NR_OPS = 10000000;
	// single block representations
	PerformanceRunner("bfloat16      copy           ", CopyWorkload< sw::universal::bfloat16 >, NR_OPS);
}

template<typename Scalar>
void DecodeWorkload(size_t NR_OPS) {
	using namespace sw::universal;

	bool first = true;
	Scalar a{ 0 };
	volatile float decoded{ 0.0f };
	for (size_t i = 0; i < NR_OPS; ++i) {
		a.setbits(i);
		// float(bfloat16) = decode, assignment is encode
		decoded = float(a);
		Scalar b = static_cast<Scalar>(decoded);
		if (b == Scalar(1.0e34f)) {
			if (!first) {
				first = false;
				std::cout << to_binary(b) << " : " << b << " dummy case to fool the optimizer\n";
			}
		}
	}
}


/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestDecodePerformance() {
	using namespace sw::universal;
	std::cout << "bfloat16 decode operator performance\n";

	uint64_t NR_OPS = 100000;
	PerformanceRunner("bfloat16      decode         ", DecodeWorkload< sw::universal::bfloat16 >, NR_OPS);
}

#ifdef LATER
template<typename Bfloat>
void NormalizeWorkload(size_t NR_OPS) {
	using namespace sw::universal;
	constexpr size_t nbits = Bfloat::nbits;
	constexpr size_t es = Bfloat::es;
	using bt = typename Bfloat::BlockType;
	constexpr size_t fhbits = nbits - es;
	Bfloat a;
	blocktriple<fhbits> b;  // representing significant

	bool bFail = false;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a.setbits(i);
		a.normalize(b);
		if (a.sign() != b.sign()) {
//			std::cout << to_binary(a, true) << " : " << to_triple(b, true) << '\n';
			bFail = true;
		}
	}
	if (bFail) std::cout << "NORMALIZE FAIL\n"; // just a quick double check that all went well
}

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
static void TestNormalizePerformance() {
	using namespace sw::universal;
	cout << endl << "bfloat16 normalize operator performance" << endl;

	uint64_t NR_OPS = 100000;
	PerformanceRunner("bfloat16      normalize      ", NormalizeWorkload< sw::universal::bfloat16 >, NR_OPS * 10);
}
#endif // LATER

// measure performance of conversion operators
[[maybe_unused]] static void TestConversionPerformance() {
	using namespace sw::universal;
	std::cout << "bfloat16 conversion performance\n";

//	uint64_t NR_OPS = 1000000;
}

// measure performance of arithmetic operators
static void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "bfloat16 arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("bfloat16      add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat16 >, NR_OPS);
	PerformanceRunner("bfloat16      multiply       ", MultiplicationWorkload< sw::universal::bfloat16 >, NR_OPS);
	PerformanceRunner("bfloat16      divide         ", DivisionWorkload< sw::universal::bfloat16 >, NR_OPS);
}

// measure baseline float performance of arithmetic operators
static void TestArithmeticOperatorHardwarePerformance() {
	using namespace sw::universal;
	std::cout << "hardware float arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("float         add/subtract   ", AdditionSubtractionWorkload< float >, NR_OPS);
	PerformanceRunner("float         multiply       ", MultiplicationWorkload< float >, NR_OPS);
	PerformanceRunner("float         divide         ", DivisionWorkload< float >, NR_OPS);
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "bfloat16 operator performance benchmarking";

#if MANUAL_TESTING

	using Scalar = bfloat16;
	Scalar a{ 1.0f }, b;
	b = a;
	std::cout << a << " : " << b << std::endl;

	size_t NR_OPS = 10000000;
	PerformanceRunner("bfloat16    copy           ", CopyWorkload< sw::universal::bfloat16 >, NR_OPS);


	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;
	   
	TestCopyPerformance();
	TestDecodePerformance();
#ifdef LATER
	TestNormalizePerformance();
#endif
	TestArithmeticOperatorPerformance();
	TestArithmeticOperatorHardwarePerformance();

	return EXIT_SUCCESS;

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << '\n';
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
Date run : 8/23/2025
Processor: Ryzen 9 8895HX 16-Core Mobile Processor
Memory   : 32GB
System   : 64-bit Windows 11 Pro

bfloat16 operator performance benchmarking
bfloat16 copy performance
bfloat16      copy              10000000 per       0.0023826sec ->   4 Gops/sec
bfloat16 decode operator performance
bfloat16      decode              100000 per       0.0008627sec -> 115 Mops/sec
bfloat16 arithmetic operator performance
bfloat16      add/subtract       1000000 per       0.0064522sec -> 154 Mops/sec
bfloat16      multiply           1000000 per       0.0067358sec -> 148 Mops/sec
bfloat16      divide             1000000 per       0.0078903sec -> 126 Mops/sec
hardware float arithmetic operator performance
float         add/subtract       1000000 per       0.0006051sec ->   1 Gops/sec
float         multiply           1000000 per       0.0006018sec ->   1 Gops/sec
float         divide             1000000 per       0.0005284sec ->   1 Gops/sec
*/

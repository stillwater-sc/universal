// performance.cpp : performance benchmarking for native floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>   // TODO: can this be integrated in category headers?
#include <chrono>
#include <vector>

#include <universal/native/ieee754.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

template<typename NativeFloat>
void CopyWorkload(size_t NR_OPS) {
	using namespace sw::universal;
	NativeFloat a,b,c;

	bool bFail = false;
	size_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i,++j) {
		a = NativeFloat(i);
		b = a;
		c = NativeFloat(j);
		if (b != c) {
			bFail = true;
		}
	}
	if (bFail) std::cout << "COPY FAIL\n"; // just a quick double check that all went well
}

/* 

*/

/// <summary>
/// measure performance of copying numbers around
/// </summary>
void TestCopyPerformance() {
	using namespace sw::universal;
	std::cout << "native floating-point copy performance\n";

	uint64_t NR_OPS = 10000000;
	// single block representations
	std::cout << "single block representations\n";
	PerformanceRunner("float                    copy           ", CopyWorkload< float >, NR_OPS);
	PerformanceRunner("double                   copy           ", CopyWorkload< double >, NR_OPS);
#if LONG_DOUBLE_SUPPORT
	PerformanceRunner("long double              copy           ", CopyWorkload< long double >, NR_OPS);
#endif
}

template<typename NativeFloat>
void DecodeWorkload(size_t NR_OPS) {
	using namespace sw::universal;

	NativeFloat a{ 1.0f };
	size_t success{ 0 };
	bool first{ true };
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = NativeFloat(i);
		bool s;
		uint64_t e, f, b;
		extractFields(a, s, e, f, b);
		if (s == false) {
			++success;
		}
		else {
			if (first) {
				first = false;
				std::cout << typeid(a).name() << " :\n"
					<< to_binary(a) << "\n" 
					<< "sign    : " << (s ? "-1\n" : "+1\n") 
					<< "exponent: " << to_binary(e) << "\n" 
					<< "fraction: " << to_binary(f) << "\n";
			}
		}
	}
	if (success == 0) std::cout << "DECODE FAIL\n"; // just a quick double check that all went well
}

/*

*/

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestDecodePerformance() {
	using namespace sw::universal;
	std::cout << "native floating-point decode operator performance\n";

	uint64_t NR_OPS = 100000;
	// single block representations
	std::cout << "single block representations\n";
	PerformanceRunner("float                    decode         ", DecodeWorkload< float >, NR_OPS);
	PerformanceRunner("double                   decode         ", DecodeWorkload< double >, NR_OPS);
#if LONG_DOUBLE_SUPPORT
	PerformanceRunner("long double              decode         ", DecodeWorkload< long double >, NR_OPS);
#endif
}

// measure performance of conversion operators
void TestConversionPerformance() {
	using namespace sw::universal;
	std::cout << "native floating-point conversion performance\n";

//	uint64_t NR_OPS = 1000000;
}

// Generic set of adds and subtracts for a given number system type
template<typename NativeFloat>
void AdditionSubtractionWorkload(size_t NR_OPS) {
	std::vector<NativeFloat> data = { 0.99999f, -1.00001f };
	NativeFloat a, b{ 1.0625f };
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = b + a;
	}
	if (b == 1.0625f) {
		std::cout << "dummy case to fool the optimizer\n";
	}
}

// Generic set of multiplies for a given number system type
template<typename NativeFloat>
void MultiplicationWorkload(size_t NR_OPS) {
	std::vector<NativeFloat> data = { 0.99999f, 1.00001f };
	NativeFloat a, b{ 1.0625f };
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = b * a;
	}
	if (b == 1.0625f) {
		std::cout << "dummy case to fool the optimizer\n";
	}
}

// Generic set of divides for a given number system type
template<typename NativeFloat>
void DivisionWorkload(size_t NR_OPS) {
	std::vector<NativeFloat> data = { 0.99999f, 1.00001f };
	NativeFloat a, b{ 1.0625f };
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i%2];
		b = b / a;
	}
	if (b == 1.0625f) {
		std::cout << "dummy case to fool the optimizer\n";
	}
}

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	std::cout << "native floating-point  arithmetic operator performance\n";

	uint64_t NR_OPS = 16 * 1024 * 1024;

	sw::universal::PerformanceRunner("float                    add/subtract   ", AdditionSubtractionWorkload< float >, NR_OPS);
	sw::universal::PerformanceRunner("double                   add/subtract   ", AdditionSubtractionWorkload< double >, NR_OPS);
#if LONG_DOUBLE_SUPPORT
	sw::universal::PerformanceRunner("long double              add/subtract   ", AdditionSubtractionWorkload< long double >, NR_OPS);
#endif

	sw::universal::PerformanceRunner("float                    multiply       ", MultiplicationWorkload< float >, NR_OPS);
	sw::universal::PerformanceRunner("double                   multiply       ", MultiplicationWorkload< double >, NR_OPS);
#if LONG_DOUBLE_SUPPORT
	sw::universal::PerformanceRunner("long double              multiply       ", MultiplicationWorkload< long double >, NR_OPS);
#endif

	sw::universal::PerformanceRunner("float                    division       ", DivisionWorkload< float >, NR_OPS);
	sw::universal::PerformanceRunner("double                   division       ", DivisionWorkload< double >, NR_OPS);
#if LONG_DOUBLE_SUPPORT
	sw::universal::PerformanceRunner("long double              division       ", DivisionWorkload< long double >, NR_OPS);
#endif
}

// special values handling

template<typename NativeFloat>
void CustomPerfRunner(const std::string& tag, void (f)(std::vector<NativeFloat>&), std::vector<NativeFloat>& data) {
	using namespace std;
	using namespace std::chrono;

	size_t NR_OPS = data.size();
	steady_clock::time_point begin = steady_clock::now();
	f(data);
	steady_clock::time_point end = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>> (end - begin);
	double elapsed_time = time_span.count();

	cout << tag << ' ' << setw(10) << NR_OPS << " per " << setw(15) << elapsed_time << "sec -> " << sw::universal::toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;
}

template<typename NativeFloat>
void ArrayWorkload(std::vector<NativeFloat>& data) {
	for (size_t i = 0; i < data.size()-1; ++i)
		data[i] = NativeFloat(0.5) * (data[i] + data[i + 1]);
}

template<typename NativeFloat>
void TestSpecialValueWorkload(const std::string& tag, size_t NR_ELEMENTS) {
	std::vector<NativeFloat> data(NR_ELEMENTS);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) data[i] = 0.0;
	CustomPerfRunner(tag + std::string("zeros          "), ArrayWorkload<NativeFloat>, data);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) data[i] = 1.0;
	CustomPerfRunner(tag + std::string("ones           "), ArrayWorkload<NativeFloat>, data);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) data[i] = std::numeric_limits<NativeFloat>::denorm_min();
	CustomPerfRunner(tag + std::string("subnormals     "), ArrayWorkload<NativeFloat>, data);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) data[i] = std::numeric_limits<NativeFloat>::infinity();
	CustomPerfRunner(tag + std::string("Inf            "), ArrayWorkload<NativeFloat>, data);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) data[i] = std::numeric_limits<NativeFloat>::quiet_NaN();
	CustomPerfRunner(tag + std::string("NaN            "), ArrayWorkload<NativeFloat>, data);
}

void TestSpecialValuePerformance() {
	std::cout << "comparative special value processing performance\n";
	constexpr size_t NR_OPS = 1024 * 1024;

	TestSpecialValueWorkload<float>      (std::string("float                    "), NR_OPS);
	TestSpecialValueWorkload<double>     (std::string("double                   "), NR_OPS);
#if LONG_DOUBLE_SUPPORT
	TestSpecialValueWorkload<long double>(std::string("long double              "), NR_OPS);
#endif
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "comparative arithmetic operator performance benchmarking";

#if MANUAL_TESTING

	using Scalar = float;
	Scalar a{ 1.0f }, b;
	b = a;
	std::cout << a << " : " << b << std::endl;

	size_t NR_OPS = 10000000;
	PerformanceRunner("float                    copy           ", CopyWorkload< float >, NR_OPS);
	PerformanceRunner("double                   copy           ", CopyWorkload< double >, NR_OPS);

		std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestCopyPerformance();
	TestDecodePerformance();
	TestArithmeticOperatorPerformance();
	TestSpecialValuePerformance();

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

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
Date run : 2/23/2020
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/

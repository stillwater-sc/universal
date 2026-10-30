// performance.cpp: test suite runner for measuring performance off adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdint>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

/*
   Throughput of the einteger operators at a few operand widths. The timings are reported, never
   judged: they depend on the machine. Each workload does check its own result, and a wrong one
   counts as a failed test case, so the regression run still verifies the arithmetic it times.
*/

namespace sw::universal::internal {

	// PerformanceRunner takes a void(size_t), so the workloads report a wrong result here
	inline int& WorkloadFailures() {
		static int failures = 0;
		return failures;
	}

	inline void Expect(bool ok, const char* workload) {
		if (!ok) {
			++WorkloadFailures();
			std::cerr << "FAIL: " << workload << " workload computed a wrong result\n";
		}
	}

	// an operand of nbits - 1 bits with every limb populated, 0b0101...01, built by shift and add
	// alone: division is one of the operators measured, so it must not shape the operands
	template<typename BlockType, unsigned nbits>
	einteger<BlockType> Operand() {
		einteger<BlockType> a(1);
		for (unsigned i = 2; i < nbits; i += 2) {
			a <<= 2;
			a += 1ll;
		}
		Expect(a.findMsb() == static_cast<int>(nbits) - 2, "operand width");   // the top set bit is nbits - 2
		return a;
	}

	template<typename BlockType, unsigned nbits>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		using EInteger = einteger<BlockType>;
		const EInteger a = Operand<BlockType, nbits>();
		EInteger b = a, c;
		// two operators per iteration
		NR_OPS >>= 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			b = c - a;
		}
		Expect(b == a && c == a + a, "add/subtract");
	}

	template<typename BlockType, unsigned nbits>
	void MultiplicationWorkload(size_t NR_OPS) {
		using EInteger = einteger<BlockType>;
		const EInteger a = Operand<BlockType, nbits>(), b = Operand<BlockType, nbits / 2>();
		EInteger c;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
		}
		Expect(c / b == a && c % b == EInteger(0), "multiplication");
	}

	template<typename BlockType, unsigned nbits>
	void DivisionWorkload(size_t NR_OPS) {
		using EInteger = einteger<BlockType>;
		const EInteger a = Operand<BlockType, nbits>(), b = Operand<BlockType, nbits / 2>() + EInteger(7);
		EInteger q;
		for (size_t i = 0; i < NR_OPS; ++i) {
			q = a / b;
		}
		const EInteger r = a - q * b;
		Expect(!r.isneg() && r < b, "division");
	}

	template<typename BlockType, unsigned nbits>
	void RemainderWorkload(size_t NR_OPS) {
		using EInteger = einteger<BlockType>;
		const EInteger a = Operand<BlockType, nbits>(), b = Operand<BlockType, nbits / 2>() + EInteger(7);
		EInteger r;
		for (size_t i = 0; i < NR_OPS; ++i) {
			r = a % b;
		}
		Expect(r == a - (a / b) * b, "remainder");
	}

	// decimal serialization and the parse back
	template<typename BlockType, unsigned nbits>
	void SerializationWorkload(size_t NR_OPS) {
		using EInteger = einteger<BlockType>;
		const EInteger a = Operand<BlockType, nbits>();
		EInteger b;
		// two operations per iteration
		NR_OPS >>= 1;
		for (size_t i = 0; i < NR_OPS; ++i) {
			std::ostringstream s;
			s << a;
			b.assign(s.str());
		}
		Expect(NR_OPS == 0 || b == a, "serialization");
	}

	void TestArithmeticOperatorPerformance(size_t scale) {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 64ull * 1024ull * scale;
		PerformanceRunner("einteger<uint32_t>   128 bits add/subtract  ", AdditionSubtractionWorkload<std::uint32_t, 128>, NR_OPS);
		PerformanceRunner("einteger<uint32_t>  1024 bits add/subtract  ", AdditionSubtractionWorkload<std::uint32_t, 1024>, NR_OPS / 4);
		PerformanceRunner("einteger<uint8_t>    128 bits add/subtract  ", AdditionSubtractionWorkload<std::uint8_t, 128>, NR_OPS / 4);

		NR_OPS = 16ull * 1024ull * scale;
		PerformanceRunner("einteger<uint32_t>   128 bits multiplication", MultiplicationWorkload<std::uint32_t, 128>, NR_OPS);
		PerformanceRunner("einteger<uint32_t>  1024 bits multiplication", MultiplicationWorkload<std::uint32_t, 1024>, NR_OPS / 16);
		PerformanceRunner("einteger<uint8_t>    128 bits multiplication", MultiplicationWorkload<std::uint8_t, 128>, NR_OPS / 4);

		NR_OPS = 16ull * 1024ull * scale;
		PerformanceRunner("einteger<uint32_t>   128 bits division      ", DivisionWorkload<std::uint32_t, 128>, NR_OPS);
		PerformanceRunner("einteger<uint32_t>  1024 bits division      ", DivisionWorkload<std::uint32_t, 1024>, NR_OPS / 16);
		PerformanceRunner("einteger<uint8_t>    128 bits division      ", DivisionWorkload<std::uint8_t, 128>, NR_OPS / 4);

		NR_OPS = 16ull * 1024ull * scale;
		PerformanceRunner("einteger<uint32_t>   128 bits remainder     ", RemainderWorkload<std::uint32_t, 128>, NR_OPS);
		PerformanceRunner("einteger<uint32_t>  1024 bits remainder     ", RemainderWorkload<std::uint32_t, 1024>, NR_OPS / 16);
	}

	void TestSerializationPerformance(size_t scale) {
		using namespace sw::universal;
		std::cout << "\nSerialization performance (decimal print and parse)\n";

		size_t NR_OPS = 1024ull * scale;
		PerformanceRunner("einteger<uint32_t>   128 bits ostream/parse ", SerializationWorkload<std::uint32_t, 128>, NR_OPS);
		PerformanceRunner("einteger<uint32_t>  1024 bits ostream/parse ", SerializationWorkload<std::uint32_t, 1024>, NR_OPS / 16);
	}

}  // namespace sw::universal::internal

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

	std::string test_suite  = "einteger performance testing";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance(16);
	TestSerializationPerformance(16);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	// each level scales the operation counts up; the CI levels stay well under a second
#if REGRESSION_LEVEL_1
	TestArithmeticOperatorPerformance(1);
	TestSerializationPerformance(1);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	TestArithmeticOperatorPerformance(16);
	TestSerializationPerformance(16);
#endif

	nrOfFailedTestCases += WorkloadFailures();
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Uncaught einteger arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Uncaught einteger internal exception: " << err.what() << std::endl;
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

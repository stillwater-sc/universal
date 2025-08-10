// performance.cpp : performance benchmarking for the standard posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
// configure the posit arithmetic class
#define POSIT_FAST_SPECIALIZATION
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define POSIT_OLD_GENERATION
#ifdef POSIT_OLD_GENERATION
#include <universal/number/posit/posit.hpp>
#else
#include <universal/number/posit2/posit.hpp>
#endif
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

template<typename positConfiguration>
void CopyWorkload(size_t NR_OPS) {
	using namespace sw::universal;
	positConfiguration a,b,c;

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

/*
08/08/2025 Ryzen 9 8945HS
posit copy performance
posit< 8, 2>     copy                100000 per        2.45e-05sec ->   4 Gops/sec
posit<16, 2>     copy                100000 per        3.49e-05sec ->   2 Gops/sec
posit<32, 2>     copy                100000 per        2.01e-05sec ->   4 Gops/sec
posit<64, 2>     copy                100000 per        0.014343sec ->   6 Mops/sec
*/


/// <summary>
/// measure performance of copying numbers around
/// </summary>
void TestCopyPerformance() {
	using namespace sw::universal;
	std::cout << "posit copy performance\n";

	uint64_t NR_OPS = 100000;

#ifdef POSIT_OLD_GENERATION
	PerformanceRunner("posit< 8, 2>     copy           ", CopyWorkload< sw::universal::posit<  8, 2> >, NR_OPS);
	PerformanceRunner("posit<16, 2>     copy           ", CopyWorkload< sw::universal::posit< 16, 2> >, NR_OPS);
	PerformanceRunner("posit<32, 2>     copy           ", CopyWorkload< sw::universal::posit< 32, 2> >, NR_OPS);
	PerformanceRunner("posit<64, 2>     copy           ", CopyWorkload< sw::universal::posit< 64, 2> >, NR_OPS);
#else
	// single block representations
	std::cout << "single block representations\n";
	PerformanceRunner("posit< 8,2,uint8_t>      copy           ", CopyWorkload< sw::universal::posit<  8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<16,5,uint16_t>     copy           ", CopyWorkload< sw::universal::posit< 16, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<32,8,uint32_t>     copy           ", CopyWorkload< sw::universal::posit< 32, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<64,11,uint64_t>    copy           ", CopyWorkload< sw::universal::posit< 64, 2, uint64_t> >, NR_OPS);

	// multi-block representations
	std::cout << "byte representations\n";
	PerformanceRunner("posit<  8,2,uint8_t>     copy           ", CopyWorkload< sw::universal::posit<  8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit< 16,5,uint8_t>     copy           ", CopyWorkload< sw::universal::posit< 16, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit< 32,8,uint8_t>     copy           ", CopyWorkload< sw::universal::posit< 32, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit< 64,11,uint8_t>    copy           ", CopyWorkload< sw::universal::posit< 64, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<128,11,uint8_t>    copy           ", CopyWorkload< sw::universal::posit<128, 2, uint8_t> >, NR_OPS);

	std::cout << "2-byte representations\n";
	PerformanceRunner("posit<  8,2,uint16_t>    copy           ", CopyWorkload< sw::universal::posit<  8, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit< 16,5,uint16_t>    copy           ", CopyWorkload< sw::universal::posit< 16, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit< 32,8,uint16_t>    copy           ", CopyWorkload< sw::universal::posit< 32, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit< 64,11,uint16_t>   copy           ", CopyWorkload< sw::universal::posit< 64, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<128,11,uint16_t>   copy           ", CopyWorkload< sw::universal::posit<128, 2, uint16_t> >, NR_OPS);

	std::cout << "4-byte representations\n";
	PerformanceRunner("posit<  8,2,uint32_t>    copy           ", CopyWorkload< sw::universal::posit<  8, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit< 16,5,uint32_t>    copy           ", CopyWorkload< sw::universal::posit< 16, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit< 32,8,uint32_t>    copy           ", CopyWorkload< sw::universal::posit< 32, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit< 64,11,uint32_t>   copy           ", CopyWorkload< sw::universal::posit< 64, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<128,11,uint32_t>   copy           ", CopyWorkload< sw::universal::posit<128, 2, uint32_t> >, NR_OPS);

	//std::cout << "8-byte representations\n";
	//PerformanceRunner("posit<  8,2,uint64_t>    copy           ", CopyWorkload< sw::universal::posit<  8, 2, uint8_t> >, NR_OPS);
	//PerformanceRunner("posit< 16,5,uint64_t>    copy           ", CopyWorkload< sw::universal::posit< 16, 2, uint16_t> >, NR_OPS);
	//PerformanceRunner("posit< 32,8,uint64_t>    copy           ", CopyWorkload< sw::universal::posit< 32, 2, uint32_t> >, NR_OPS);
	//PerformanceRunner("posit< 64,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit< 64, 2, uint64_t> >, NR_OPS);
	//PerformanceRunner("posit<128,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit<128, 2, uint64_t> >, NR_OPS);

	//std::cout << "very large representations\n";
	//PerformanceRunner("posit< 80,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit< 80, 2, uint64_t> >, NR_OPS);
	//PerformanceRunner("posit< 96,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit< 96, 2, uint64_t> >, NR_OPS);
	//PerformanceRunner("posit<128,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit<128, 2, uint64_t> >, NR_OPS);
	//PerformanceRunner("posit<256,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit<256, 2, uint64_t> >, NR_OPS);
	//PerformanceRunner("posit<512,11,uint64_t>   copy           ", CopyWorkload< sw::universal::posit<512, 2, uint64_t> >, NR_OPS);
#endif
}

template<typename Scalar>
void DecodeWorkload(size_t NR_OPS) {
	using namespace sw::universal;

	Scalar a{ 0 };
	size_t success{ 0 };
	bool first{ true };
	for (size_t i = 0; i < NR_OPS; ++i) {
		a.setbits(i);
		bool s{ false };
#ifdef POSIT_OLD_GENERATION
		positRegime<a.nbits, a.es> r;
		positExponent<a.nbits, a.es> e;
		positFraction<a.fbits + 1> f;
#else
		positRegime<a.nbits, a.es, typename Scalar::BlockType> r;
		positExponent<a.nbits, a.es, typename Scalar::BlockType> e;
		positFraction<a.fbits+1, typename Scalar::BlockType> f;
#endif
		auto raw_bits = a.get();
		decode(raw_bits, s, r, e, f);
		
		if (s == raw_bits.test(a.nbits - 1)) {
			++success;
		}
		else {
			if (a.isnar()) continue;
			if (f.nrBits() > 0) {
				if (first) {
					first = false;
					std::cout << "decode failed for " << a << "\n";
					std::cout << typeid(a).name() << " :\n"
						<< to_binary(a, true) << "\n"
						<< "sign    : " << (s ? "-1\n" : "+1\n")
						<< "regime  : " << r << "\n"
						<< "exponent: " << e << "\n"
						<< "fraction: " << f << "\n";
				}
			}
		}
	}
	if (success == 0) std::cout << "DECODE FAIL\n"; // just a quick double check that all went well
}

/*
08/08/2025 Ryzen 9 8945HS
posit decode operator performance
posit< 8, 2>     decode               10000 per       0.0002613sec ->  38 Mops/sec
posit<16, 2>     decode               10000 per       0.0003376sec ->  29 Mops/sec
posit<32, 2>     decode               10000 per       0.0003629sec ->  27 Mops/sec
posit<64, 2>     decode               10000 per           0.001sec ->  10 Mops/sec
*/

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestDecodePerformance() {
	using namespace sw::universal;
	std::cout << "posit decode operator performance\n";

	uint64_t NR_OPS = 10000;

#ifdef POSIT_OLD_GENERATION
	PerformanceRunner("posit< 8, 2>     decode         ", DecodeWorkload< sw::universal::posit< 8, 2> >, NR_OPS);
	PerformanceRunner("posit<16, 2>     decode         ", DecodeWorkload< sw::universal::posit<16, 2> >, NR_OPS);
	PerformanceRunner("posit<32, 2>     decode         ", DecodeWorkload< sw::universal::posit<32, 2> >, NR_OPS);
	PerformanceRunner("posit<64, 2>     decode         ", DecodeWorkload< sw::universal::posit<64, 2> >, NR_OPS);
#else
	// single block representations
	std::cout << "single block representations\n";
	PerformanceRunner("posit< 8,2,uint8_t>     decode         ", DecodeWorkload< sw::universal::posit< 8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint16_t>    decode         ", DecodeWorkload< sw::universal::posit<16, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint32_t>    decode         ", DecodeWorkload< sw::universal::posit<32, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint64_t>    decode         ", DecodeWorkload< sw::universal::posit<64, 2, uint64_t> >, NR_OPS);

	// multi-block representations
	std::cout << "byte representations\n";
	PerformanceRunner("posit< 8,2,uint8_t>     decode         ", DecodeWorkload< sw::universal::posit< 8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint8_t>     decode         ", DecodeWorkload< sw::universal::posit<16, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint8_t>     decode         ", DecodeWorkload< sw::universal::posit<32, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint8_t>     decode         ", DecodeWorkload< sw::universal::posit<64, 2, uint8_t> >, NR_OPS);

	std::cout << "2-byte representations\n";
	PerformanceRunner("posit< 8,2,uint16_t>    decode         ", DecodeWorkload< sw::universal::posit<  8, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint16_t>    decode         ", DecodeWorkload< sw::universal::posit< 16, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint16_t>    decode         ", DecodeWorkload< sw::universal::posit< 32, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint16_t>    decode         ", DecodeWorkload< sw::universal::posit< 64, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<128,2,uint16_t>   decode         ", DecodeWorkload< sw::universal::posit<128, 2, uint16_t> >, NR_OPS);

	std::cout << "4-byte representations\n";
	PerformanceRunner("posit< 8,2,uint32_t>    decode         ", DecodeWorkload< sw::universal::posit<  8, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint32_t>    decode         ", DecodeWorkload< sw::universal::posit< 16, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint32_t>    decode         ", DecodeWorkload< sw::universal::posit< 32, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint32_t>    decode         ", DecodeWorkload< sw::universal::posit< 64, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<128,2,uint32_t>   decode         ", DecodeWorkload< sw::universal::posit<128, 2, uint32_t> >, NR_OPS);

	std::cout << "8-byte representations\n";
	PerformanceRunner("posit< 8,2,uint64_t>    decode         ", DecodeWorkload< sw::universal::posit<  8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint64_t>    decode         ", DecodeWorkload< sw::universal::posit< 16, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint64_t>    decode         ", DecodeWorkload< sw::universal::posit< 32, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint64_t>    decode         ", DecodeWorkload< sw::universal::posit< 64, 2, uint64_t> >, NR_OPS);
	PerformanceRunner("posit<128,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit<128, 2, uint32_t> >, NR_OPS);

	std::cout << "very large representations\n";
	PerformanceRunner("posit< 80,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit< 80, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit< 96,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit< 96, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<128,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit<128, 2, uint32_t> >, NR_OPS); 
	PerformanceRunner("posit<256,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit<256, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("posit<512,2,uint64_t>   decode         ", DecodeWorkload< sw::universal::posit<512, 2, uint32_t> >, NR_OPS);
#endif
}

#ifdef LATER
template<typename positConfiguration>
void NormalizeWorkload(size_t NR_OPS) {
	using namespace sw::universal;
	constexpr size_t nbits = positConfiguration::nbits;
	constexpr size_t es = positConfiguration::es;
	using bt = typename positConfiguration::BlockType;
	constexpr size_t fhbits = nbits - es;
	posit<nbits, es, bt> a;
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
void TestNormalizePerformance() {
	using namespace sw::universal;
	cout << endl << "posit normalize operator performance" << endl;

	uint64_t NR_OPS = 100000;
	// single block representations
	cout << "single block representations\n";
	PerformanceRunner("posit< 8,2,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::posit< 8, 2> >, NR_OPS * 10);
	PerformanceRunner("posit<16,2,uint16_t>    normalize      ", NormalizeWorkload< sw::universal::posit<16, 2> >, NR_OPS * 10);
	PerformanceRunner("posit<32,2,uint32_t>    normalize      ", NormalizeWorkload< sw::universal::posit<32, 2> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint64_t>    normalize      ", NormalizeWorkload< sw::universal::posit<64, 2> >, NR_OPS);

	// multi-block representations
	cout << "byte representations\n";
	PerformanceRunner("posit< 8,2,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::posit< 8, 2> >, NR_OPS);
	PerformanceRunner("posit<16,2,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::posit<16, 2> >, NR_OPS);
	PerformanceRunner("posit<32,2,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::posit<32, 2> >, NR_OPS);
	PerformanceRunner("posit<64,2,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::posit<64, 2> >, NR_OPS);
}
#endif // LATER

// measure performance of conversion operators
void TestConversionPerformance() {
	using namespace sw::universal;
	std::cout << "posit conversion performance\n";

//	uint64_t NR_OPS = 1000000;
}

/*
08/08/2025 Ryzen 9 8945HS
posit arithmetic operator performance
posit< 8, 2>     add/subtract       1000000 per       0.0087443sec -> 114 Mops/sec
posit<16, 2>     add/subtract       1000000 per        0.007859sec -> 127 Mops/sec
posit<32, 2>     add/subtract       1000000 per       0.0055906sec -> 178 Mops/sec
posit<64, 2>     add/subtract       1000000 per          1.4961sec -> 668 Kops/sec
posit< 8, 2>     division             10000 per        8.57e-05sec -> 116 Mops/sec
posit<16, 2>     division             10000 per        8.45e-05sec -> 118 Mops/sec
posit<32, 2>     division             10000 per        5.56e-05sec -> 179 Mops/sec
posit<64, 2>     division             10000 per        0.281112sec ->  35 Kops/sec
posit< 8, 2>     multiplication       10000 per       0.0001314sec ->  76 Mops/sec
posit<16, 2>     multiplication       10000 per       0.0001199sec ->  83 Mops/sec
posit<32, 2>     multiplication       10000 per        4.55e-05sec -> 219 Mops/sec
posit<64, 2>     multiplication       10000 per        0.093126sec -> 107 Kops/sec
*/

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "posit arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("posit< 8, 2>     add/subtract   ", AdditionSubtractionWorkload< sw::universal::posit< 8,2> >, NR_OPS);
	PerformanceRunner("posit<16, 2>     add/subtract   ", AdditionSubtractionWorkload< sw::universal::posit<16,2> >, NR_OPS);
	PerformanceRunner("posit<32, 2>     add/subtract   ", AdditionSubtractionWorkload< sw::universal::posit<32,2> >, NR_OPS);
	PerformanceRunner("posit<64, 2>     add/subtract   ", AdditionSubtractionWorkload< sw::universal::posit<64,2> >, NR_OPS);

	NR_OPS = 10000;
	PerformanceRunner("posit< 8, 2>     division       ", DivisionWorkload< sw::universal::posit< 8,2> >, NR_OPS);
	PerformanceRunner("posit<16, 2>     division       ", DivisionWorkload< sw::universal::posit<16,2> >, NR_OPS);
	PerformanceRunner("posit<32, 2>     division       ", DivisionWorkload< sw::universal::posit<32,2> >, NR_OPS);
	PerformanceRunner("posit<64, 2>     division       ", DivisionWorkload< sw::universal::posit<64,2> >, NR_OPS);

	// multiplication is the slowest operator
	NR_OPS = 10000;
	PerformanceRunner("posit< 8, 2>     multiplication ", MultiplicationWorkload< sw::universal::posit< 8,2> >, NR_OPS);
	PerformanceRunner("posit<16, 2>     multiplication ", MultiplicationWorkload< sw::universal::posit<16,2> >, NR_OPS);
	PerformanceRunner("posit<32, 2>     multiplication ", MultiplicationWorkload< sw::universal::posit<32,2> >, NR_OPS);
	PerformanceRunner("posit<64, 2>     multiplication ", MultiplicationWorkload< sw::universal::posit<64,2> >, NR_OPS);

}

// conditional compilation
#define MANUAL_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "posit operator performance benchmarking";
	std::cout << tag << std::endl;

#if MANUAL_TESTING

	using Scalar = posit<16, 5, uint16_t>;
	Scalar a{ 1.0f }, b;
	b = a;
	std::cout << a << " : " << b << std::endl;

	size_t NR_OPS = 10000000;
	PerformanceRunner("posit<16,5,uint16_t>    copy           ", CopyWorkload< sw::universal::posit<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("posit<16,5,uint32_t>    copy           ", CopyWorkload< sw::universal::posit<16, 5, uint32_t> >, NR_OPS);

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	   
	TestCopyPerformance();
	TestDecodePerformance();
#ifdef LATER
	TestNormalizePerformance();
#endif
	TestArithmeticOperatorPerformance();

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
Date run : 2/23/2020
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/

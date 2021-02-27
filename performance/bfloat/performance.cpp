// performance.cpp : performance benchmarking for abitrary fixed-precision reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
// configure the bfloat arithmetic class
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/bfloat/bfloat>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/performance_runner.hpp>

/*
   The goal of the arbitrary fixed-precision reals is to provide a constrained 
   linear floating-point type to explore the benefits of multi-precision algorithms.
*/

// measure performance of conversion operators
void TestConversionPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT Conversion operator performance" << endl;

	uint64_t NR_OPS = 1000000;
}

template<typename Scalar>
void DecodeWorkload(uint64_t NR_OPS) {
	using namespace std;
	using namespace sw::universal;

	Scalar a{ 0 };
	size_t success{ 0 };
	bool first{ true };
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		a.set_raw_bits(i);
		bool s{ false };
		blockbinary<a.es, typename Scalar::BlockType> e;
		blockbinary<a.fbits, typename Scalar::BlockType> f;
		sw::universal::decode(a, s, e, f);
		if ((i%2) == f.at(0)) {
			++success;
		}
		else {
			// this shouldn't happen, but found a bug this way with bfloat<64,11,uint64_t> as type
			if (first) {
				first = false;
				cout << typeid(a).name() << " :\n" 
					<< to_binary(a,true) << "\n" 
					<< "sign    : " << (s ? "-1\n" : "+1\n") 
					<< "exponent: " << to_binary(e,true) << "\n" 
					<< "fraction: " << to_binary(f,true) << "\n";
			}
		}
	}
	if (success == 0) cout << "DECODE FAIL\n"; // just a quick double check that all went well
}

/*
2/26/2021
BFLOAT decode operator performance                                                           <---- this includes set_raw_bits()
bfloat<8,2,uint8_t>      decode            10000000 per       0.0105318sec -> 949 Mops/sec
bfloat<16,5,uint16_t>    decode            10000000 per        0.017448sec -> 573 Mops/sec
bfloat<32,8,uint32_t>    decode            10000000 per       0.0158896sec -> 629 Mops/sec
bfloat<64,11,uint64_t>   decode            10000000 per       0.0149587sec -> 668 Mops/sec

2/27/2021
BFLOAT decode operator performance
single block representations
bfloat<8,2,uint8_t>      decode              100000 per        9.81e-05sec ->   1 Gops/sec
bfloat<16,5,uint16_t>    decode              100000 per       0.0001751sec -> 571 Mops/sec
bfloat<32,8,uint32_t>    decode              100000 per       0.0001525sec -> 655 Mops/sec
bfloat<64,11,uint64_t>   decode              100000 per       0.0001251sec -> 799 Mops/sec
byte representations
bfloat<8,2,uint8_t>      decode              100000 per        9.84e-05sec ->   1 Gops/sec
bfloat<16,5,uint8_t>     decode              100000 per       0.0017394sec ->  57 Mops/sec
bfloat<32,8,uint8_t>     decode              100000 per       0.0054993sec ->  18 Mops/sec
bfloat<64,11,uint8_t>    decode              100000 per       0.0114794sec ->   8 Mops/sec
bfloat<128,11,uint8_t>   decode              100000 per       0.0246191sec ->   4 Mops/sec
2-byte representations
bfloat<8,2,uint16_t>     decode              100000 per       0.0001714sec -> 583 Mops/sec
bfloat<16,5,uint16_t>    decode              100000 per       0.0001713sec -> 583 Mops/sec
bfloat<32,8,uint16_t>    decode              100000 per        0.004117sec ->  24 Mops/sec
bfloat<64,11,uint16_t>   decode              100000 per       0.0091907sec ->  10 Mops/sec
bfloat<128,11,uint16_t>  decode              100000 per       0.0209605sec ->   4 Mops/sec
4-byte representations
bfloat<8,2,uint32_t>     decode              100000 per       0.0001122sec -> 891 Mops/sec
bfloat<16,5,uint32_t>    decode              100000 per       0.0005336sec -> 187 Mops/sec
bfloat<32,8,uint32_t>    decode              100000 per        0.000147sec -> 680 Mops/sec
bfloat<64,11,uint32_t>   decode              100000 per        0.009177sec ->  10 Mops/sec
bfloat<128,11,uint32_t>  decode              100000 per       0.0209432sec ->   4 Mops/sec
8-byte representations
bfloat<8,2,uint64_t>     decode              100000 per       0.0001053sec -> 949 Mops/sec
bfloat<16,5,uint64_t>    decode              100000 per       0.0001713sec -> 583 Mops/sec
bfloat<32,8,uint64_t>    decode              100000 per       0.0001472sec -> 679 Mops/sec
bfloat<64,11,uint64_t>   decode              100000 per       0.0001225sec -> 816 Mops/sec
bfloat<128,11,uint64_t>  decode              100000 per       0.0210058sec ->   4 Mops/sec
very large representations
bfloat<80,11,uint64_t>   decode              100000 per       0.0121534sec ->   8 Mops/sec
bfloat<96,11,uint64_t>   decode              100000 per       0.0156355sec ->   6 Mops/sec
bfloat<128,11,uint64_t>  decode              100000 per       0.0210453sec ->   4 Mops/sec
bfloat<256,11,uint64_t>  decode              100000 per       0.0433087sec ->   2 Mops/sec
bfloat<256,11,uint64_t>  decode              100000 per       0.0447077sec ->   2 Mops/sec
*/

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestDecodePerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT decode operator performance" << endl;

	uint64_t NR_OPS = 100000;
	// single block representations
	cout << "single block representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      decode         ", DecodeWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    decode         ", DecodeWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    decode         ", DecodeWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   decode         ", DecodeWorkload< sw::universal::bfloat<64, 11, uint64_t> >, NR_OPS);

	// multi-block representations
	cout << "byte representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      decode         ", DecodeWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint8_t>     decode         ", DecodeWorkload< sw::universal::bfloat<16, 5, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint8_t>     decode         ", DecodeWorkload< sw::universal::bfloat<32, 8, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint8_t>    decode         ", DecodeWorkload< sw::universal::bfloat<64, 11, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint8_t>   decode         ", DecodeWorkload< sw::universal::bfloat<128, 11, uint8_t> >, NR_OPS);

	cout << "2-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint16_t>     decode         ", DecodeWorkload< sw::universal::bfloat<8, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    decode         ", DecodeWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint16_t>    decode         ", DecodeWorkload< sw::universal::bfloat<32, 8, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint16_t>   decode         ", DecodeWorkload< sw::universal::bfloat<64, 11, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint16_t>  decode         ", DecodeWorkload< sw::universal::bfloat<128, 11, uint16_t> >, NR_OPS);

	cout << "4-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint32_t>     decode         ", DecodeWorkload< sw::universal::bfloat<8, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint32_t>    decode         ", DecodeWorkload< sw::universal::bfloat<16, 5, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    decode         ", DecodeWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint32_t>   decode         ", DecodeWorkload< sw::universal::bfloat<64, 11, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint32_t>  decode         ", DecodeWorkload< sw::universal::bfloat<128, 11, uint32_t> >, NR_OPS);

	cout << "8-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint64_t>     decode         ", DecodeWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint64_t>    decode         ", DecodeWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint64_t>    decode         ", DecodeWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   decode         ", DecodeWorkload< sw::universal::bfloat<64, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint64_t>  decode         ", DecodeWorkload< sw::universal::bfloat<128, 11, uint64_t> >, NR_OPS);

	cout << "very large representations\n";
	PerformanceRunner("bfloat<80,11,uint64_t>   decode         ", DecodeWorkload< sw::universal::bfloat<80, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<96,11,uint64_t>   decode         ", DecodeWorkload< sw::universal::bfloat<96, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint64_t>  decode         ", DecodeWorkload< sw::universal::bfloat<128, 11, uint64_t> >, NR_OPS); 
	PerformanceRunner("bfloat<256,11,uint64_t>  decode         ", DecodeWorkload< sw::universal::bfloat<256, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<256,11,uint64_t>  decode         ", DecodeWorkload< sw::universal::bfloat<256, 11, uint64_t> >, NR_OPS);
}

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT Arithmetic operator performance" << endl;

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("bfloat<8,2,uint8_t>      add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<8,2,uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<64,11,uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<128,11,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("bfloat<128,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("bfloat<256,15,uint64_t   add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("bfloat<512,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("bfloat<1024,15,uint64_t> add/subtract   ", AdditionSubtractionWorkload< sw::universal::bfloat<1024,15,uint64_t> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("bfloat<8,2,uint16_t>     division       ", DivisionWorkload< sw::universal::bfloat<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    division       ", DivisionWorkload< sw::universal::bfloat<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    division       ", DivisionWorkload< sw::universal::bfloat<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   division       ", DivisionWorkload< sw::universal::bfloat<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("bfloat<128,15,uint64_t>  division       ", DivisionWorkload< sw::universal::bfloat<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("bfloat<256,15,uint64_t   division       ", DivisionWorkload< sw::universal::bfloat<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("bfloat<512,15,uint64_t>  division       ", DivisionWorkload< sw::universal::bfloat<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("bfloat<1024,15,uint64_t> division       ", DivisionWorkload< sw::universal::bfloat<1024,15,uint64_t> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("bfloat<8,2,uint16_t>     multiplication ", MultiplicationWorkload< sw::universal::bfloat<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    multiplication ", MultiplicationWorkload< sw::universal::bfloat<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    multiplication ", MultiplicationWorkload< sw::universal::bfloat<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   multiplication ", MultiplicationWorkload< sw::universal::bfloat<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("bfloat<128,15,uint64_t>  multiplication ", MultiplicationWorkload< sw::universal::bfloat<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("bfloat<256,15,uint64_t   multiplication ", MultiplicationWorkload< sw::universal::bfloat<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("bfloat<512,15,uint64_t>  multiplication ", MultiplicationWorkload< sw::universal::bfloat<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("bfloat<1024,15,uint64_t> multiplication ", MultiplicationWorkload< sw::universal::bfloat<1024,15,uint64_t> >, NR_OPS / 16);

}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "BFLOAT operator performance benchmarking";

#if MANUAL_TESTING

	using Scalar = bfloat<64, 11, uint64_t>;
	Scalar a;
	a.set_raw_bits(0xEEEEEEEEEEEEEEEEull);
	bool s{ false };
	blockbinary<a.es, typename Scalar::BlockType> e;
	blockbinary<a.fbits, typename Scalar::BlockType> f;
	sw::universal::decode(a, s, e, f);
	cout << typeid(a).name() << " :\n"
		<< to_binary(a, true) << "\n"
		<< "sign    : " << (s ? "-1\n" : "+1\n")
		<< "exponent: " << to_binary(e, true) << "\n"
		<< "fraction: " << to_binary(f, true) << "\n";

	cout << "nbits: " << a.nbits << endl;
	cout << "es   : " << a.es << endl;
	cout << "fbits: " << a.fbits << endl;

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestDecodePerformance();
	TestArithmeticOperatorPerformance();

#if STRESS_TESTING

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
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
Date run : 2/23/2020
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/

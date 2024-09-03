//  performance.cpp : performance benchmarking for fixed-sized, arbitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <chrono>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

// test performance of shift operator on integer<> class
void TestShiftOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nINTEGER Logical shift operator performance\n";

	constexpr uint64_t NR_OPS = 1000000;

	PerformanceRunner("integer<   8> shifts        ", ShiftPerformanceWorkload< sw::universal::integer<   8, uint8_t > >, NR_OPS);
	PerformanceRunner("integer<  16> shifts        ", ShiftPerformanceWorkload< sw::universal::integer<  16, uint16_t> >, NR_OPS);
	PerformanceRunner("integer<  32> shifts        ", ShiftPerformanceWorkload< sw::universal::integer<  32, uint32_t> >, NR_OPS);
	PerformanceRunner("integer<  64> shifts        ", ShiftPerformanceWorkload< sw::universal::integer<  64, uint64_t> >, NR_OPS);
	PerformanceRunner("integer< 128> shifts        ", ShiftPerformanceWorkload< sw::universal::integer< 128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("integer< 256> shifts        ", ShiftPerformanceWorkload< sw::universal::integer< 256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("integer< 512> shifts        ", ShiftPerformanceWorkload< sw::universal::integer< 512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("integer<1024> shifts        ", ShiftPerformanceWorkload< sw::universal::integer<1024, uint32_t> >, NR_OPS / 16);
}

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nINTEGER Arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("integer<   8> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<   8, uint8_t > >, NR_OPS);
	PerformanceRunner("integer<  16> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<  16, uint16_t> >, NR_OPS);
	PerformanceRunner("integer<  32> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<  32, uint32_t> >, NR_OPS);
	PerformanceRunner("integer<  64> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<  64, uint64_t> >, NR_OPS);
	PerformanceRunner("integer< 128> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("integer< 256> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("integer< 512> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer< 512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("integer<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::integer<1024, uint32_t> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("integer<   8> division      ", DivisionWorkload< sw::universal::integer<   8, uint8_t> >, NR_OPS);
	PerformanceRunner("integer<  16> division      ", DivisionWorkload< sw::universal::integer<  16, uint16_t> >, NR_OPS);
	PerformanceRunner("integer<  32> division      ", DivisionWorkload< sw::universal::integer<  32, uint32_t> >, NR_OPS);
	PerformanceRunner("integer<  64> division      ", DivisionWorkload< sw::universal::integer<  64, uint64_t> >, NR_OPS / 2);
	PerformanceRunner("integer< 128> division      ", DivisionWorkload< sw::universal::integer< 128, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("integer< 512> division      ", DivisionWorkload< sw::universal::integer< 512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("integer<1024> division      ", DivisionWorkload< sw::universal::integer<1024, uint32_t> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("integer<   8> remainder     ", RemainderWorkload< sw::universal::integer<   8, uint8_t > >, NR_OPS);
	PerformanceRunner("integer<  16> remainder     ", RemainderWorkload< sw::universal::integer<  16, uint16_t> >, NR_OPS);
	PerformanceRunner("integer<  32> remainder     ", RemainderWorkload< sw::universal::integer<  32, uint32_t> >, NR_OPS);
	PerformanceRunner("integer<  64> remainder     ", RemainderWorkload< sw::universal::integer<  64, uint64_t> >, NR_OPS / 2);
	PerformanceRunner("integer< 128> remainder     ", RemainderWorkload< sw::universal::integer< 128, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("integer< 512> remainder     ", RemainderWorkload< sw::universal::integer< 512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("integer<1024> remainder     ", RemainderWorkload< sw::universal::integer<1024, uint32_t> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("integer<   8> multiplication", MultiplicationWorkload< sw::universal::integer<   8, uint8_t > >, NR_OPS);
	PerformanceRunner("integer<  16> multiplication", MultiplicationWorkload< sw::universal::integer<  16, uint16_t> >, NR_OPS);
	PerformanceRunner("integer<  32> multiplication", MultiplicationWorkload< sw::universal::integer<  32, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("integer<  64> multiplication", MultiplicationWorkload< sw::universal::integer<  64, uint64_t> >, NR_OPS / 4);
	PerformanceRunner("integer< 128> multiplication", MultiplicationWorkload< sw::universal::integer< 128, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("integer< 256> multiplication", MultiplicationWorkload< sw::universal::integer< 256, uint32_t> >, NR_OPS / 16);
	PerformanceRunner("integer< 512> multiplication", MultiplicationWorkload< sw::universal::integer< 512, uint32_t> >, NR_OPS / 16);
	PerformanceRunner("integer<1024> multiplication", MultiplicationWorkload< sw::universal::integer<1024, uint32_t> >, NR_OPS / 32);
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "Integer operator performance benchmarking";

#if MANUAL_TESTING

	{
		// test a very large integer
		integer<1024 * 128, uint32_t> a, b, c;
		a = 1;
		b = 1234567890;
		c = a * b;
		c /= a;
		if (c == b) cout << "PASS\n";
	}


	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

#if STRESS_TESTING

#endif // STRESS_TESTING
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

Integer operator performance benchmarking

Logical shift operator performance
integer<16>   shifts            1000000 per       0.0099091sec -> 100 Mops/sec
integer<32>   shifts            1000000 per       0.0453919sec ->  22 Mops/sec
integer<64>   shifts            1000000 per        0.178824sec ->   5 Mops/sec
integer<128>  shifts             500000 per        0.269217sec ->   1 Mops/sec
integer<256>  shifts             250000 per        0.266083sec -> 939 Kops/sec
integer<512>  shifts             125000 per        0.285764sec -> 437 Kops/sec
integer<1024> shifts              62500 per        0.277351sec -> 225 Kops/sec

Arithmetic operator performance
integer<16>   add/subtract      1000000 per       0.0095743sec -> 104 Mops/sec
integer<32>   add/subtract      1000000 per       0.0148453sec ->  67 Mops/sec
integer<64>   add/subtract      1000000 per       0.0474723sec ->  21 Mops/sec
integer<128>  add/subtract       500000 per       0.0382776sec ->  13 Mops/sec
integer<256>  add/subtract       250000 per       0.0268772sec ->   9 Mops/sec
integer<512>  add/subtract       125000 per       0.0275169sec ->   4 Mops/sec
integer<1024> add/subtract        62500 per       0.0294444sec ->   2 Mops/sec
integer<16>   division            32768 per       0.0050946sec ->   6 Mops/sec
integer<32>   division            32768 per       0.0083216sec ->   3 Mops/sec
integer<64>   division            16384 per       0.0074452sec ->   2 Mops/sec
integer<128>  division             8192 per       0.0071203sec ->   1 Mops/sec
integer<512>  division             4096 per       0.0148553sec -> 275 Kops/sec
integer<1024> division             2048 per       0.0154237sec -> 132 Kops/sec
integer<16>   remainder           32768 per       0.0051223sec ->   6 Mops/sec
integer<32>   remainder           32768 per       0.0082141sec ->   3 Mops/sec
integer<64>   remainder           16384 per       0.0077429sec ->   2 Mops/sec
integer<128>  remainder            8192 per       0.0078737sec ->   1 Mops/sec
integer<512>  remainder            4096 per       0.0148961sec -> 274 Kops/sec
integer<1024> remainder            2048 per       0.0150371sec -> 136 Kops/sec
integer<16>   multiplication      32768 per       0.0232329sec ->   1 Mops/sec
integer<32>   multiplication      16384 per       0.0424617sec -> 385 Kops/sec
integer<64>   multiplication       8192 per         0.08589sec ->  95 Kops/sec
integer<128>  multiplication       4096 per        0.166093sec ->  24 Kops/sec
integer<512>  multiplication       2048 per         1.33028sec ->   1 Kops/sec
integer<1024> multiplication       1024 per         2.58557sec -> 396  ops/sec
*/

/*
ETLO
Date run : 03/19/2022
Processor: AMD Ryzen 7 2700X Eight-Core Processor 3.70 GHz desktop 105W processor
Cache    : L1 96KB/core (768kB total), L2 512k/core (4.0MB total), L3 16.0MB
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

Integer operator performance benchmarking

with default uint8_t BlockType
INTEGER Logical shift operator performance
integer<8>    shifts            1000000 per           1e-07sec ->  10 Tops/sec
integer<16>   shifts            1000000 per       0.0189153sec ->  52 Mops/sec
integer<32>   shifts            1000000 per       0.0175485sec ->  56 Mops/sec
integer<64>   shifts            1000000 per       0.0391782sec ->  25 Mops/sec
integer<128>  shifts             500000 per       0.0209925sec ->  23 Mops/sec
integer<256>  shifts             250000 per       0.0289021sec ->   8 Mops/sec
integer<512>  shifts             125000 per        0.019476sec ->   6 Mops/sec
integer<1024> shifts              62500 per       0.0253791sec ->   2 Mops/sec

INTEGER Arithmetic operator performance
integer<8>    add/subtract      1000000 per       0.0005549sec ->   1 Gops/sec
integer<16>   add/subtract      1000000 per       0.0086505sec -> 115 Mops/sec
integer<32>   add/subtract      1000000 per       0.0099939sec -> 100 Mops/sec
integer<64>   add/subtract      1000000 per        0.015391sec ->  64 Mops/sec
integer<128>  add/subtract       500000 per       0.0096107sec ->  52 Mops/sec
integer<256>  add/subtract       250000 per       0.0085242sec ->  29 Mops/sec
integer<512>  add/subtract       125000 per       0.0069788sec ->  17 Mops/sec
integer<1024> add/subtract        62500 per       0.0076963sec ->   8 Mops/sec
integer<8>    division            32768 per       0.0023556sec ->  13 Mops/sec
integer<16>   division            32768 per       0.0029534sec ->  11 Mops/sec
integer<32>   division            32768 per       0.0035336sec ->   9 Mops/sec
integer<64>   division            16384 per       0.0024298sec ->   6 Mops/sec
integer<128>  division             8192 per       0.0018567sec ->   4 Mops/sec
integer<512>  division             4096 per       0.0031357sec ->   1 Mops/sec
integer<1024> division             2048 per       0.0029386sec -> 696 Kops/sec
integer<8>    remainder           32768 per        0.002537sec ->  12 Mops/sec
integer<16>   remainder           32768 per       0.0033418sec ->   9 Mops/sec
integer<32>   remainder           32768 per       0.0037914sec ->   8 Mops/sec
integer<64>   remainder           16384 per       0.0025117sec ->   6 Mops/sec
integer<128>  remainder            8192 per       0.0017473sec ->   4 Mops/sec
integer<512>  remainder            4096 per       0.0031309sec ->   1 Mops/sec
integer<1024> remainder            2048 per       0.0029546sec -> 693 Kops/sec
integer<8>    multiplication      32768 per       0.0022627sec ->  14 Mops/sec
integer<16>   multiplication      32768 per         0.00256sec ->  12 Mops/sec
integer<32>   multiplication      16384 per       0.0014768sec ->  11 Mops/sec
integer<64>   multiplication       8192 per       0.0012147sec ->   6 Mops/sec
integer<128>  multiplication       4096 per       0.0019705sec ->   2 Mops/sec
integer<512>  multiplication       2048 per        0.011195sec -> 182 Kops/sec
integer<1024> multiplication       1024 per       0.0205978sec ->  49 Kops/sec


With tailored BlockType
Integer operator performance benchmarking

INTEGER Logical shift operator performance
integer<   8> shifts            1000000 per               0sec ->   0  ops/sec
integer<  16> shifts            1000000 per       0.0211381sec ->  47 Mops/sec
integer<  32> shifts            1000000 per       0.0190994sec ->  52 Mops/sec
integer<  64> shifts            1000000 per       0.0326373sec ->  30 Mops/sec
integer< 128> shifts             500000 per       0.0083589sec ->  59 Mops/sec
integer< 256> shifts             250000 per       0.0062384sec ->  40 Mops/sec
integer< 512> shifts             125000 per       0.0045322sec ->  27 Mops/sec
integer<1024> shifts              62500 per       0.0055964sec ->  11 Mops/sec

INTEGER Arithmetic operator performance
integer<   8> add/subtract      1000000 per       0.0005934sec ->   1 Gops/sec
integer<  16> add/subtract      1000000 per       0.0005815sec ->   1 Gops/sec
integer<  32> add/subtract      1000000 per       0.0008533sec ->   1 Gops/sec
integer<  64> add/subtract      1000000 per       0.0003468sec ->   2 Gops/sec
integer< 128> add/subtract       500000 per       0.0052717sec ->  94 Mops/sec
integer< 256> add/subtract       250000 per       0.0025542sec ->  97 Mops/sec
integer< 512> add/subtract       125000 per       0.0021965sec ->  56 Mops/sec
integer<1024> add/subtract        62500 per        0.001869sec ->  33 Mops/sec
integer<   8> division            32768 per       0.0023856sec ->  13 Mops/sec
integer<  16> division            32768 per       0.0026805sec ->  12 Mops/sec
integer<  32> division            32768 per       0.0037076sec ->   8 Mops/sec
integer<  64> division            16384 per       0.0005425sec ->  30 Mops/sec
integer< 128> division             8192 per       0.0011302sec ->   7 Mops/sec
integer< 512> division             4096 per       0.0010917sec ->   3 Mops/sec
integer<1024> division             2048 per        0.001076sec ->   1 Mops/sec
integer<   8> remainder           32768 per       0.0025818sec ->  12 Mops/sec
integer<  16> remainder           32768 per       0.0028768sec ->  11 Mops/sec
integer<  32> remainder           32768 per       0.0035723sec ->   9 Mops/sec
integer<  64> remainder           16384 per       0.0004941sec ->  33 Mops/sec
integer< 128> remainder            8192 per       0.0008867sec ->   9 Mops/sec
integer< 512> remainder            4096 per       0.0009781sec ->   4 Mops/sec
integer<1024> remainder            2048 per       0.0009335sec ->   2 Mops/sec
integer<   8> multiplication      32768 per       0.0023107sec ->  14 Mops/sec
integer<  16> multiplication      32768 per       0.0024994sec ->  13 Mops/sec
integer<  32> multiplication      16384 per       0.0012404sec ->  13 Mops/sec
integer<  64> multiplication       8192 per       0.0007763sec ->  10 Mops/sec
integer< 128> multiplication       4096 per       0.0005118sec ->   8 Mops/sec
integer< 256> multiplication       2048 per       0.0003702sec ->   5 Mops/sec
integer< 512> multiplication       2048 per       0.0009729sec ->   2 Mops/sec
integer<1024> multiplication       1024 per        0.001776sec -> 576 Kops/sec

*/

/*
ETLO
Date run : 03/24/2022
Processor: AMD Ryzen 7 2700X Eight-Core Processor 3.70 GHz desktop 105W processor
Cache    : L1 96KB/core (768kB total), L2 512k/core (4.0MB total), L3 16.0MB
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

optimization: special casing single block configurations: 
see results for integer<64, uint64_t>
add/sub: 3 Gops/sec
mul    : 1 Gops/sec

Integer operator performance benchmarking

INTEGER Logical shift operator performance
integer<   8> shifts            1000000 per               0sec ->   0  ops/sec
integer<  16> shifts            1000000 per       0.0164093sec ->  60 Mops/sec
integer<  32> shifts            1000000 per       0.0204416sec ->  48 Mops/sec
integer<  64> shifts            1000000 per       0.0319357sec ->  31 Mops/sec
integer< 128> shifts             500000 per       0.0087293sec ->  57 Mops/sec
integer< 256> shifts             250000 per       0.0051965sec ->  48 Mops/sec
integer< 512> shifts             125000 per       0.0040006sec ->  31 Mops/sec
integer<1024> shifts              62500 per       0.0039462sec ->  15 Mops/sec

INTEGER Arithmetic operator performance
integer<   8> add/subtract      1000000 per       0.0003457sec ->   2 Gops/sec
integer<  16> add/subtract      1000000 per       0.0003146sec ->   3 Gops/sec
integer<  32> add/subtract      1000000 per       0.0006857sec ->   1 Gops/sec
integer<  64> add/subtract      1000000 per       0.0003146sec ->   3 Gops/sec
integer< 128> add/subtract       500000 per       0.0050097sec ->  99 Mops/sec
integer< 256> add/subtract       250000 per       0.0025866sec ->  96 Mops/sec
integer< 512> add/subtract       125000 per       0.0018629sec ->  67 Mops/sec
integer<1024> add/subtract        62500 per       0.0025402sec ->  24 Mops/sec
integer<   8> division            32768 per        0.002713sec ->  12 Mops/sec
integer<  16> division            32768 per       0.0028828sec ->  11 Mops/sec
integer<  32> division            32768 per       0.0034744sec ->   9 Mops/sec
integer<  64> division            16384 per       0.0006312sec ->  25 Mops/sec
integer< 128> division             8192 per       0.0012048sec ->   6 Mops/sec
integer< 512> division             4096 per       0.0012505sec ->   3 Mops/sec
integer<1024> division             2048 per        0.001018sec ->   2 Mops/sec
integer<   8> remainder           32768 per       0.0027676sec ->  11 Mops/sec
integer<  16> remainder           32768 per       0.0031366sec ->  10 Mops/sec
integer<  32> remainder           32768 per       0.0035994sec ->   9 Mops/sec
integer<  64> remainder           16384 per         0.00049sec ->  33 Mops/sec
integer< 128> remainder            8192 per       0.0008721sec ->   9 Mops/sec
integer< 512> remainder            4096 per       0.0009715sec ->   4 Mops/sec
integer<1024> remainder            2048 per       0.0008951sec ->   2 Mops/sec
integer<   8> multiplication      32768 per         3.4e-05sec -> 963 Mops/sec
integer<  16> multiplication      32768 per         3.4e-05sec -> 963 Mops/sec
integer<  32> multiplication      16384 per        1.73e-05sec -> 947 Mops/sec
integer<  64> multiplication       8192 per         6.6e-06sec ->   1 Gops/sec
integer< 128> multiplication       4096 per       0.0005048sec ->   8 Mops/sec
integer< 256> multiplication       2048 per       0.0003359sec ->   6 Mops/sec
integer< 512> multiplication       2048 per       0.0009838sec ->   2 Mops/sec
integer<1024> multiplication       1024 per       0.0018267sec -> 560 Kops/sec
*/
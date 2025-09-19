// performance.cpp : performance benchmarking for the standard posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit_c_api.h>
#include <universal/benchmark/performance_runner.h>

void CopyWorkloadPosit8(size_t NR_OPS) {
	posit8_t a,b,c;

	bool bFail = false;
	uint8_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i,++j) {
		a = posit8_reinterpret(i);
		b = a;
		c = posit8_reinterpret(j);
		if (posit8_bits(b) != posit8_bits(c)) {
			bFail = true;
		}
		//printf("posit8_t: %d %d %d\n", i, j, posit8_bits(c));
	}
	if (bFail) printf("COPY FAIL\n"); // just a quick double check that all went well
}

void CopyWorkloadPosit16(size_t NR_OPS) {
	posit16_t a, b, c;

	bool bFail = false;
	uint16_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i, ++j) {
		a = posit16_reinterpret(i);
		b = a;
		c = posit16_reinterpret(j);
		if (posit16_bits(b) != posit16_bits(c)) {
			bFail = true;
		}
	}
	if (bFail) printf("COPY FAIL\n"); // just a quick double check that all went well
}

void CopyWorkloadPosit32(size_t NR_OPS) {
	posit32_t a, b, c;

	bool bFail = false;
	uint32_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i, ++j) {
		a = posit32_reinterpret(i);
		b = a;
		c = posit32_reinterpret(j);
		if (posit32_bits(b) != posit32_bits(c)) {
			bFail = true;
		}
	}
	if (bFail) printf("COPY FAIL\n"); // just a quick double check that all went well
}

void CopyWorkloadPosit64(size_t NR_OPS) {
	posit64_t a, b, c;

	bool bFail = false;
	uint64_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i, ++j) {
		a = posit64_reinterpret(i);
		b = a;
		c = posit64_reinterpret(j);
		if (posit64_bits(b) != posit64_bits(c)) {
			bFail = true;
		}
	}
	if (bFail) printf("COPY FAIL\n"); // just a quick double check that all went well
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
void TestCopyPerformance(void) {
	printf("posit copy performance\n");

	uint64_t NR_OPS = 10000;

	PerformanceRunner("posit8_t      copy           ", CopyWorkloadPosit8, NR_OPS);
	PerformanceRunner("posit16_t     copy           ", CopyWorkloadPosit16, NR_OPS);
	PerformanceRunner("posit32_t     copy           ", CopyWorkloadPosit32, NR_OPS);
	PerformanceRunner("posit64_t     copy           ", CopyWorkloadPosit64, NR_OPS);

}

void DecodeWorkloadPosit8(size_t NR_OPS) {
	posit8_t a;
	size_t success = 1;
	bool first = true;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = posit8_reinterpret(i);
		uint16_t bits = posit8_bits(a);
		if (bits%2 != i%2) {
			if (first) {
				first = false;
				printf("decode failed for %d\n", (int)i);
				printf("posit8_t : 0x%x\n", bits);
			}
			success = 0;
		}
	}
	if (success == 0) printf("DECODE FAIL\n"); // just a quick double check that all went well
}

void DecodeWorkloadPosit16(size_t NR_OPS) {
	posit16_t a;
	size_t success = 1;
	bool first = true;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = posit16_reinterpret(i);
		uint16_t bits = posit16_bits(a);
		if (bits%2 != i%2) {
			if (first) {
				first = false;
				printf("decode failed for %d\n", (int)i);
				printf("posit16_t : 0x%x\n", bits);
			}
			success = 0;
		}
	}
	if (success == 0) printf("DECODE FAIL\n"); // just a quick double check that all went well
}

void DecodeWorkloadPosit32(size_t NR_OPS) {
	posit32_t a;
	size_t success = 1;
	bool first = true;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = posit32_reinterpret(i);
		uint32_t bits = posit32_bits(a);
		if (bits%2 != i%2) {
			if (first) {
				first = false;
				printf("decode failed for %d\n", (int)i);
				printf("posit32_t : 0x%x\n", bits);
			}
			success = 0;
		}
	}
	if (success == 0) printf("DECODE FAIL\n"); // just a quick double check that all went well
}

#include <inttypes.h>

void DecodeWorkloadPosit64(size_t NR_OPS) {
	posit64_t a;
	size_t success = 1;
	bool first = true;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = posit64_reinterpret(i);
		uint64_t bits = posit64_bits(a);
		if (bits%2 != i%2) {
			if (first) {
				first = false;
				printf("decode failed for %d\n", (int)i);
                // Replace this line:
                // printf("posit64_t : 0x%lx\n", bits);
                // With this portable fix:
                printf("posit64_t : 0x%" PRIu64 "\n", bits);

			}
			success = 0;
		}
	}
	if (success == 0) printf("DECODE FAIL\n"); // just a quick double check that all went well
}

/*
08/08/2025 Ryzen 9 8945HS
posit decode operator performance
posit8_t      decode               10000 per     0.000004000 sec ->   2 G ops/sec
posit16_t     decode               10000 per     0.000007000 sec ->   1 G ops/sec
posit32_t     decode               10000 per     0.000007000 sec ->   1 G ops/sec
posit64_t     decode               10000 per     0.000000000 sec ->   0   ops/sec
*/

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestDecodePerformance(void) {
	printf("posit decode operator performance\n");

	uint64_t NR_OPS = 10000;

	PerformanceRunner("posit8_t      decode         ", DecodeWorkloadPosit8, NR_OPS);
	PerformanceRunner("posit16_t     decode         ", DecodeWorkloadPosit16, NR_OPS);
	PerformanceRunner("posit32_t     decode         ", DecodeWorkloadPosit32, NR_OPS);
	PerformanceRunner("posit64_t     decode         ", DecodeWorkloadPosit64, NR_OPS);

}

// measure performance of conversion operators
void TestConversionPerformance(void) {
	printf("posit conversion performance\n");

//	uint64_t NR_OPS = 1000000;
}

void printDummy() {
	printf("dummy case to fool the optimizer\n");
}

// Generic set of adds and subtracts for a given number system type
void AdditionSubtractionWorkloadPosit8(size_t NR_OPS) {
	posit8_t data[2] = {posit8_fromf(0.99999f), posit8_fromf(-1.00001)};
	posit8_t a, b = posit8_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit8_add(a, b);
	}
	if (posit8_tof(b) == 0.0f) {
		printDummy();
	}
	//printf("%f\n", posit8_tof(b));
}
void AdditionSubtractionWorkloadPosit16(size_t NR_OPS) {
	posit16_t data[2] = { posit16_fromf(0.99999f), posit16_fromf(-1.00001) };
	posit16_t a, b = posit16_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit16_add(a, b);
	}
	if (posit16_tof(b) == 0.0f) {
		printDummy();
	}
	//printf("%f\n", posit16_tof(b));
}
void AdditionSubtractionWorkloadPosit32(size_t NR_OPS) {
	posit32_t data[2] = { posit32_fromf(0.99999f), posit32_fromf(-1.00001) };
	posit32_t a, b = posit32_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit32_add(a, b);
	}
	if (posit32_tof(b) == 0.0f) {
		printDummy();
	}
	//printf("%f\n", posit32_tof(b));
}
void AdditionSubtractionWorkloadPosit64(size_t NR_OPS) {
	posit64_t data[2] = { posit64_fromf(0.99999f), posit64_fromf(-1.00001) };
	posit64_t a, b = posit64_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit64_add(a, b);
	}
	if (posit64_tof(b) == 0.0f) {
		printDummy();
	}
	//printf("%g\n", posit64_tof(b));
}
// Generic set of multiplies for a given number system type
void MultiplicationWorkloadPosit8(size_t NR_OPS) {
	posit8_t data[2] = { posit8_fromf(0.99999f), posit8_fromf(-1.00001) };
	posit8_t a, b = posit8_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit8_mul(a, b);
	}
	if (posit8_tof(b) == -1.0f) {
		a = posit8_fromf(0.0f), b = posit8_fromf(1.0625f);
		for (size_t i = 1; i < 10; ++i) {
			a = data[i % 2];
			printf("%f : %f\n", posit8_tof(a), posit8_tof(b));
			b = posit8_mul(a, b);
		}
		printDummy();
	}
}
void MultiplicationWorkloadPosit16(size_t NR_OPS) {
	posit16_t data[2] = { posit16_fromf(0.99999f), posit16_fromf(-1.00001) };
	posit16_t a, b = posit16_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit16_mul(a, b);
	}
	if (posit16_tof(b) == -1.0f) {
		a = posit16_fromf(0.0f), b = posit16_fromf(1.0625f);
		for (size_t i = 1; i < 10; ++i) {
			a = data[i % 2];
			printf("%f : %f\n", posit16_tof(a), posit16_tof(b));
			b = posit16_mul(a, b);
		}
		printDummy();
	}
}
void MultiplicationWorkloadPosit32(size_t NR_OPS) {
	posit32_t data[2] = { posit32_fromf(0.99999f), posit32_fromf(-1.00001) };
	posit32_t a, b = posit32_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit32_mul(a, b);
	}
	if (posit32_tof(b) == -1.0f) {
		a = posit32_fromf(0.0f), b = posit32_fromf(1.0625f);
		for (size_t i = 1; i < 10; ++i) {
			a = data[i % 2];
			printf("%f : %f\n", posit32_tof(a), posit32_tof(b));
			b = posit32_mul(a, b);
		}
		printDummy();
	}
}
void MultiplicationWorkloadPosit64(size_t NR_OPS) {
	posit64_t data[2] = { posit64_fromf(0.99999f), posit64_fromf(-1.00001) };
	posit64_t a, b = posit64_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit64_mul(a, b);
	}
	if (posit64_tof(b) == -1.0f) {
		a = posit64_fromf(0.0f), b = posit64_fromf(1.0625f);
		for (size_t i = 1; i < 10; ++i) {
			a = data[i % 2];
			printf("%f : %f\n", posit64_tof(a), posit64_tof(b));
			b = posit64_mul(a, b);
		}
		printDummy();
	}
}
// Generic set of divides for a given number system type
void DivisionWorkloadPosit8(size_t NR_OPS) {
	posit8_t data[2] = { posit8_fromf(0.99999f), posit8_fromf(1.00001) };
	if (posit8_cmp(data[0], posit8_reinterpret(0))) { data[0] = posit8_reinterpret(1); }
	posit8_t a, b = posit8_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit8_div(b, a);
	}
	if (posit8_tof(b) == -1.0f) {
		printDummy();
	}
	//printf("%f\n", posit8_tof(b));
}
void DivisionWorkloadPosit16(size_t NR_OPS) {
	posit16_t data[2] = { posit16_fromf(0.99999f), posit16_fromf(1.00001) };
	if (posit16_cmp(data[0], posit16_reinterpret(0))) { data[0] = posit16_reinterpret(1); }
	posit16_t a, b = posit16_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit16_div(b, a);
	}
	if (posit16_tof(b) == -1.0f) {
		printDummy();
	}
	//printf("%f\n", posit16_tof(b));
}
void DivisionWorkloadPosit32(size_t NR_OPS) {
	posit32_t data[2] = { posit32_fromf(0.99999f), posit32_fromf(1.00001) };
	if (posit32_cmp(data[0], posit32_reinterpret(0))) { data[0] = posit32_reinterpret(1); }
	posit32_t a, b = posit32_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit32_div(b, a);
	}
	if (posit32_tof(b) == -1.0f) {
		printDummy();
	}
	//printf("%f\n", posit32_tof(b));
}
void DivisionWorkloadPosit64(size_t NR_OPS) {
	posit64_t data[2] = { posit64_fromf(0.99999f), posit64_fromf(1.00001) };
	if (posit64_cmp(data[0], posit64_reinterpret(0))) { data[0] = posit64_reinterpret(1); }
	posit64_t a, b = posit64_fromf(1.0625f);
	for (size_t i = 1; i < NR_OPS; ++i) {
		a = data[i % 2];
		b = posit64_div(b, a);
	}
	//if (posit64_cmp(b, posit64_fromf(-1.0f))) { // for some reason this yields true
	if (posit64_tof(b) == -1.0f) {
		printDummy();
	}
	//printf("%g\n", posit64_tof(b));
}
/*
08/08/2025 Ryzen 9 8945HS
posit arithmetic operator performance
posit8_t      add/subtract         10000 per     0.001776300 sec ->   5 M ops/sec
posit16_t     add/subtract         10000 per     0.000881100 sec ->  11 M ops/sec
posit32_t     add/subtract         10000 per     0.001254000 sec ->   7 M ops/sec
posit64_t     add/subtract         10000 per     0.012010400 sec -> 832 K ops/sec
posit8_t      division             10000 per     0.002237800 sec ->   4 M ops/sec
posit16_t     division             10000 per     0.000449200 sec ->  22 M ops/sec
posit32_t     division             10000 per     0.000658500 sec ->  15 M ops/sec
posit64_t     division             10000 per     0.178718200 sec ->  55 K ops/sec
posit8_t      multiplication       10000 per     0.001264300 sec ->   7 M ops/sec
posit16_t     multiplication       10000 per     0.000376200 sec ->  26 M ops/sec
posit32_t     multiplication       10000 per     0.000972600 sec ->  10 M ops/sec
posit64_t     multiplication       10000 per     0.059722500 sec -> 167 K ops/sec
*/

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	printf("posit arithmetic operator performance\n");

	uint64_t NR_OPS = 10000;

	PerformanceRunner("posit8_t      add/subtract   ", AdditionSubtractionWorkloadPosit8, NR_OPS);
	PerformanceRunner("posit16_t     add/subtract   ", AdditionSubtractionWorkloadPosit16, NR_OPS);
	PerformanceRunner("posit32_t     add/subtract   ", AdditionSubtractionWorkloadPosit32, NR_OPS);
	PerformanceRunner("posit64_t     add/subtract   ", AdditionSubtractionWorkloadPosit64, NR_OPS);

	NR_OPS = 10000;
	PerformanceRunner("posit8_t      division       ", DivisionWorkloadPosit8, NR_OPS);
	PerformanceRunner("posit16_t     division       ", DivisionWorkloadPosit16, NR_OPS);
	PerformanceRunner("posit32_t     division       ", DivisionWorkloadPosit32, NR_OPS);
	PerformanceRunner("posit64_t     division       ", DivisionWorkloadPosit64, NR_OPS);

	// multiplication is the slowest operator
	NR_OPS = 10000;
	PerformanceRunner("posit8_t      multiplication ", MultiplicationWorkloadPosit8, NR_OPS);
	PerformanceRunner("posit16_t     multiplication ", MultiplicationWorkloadPosit16, NR_OPS);
	PerformanceRunner("posit32_t     multiplication ", MultiplicationWorkloadPosit32, NR_OPS);
	PerformanceRunner("posit64_t     multiplication ", MultiplicationWorkloadPosit64, NR_OPS);

}

// conditional compilation
#define MANUAL_TESTING 0

int main()
{
	char* tag = "posit operator performance benchmarking";
	printf("%s\n", tag);

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance();

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


/*
ETLO
Date run      : 08/08/2025 
Processor	  : AMD Ryzen 9 8945HS w/ Radeon 780M Graphics (4.00 GHz)
Installed RAM :	32.0 GB (27.8 GB usable)
System type	  : 64-bit operating system, x64-based processor
Pen and touch :	No pen or touch input is available for this display

*/

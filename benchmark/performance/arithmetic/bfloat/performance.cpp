// performance.cpp : performance benchmarking for abitrary fixed-precision bfloats
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
   The goal of the arbitrary fixed-precision bfloats is to provide a constrained 
   linear floating-point type to explore the benefits of mixed-precision algorithms.
*/

template<typename BfloatConfiguration>
void CopyWorkload(uint64_t NR_OPS) {
	using namespace std;
	using namespace sw::universal;
	BfloatConfiguration a,b,c;

	bool bFail = false;
	size_t j = 0;
	for (size_t i = 0; i < NR_OPS; ++i,++j) {
		a.set_raw_bits(i);
		b = a;
		c.set_raw_bits(j);
		if (b.sign() != c.sign()) {
			bFail = true;
		}
	}
	if (bFail) cout << "COPY FAIL\n"; // just a quick double check that all went well
}

/* 
2/28/2021
BFLOAT decode operator performance
single block representations
bfloat<8,2,uint8_t>      copy              10000000 per       0.0024806sec ->   4 Gops/sec
bfloat<16,5,uint16_t>    copy              10000000 per       0.0024583sec ->   4 Gops/sec
bfloat<32,8,uint32_t>    copy              10000000 per       0.0024478sec ->   4 Gops/sec
bfloat<64,11,uint64_t>   copy              10000000 per       0.0024541sec ->   4 Gops/sec
byte representations
bfloat<8,2,uint8_t>      copy              10000000 per       0.0024634sec ->   4 Gops/sec
bfloat<16,5,uint8_t>     copy              10000000 per       0.0490892sec -> 203 Mops/sec
bfloat<32,8,uint8_t>     copy              10000000 per        0.051731sec -> 193 Mops/sec
bfloat<64,11,uint8_t>    copy              10000000 per       0.0614276sec -> 162 Mops/sec
bfloat<128,11,uint8_t>   copy              10000000 per        0.160459sec ->  62 Mops/sec
2-byte representations
bfloat<8,2,uint16_t>     copy              10000000 per       0.0049371sec ->   2 Gops/sec
bfloat<16,5,uint16_t>    copy              10000000 per       0.0029677sec ->   3 Gops/sec
bfloat<32,8,uint16_t>    copy              10000000 per       0.0521831sec -> 191 Mops/sec
bfloat<64,11,uint16_t>   copy              10000000 per       0.0526742sec -> 189 Mops/sec
bfloat<128,11,uint16_t>  copy              10000000 per       0.0540298sec -> 185 Mops/sec
4-byte representations
bfloat<8,2,uint32_t>     copy              10000000 per       0.0097006sec ->   1 Gops/sec
bfloat<16,5,uint32_t>    copy              10000000 per       0.0844581sec -> 118 Mops/sec   <--- weird
bfloat<32,8,uint32_t>    copy              10000000 per       0.0025548sec ->   3 Gops/sec
bfloat<64,11,uint32_t>   copy              10000000 per       0.0495027sec -> 202 Mops/sec
bfloat<128,11,uint32_t>  copy              10000000 per       0.0470849sec -> 212 Mops/sec
8-byte representations
bfloat<8,2,uint64_t>     copy              10000000 per       0.0024524sec ->   4 Gops/sec
bfloat<16,5,uint64_t>    copy              10000000 per       0.0023941sec ->   4 Gops/sec
bfloat<32,8,uint64_t>    copy              10000000 per       0.0023966sec ->   4 Gops/sec
bfloat<64,11,uint64_t>   copy              10000000 per        0.002542sec ->   3 Gops/sec
bfloat<128,11,uint64_t>  copy              10000000 per           1e-07sec ->  99 Tops/sec
very large representations
bfloat<80,11,uint64_t>   copy              10000000 per               0sec ->   0  ops/sec   <--- this whole section is suspect
bfloat<96,11,uint64_t>   copy              10000000 per       0.0054762sec ->   1 Gops/sec
bfloat<128,11,uint64_t>  copy              10000000 per           1e-07sec ->  99 Tops/sec
bfloat<256,11,uint64_t>  copy              10000000 per           1e-07sec ->  99 Tops/sec
bfloat<512,11,uint64_t>  copy              10000000 per           1e-07sec ->  99 Tops/sec

The optimizer appears to be able to sometimes completel remove whole sections of code in the CopyWorkload function.
When running in debug, the assembly is identical, but in Release builts the performance is ordres of magnitude higher.
*/

/// <summary>
/// measure performance of copying numbers around
/// </summary>
void TestCopyPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT decode operator performance" << endl;

	uint64_t NR_OPS = 10000000;
	// single block representations
	cout << "single block representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      copy           ", CopyWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    copy           ", CopyWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   copy           ", CopyWorkload< sw::universal::bfloat<64, 11, uint64_t> >, NR_OPS);

	// multi-block representations
	cout << "byte representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      copy           ", CopyWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint8_t>     copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint8_t>     copy           ", CopyWorkload< sw::universal::bfloat<32, 8, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint8_t>    copy           ", CopyWorkload< sw::universal::bfloat<64, 11, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint8_t>   copy           ", CopyWorkload< sw::universal::bfloat<128, 11, uint8_t> >, NR_OPS);

	cout << "2-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint16_t>     copy           ", CopyWorkload< sw::universal::bfloat<8, 2, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint16_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint16_t>    copy           ", CopyWorkload< sw::universal::bfloat<32, 8, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint16_t>   copy           ", CopyWorkload< sw::universal::bfloat<64, 11, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint16_t>  copy           ", CopyWorkload< sw::universal::bfloat<128, 11, uint16_t> >, NR_OPS);

	cout << "4-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint32_t>     copy           ", CopyWorkload< sw::universal::bfloat<8, 2, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint32_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint32_t>    copy           ", CopyWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint32_t>   copy           ", CopyWorkload< sw::universal::bfloat<64, 11, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint32_t>  copy           ", CopyWorkload< sw::universal::bfloat<128, 11, uint32_t> >, NR_OPS);

	cout << "8-byte representations\n";
	PerformanceRunner("bfloat<8,2,uint64_t>     copy           ", CopyWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint64_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint64_t>    copy           ", CopyWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   copy           ", CopyWorkload< sw::universal::bfloat<64, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint64_t>  copy           ", CopyWorkload< sw::universal::bfloat<128, 11, uint64_t> >, NR_OPS);

	cout << "very large representations\n";
	PerformanceRunner("bfloat<80,11,uint64_t>   copy           ", CopyWorkload< sw::universal::bfloat<80, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<96,11,uint64_t>   copy           ", CopyWorkload< sw::universal::bfloat<96, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint64_t>  copy           ", CopyWorkload< sw::universal::bfloat<128, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<256,11,uint64_t>  copy           ", CopyWorkload< sw::universal::bfloat<256, 11, uint64_t> >, NR_OPS);
	PerformanceRunner("bfloat<512,11,uint64_t>  copy           ", CopyWorkload< sw::universal::bfloat<512, 11, uint64_t> >, NR_OPS);

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
	PerformanceRunner("bfloat<512,11,uint64_t>  decode         ", DecodeWorkload< sw::universal::bfloat<512, 11, uint64_t> >, NR_OPS);
}

#ifdef LATER
template<typename BfloatConfiguration>
void NormalizeWorkload(uint64_t NR_OPS) {
	using namespace std;
	using namespace sw::universal;
	constexpr size_t nbits = BfloatConfiguration::nbits;
	constexpr size_t es = BfloatConfiguration::es;
	using bt = typename BfloatConfiguration::BlockType;
	constexpr size_t fhbits = nbits - es;
	bfloat<nbits, es, bt> a;
	blocktriple<fhbits> b;  // representing significant

	bool bFail = false;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		a.set_raw_bits(i);
		a.normalize(b);
		if (a.sign() != b.sign()) {
//			cout << to_binary(a, true) << " : " << to_triple(b, true) << '\n';
			bFail = true;
		}
	}
	if (bFail) cout << "NORMALIZE FAIL\n"; // just a quick double check that all went well
}

/*
02/27/2021
BFLOAT normalize operator performance
single block representations
bfloat<8,2,uint8_t>      normalize          1000000 per       0.0008232sec ->   1 Gops/sec
bfloat<16,5,uint16_t>    normalize          1000000 per       0.0007658sec ->   1 Gops/sec
bfloat<32,8,uint32_t>    normalize           100000 per       0.0006639sec -> 150 Mops/sec
bfloat<64,11,uint64_t>   normalize           100000 per       0.0024509sec ->  40 Mops/sec
byte representations
bfloat<8,2,uint8_t>      normalize           100000 per        8.33e-05sec ->   1 Gops/sec
bfloat<16,5,uint8_t>     normalize           100000 per       0.0016208sec ->  61 Mops/sec
bfloat<32,8,uint8_t>     normalize           100000 per       0.0072102sec ->  13 Mops/sec
bfloat<64,11,uint8_t>    normalize           100000 per       0.0126001sec ->   7 Mops/sec
bfloat<128,11,uint8_t>   normalize           100000 per        0.026631sec ->   3 Mops/sec
*/

/// <summary>
/// measure performance of decode operator
/// NOTE: es is <= 11 due to limits of dynamic range of a 64-bit double
/// </summary>
void TestNormalizePerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT normalize operator performance" << endl;

	uint64_t NR_OPS = 100000;
	// single block representations
	cout << "single block representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      normalize      ", NormalizeWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS * 10);
	PerformanceRunner("bfloat<16,5,uint16_t>    normalize      ", NormalizeWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS * 10);
	PerformanceRunner("bfloat<32,8,uint32_t>    normalize      ", NormalizeWorkload< sw::universal::bfloat<32, 8, uint32_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint64_t>   normalize      ", NormalizeWorkload< sw::universal::bfloat<64, 11, uint64_t> >, NR_OPS);

	// multi-block representations
	cout << "byte representations\n";
	PerformanceRunner("bfloat<8,2,uint8_t>      normalize      ", NormalizeWorkload< sw::universal::bfloat<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::bfloat<16, 5, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<32,8,uint8_t>     normalize      ", NormalizeWorkload< sw::universal::bfloat<32, 8, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<64,11,uint8_t>    normalize      ", NormalizeWorkload< sw::universal::bfloat<64, 11, uint8_t> >, NR_OPS);
	PerformanceRunner("bfloat<128,11,uint8_t>   normalize      ", NormalizeWorkload< sw::universal::bfloat<128, 11, uint8_t> >, NR_OPS);
}
#endif // LATER

// measure performance of conversion operators
void TestConversionPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "BFLOAT Conversion operator performance" << endl;

//	uint64_t NR_OPS = 1000000;
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

	using Scalar = bfloat<16, 5, uint16_t>;
	Scalar a{ 1.0f }, b;
	b = a;
	std::cout << a << " : " << b << std::endl;

	size_t NR_OPS = 10000000;
	PerformanceRunner("bfloat<16,5,uint16_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("bfloat<16,5,uint32_t>    copy           ", CopyWorkload< sw::universal::bfloat<16, 5, uint32_t> >, NR_OPS);


	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestCopyPerformance();
	TestDecodePerformance();
#ifdef LATER
	TestNormalizePerformance();
#endif
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

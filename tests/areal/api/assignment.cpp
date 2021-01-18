// assignment.cpp: functional tests for assignments of native types to areals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#endif
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
// enabling tracing
#define TRACE_CONVERSION 0
// minimum set of include files to reflect source code dependencies
#include <universal/areal/areal.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/areal/manipulators.hpp>
#include <universal/areal/math_functions.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/areal/table.hpp>

// print the constexpr values of the areal class
template<size_t nbits, size_t es, typename bt>
void configuration() {
	sw::universal::areal<nbits, es, bt> a;
	a.debug();
}

// free function that does the same as the private copyBits function of the areal class
template<typename ArgumentBlockType, typename BlockType>
void copyBits(ArgumentBlockType v, BlockType _block[16]) {
	size_t bitsInBlock = sizeof(BlockType) * 8;
	size_t nrBlocks = 16;
	int blocksRequired = (8 * sizeof(v)) / bitsInBlock;
	int maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
	BlockType b{ 0 }; b = ~b;
	ArgumentBlockType mask = ArgumentBlockType(b);
	size_t shift = 0;
	for (int i = 0; i < maxBlockNr; ++i) {
		_block[i] = (mask & v) >> shift;
		mask <<= bitsInBlock;
		shift += bitsInBlock;
	}
}

template<size_t nbits, size_t es, typename bt = uint8_t>
int VerifyReverseSampling(const std::string& tag, bool bReportIndividualTestCases = false, bool verbose = false) {
	constexpr size_t NR_SAMPLES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	using Real = sw::universal::areal<nbits, es, bt>;
	Real ref{ 0 }; Real result{ 0 };
	for (size_t i = 0; i < NR_SAMPLES; i += 2) {
		ref.set_raw_bits(i);
		double input = double(ref);
		result = input;
		if (result != ref && !result.iszero() && !ref.iszero()) {
                           // ignore the -0 case as the compiler might optimize that sign away
			nrOfFailedTestCases++;
//			std::cout << "------->  " << i << " " << sw::universal::to_binary(input) << " " << sw::universal::to_binary(result) << std::endl;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
		}
		else {
			if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
		}
	}
	return nrOfFailedTestCases;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

/*
* e = exponent bit, m = most significant fraction bit, f = fraction bit, h = hidden bit
float       s-eee'eeee'efff'ffff'ffff'ffff'ffff'ffff (23 fraction bits, 1 hidden bit)
                                                                                 float fbits = 0x007F'FFFF  fbits   hidden+raw    0x00FF'FFFF            shift right == 24 - fbits - ubit
areal<4,1>                                     'semu   fraction = '0000'0000'0000'0000'0000'0000'0000'00h0     1    sticky mask = 0x007F'FFFF   raw+hidden 0x00FF'FFFF >> 22 to get to 0x0000'0003
areal<5,1>                                    s'emfu   fraction = '0000'0000'0000'0000'0000'0000'0000'0h10     2    sticky mask = 0x003F'FFFF   raw+hidden 0x00FF'FFFF >> 21 to get to 0x0000'0007
areal<6,1>                                   se'mffu   fraction = '0000'0000'0000'0000'0000'0000'0000'h110     3    sticky mask = 0x001F'FFFF   raw+hidden 0x00FF'FFFF >> 20 to get to 0x0000'000F
areal<7,1>                                  sem'fffu   fraction = '0000'0000'0000'0000'0000'0000'000h'1110     4    sticky mask = 0x000F'FFFF   raw+hidden 0x00FF'FFFF >> 19 to get to 0x0000'001F
areal<8,1>                                'semf'fffu   fraction = '0000'0000'0000'0000'0000'0000'00h1'1110     5    sticky mask = 0x0007'FFFF   raw+hidden 0x00FF'FFFF >> 18 to get to 0x0000'003F
areal<9,1>                               s'emff'fffu   fraction = '0000'0000'0000'0000'0000'0000'0h11'1110     6    sticky mask = 0x0003'FFFF   raw+hidden 0x00FF'FFFF >> 17 to get to 0x0000'007F
areal<10,1>                             se'mfff'fffu   fraction = '0000'0000'0000'0000'0000'0000'h111'1110     7    sticky mask = 0x0001'FFFF   raw+hidden 0x00FF'FFFF >> 16 to get to 0x0000'00FF
areal<11,1>                            sem'ffff'fffu   fraction = '0000'0000'0000'0000'0000'000h'1111'1110     8    sticky mask = 0x0000'FFFF   raw+hidden 0x00FF'FFFF >> 15 to get to 0x0000'01FF
areal<12,1>                          'semf'ffff'fffu   fraction = '0000'0000'0000'0000'0000'00h1'1111'1110     9    sticky mask = 0x0000'7FFF   raw+hidden 0x00FF'FFFF >> 14 to get to 0x0000'03FF
areal<13,1>                         s'emff'ffff'fffu   fraction = '0000'0000'0000'0000'0000'0h11'1111'1110    10    sticky mask = 0x0000'3FFF   raw+hidden 0x00FF'FFFF >> 13 to get to 0x0000'07FF
areal<14,1>                        se'mfff'ffff'fffu   fraction = '0000'0000'0000'0000'0000'h111'1111'1110    11    sticky mask = 0x0000'1FFF   raw+hidden 0x00FF'FFFF >> 12 to get to 0x0000'0FFF
areal<15,1>                       sem'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'000h'1111'1111'1110    12    sticky mask = 0x0000'0FFF   raw+hidden 0x00FF'FFFF >> 11 to get to 0x0000'1FFF
areal<16,1>                     'semf'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'00h1'1111'1111'1110    13    sticky mask = 0x0000'07FF   raw+hidden 0x00FF'FFFF >> 10 to get to 0x0000'3FFF
areal<17,1>                    s'emff'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'0h11'1111'1111'1110    14    sticky mask = 0x0000'03FF   raw+hidden 0x00FF'FFFF >>  9 to get to 0x0000'7FFF
areal<18,1>                   se'mfff'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'h111'1111'1111'1110    15    sticky mask = 0x0000'01FF   raw+hidden 0x00FF'FFFF >>  8 to get to 0x0000'FFFF
areal<19,1>                  sem'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'000h'1111'1111'1111'1110    16    sticky mask = 0x0000'00FF   raw+hidden 0x00FF'FFFF >>  7 to get to 0x0001'FFFF
areal<20,1>                'semf'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'00h1'1111'1111'1111'1110    17    sticky mask = 0x0000'007F   raw+hidden 0x00FF'FFFF >>  6 to get to 0x0003'FFFF
areal<21,1>               s'emff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'0h11'1111'1111'1111'1110    18    sticky mask = 0x0000'003F   raw+hidden 0x00FF'FFFF >>  5 to get to 0x0007'FFFF
areal<22,1>              se'mfff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'h111'1111'1111'1111'1110    19    sticky mask = 0x0000'001F   raw+hidden 0x00FF'FFFF >>  4 to get to 0x000F'FFFF
areal<23,1>             sem'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'000h'1111'1111'1111'1111'1110    20    sticky mask = 0x0000'000F   raw+hidden 0x00FF'FFFF >>  3 to get to 0x001F'FFFF
areal<24,1>           'semf'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'00h1'1111'1111'1111'1111'1110    21    sticky mask = 0x0000'0007   raw+hidden 0x00FF'FFFF >>  2 to get to 0x003F'FFFF
areal<25,1>          s'emff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0h11'1111'1111'1111'1111'1110    22    sticky mask = 0x0000'0003   raw+hidden 0x00FF'FFFF >>  1 to get to 0x007F'FFFF
areal<26,1>         se'mfff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'h111'1111'1111'1111'1111'1110    23    sticky mask = 0x0000'0001   raw+hidden 0x00FF'FFFF >>  0 to get to 0x00FF'FFFF
areal<27,1>      ' sem'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'000h'1111'1111'1111'1111'1111'1110    24    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -1 to get to 0x01FF'FFFF
areal<28,1>      'semf'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'00h1'1111'1111'1111'1111'1111'1110    25    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -2 to get to 0x03FF'FFFF
areal<29,1>     s'emff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0h11'1111'1111'1111'1111'1111'1110    26    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -3 to get to 0x07FF'FFFF
areal<30,1>    se'mfff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'h111'1111'1111'1111'1111'1111'1110    27    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -4 to get to 0x0FFF'FFFF
areal<31,1> ' sem'ffff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '000h'1111'1111'1111'1111'1111'1111'1110    28    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -5 to get to 0x1FFF'FFFF
areal<32,1> 'semf'ffff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '00h1'1111'1111'1111'1111'1111'1111'1110    29    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -6 to get to 0x3FFF'FFFF

                                                                                 float fbits = 0x007F'FFFF  fbits   hidden+raw    0x00FF'FFFF            shift right == 24 - fbits - ubit
areal<4,2>                                      seeu  (N/A)                                                    0
areal<5,2>                                    s'eemu   fraction = '0000'0000'0000'0000'0000'0000'0000'00h0     1    sticky mask = 0x003F'FFFF   raw+hidden 0x00FF'FFFF >> 22 to get to 0x0000'0003
areal<6,2>                                   se'emfu   fraction = '0000'0000'0000'0000'0000'0000'0000'0h10     2    sticky mask = 0x001F'FFFF   raw+hidden 0x00FF'FFFF >> 21 to get to 0x0000'0007
areal<7,2>                                  see'mffu   fraction = '0000'0000'0000'0000'0000'0000'0000'h110     3    sticky mask = 0x000F'FFFF   raw+hidden 0x00FF'FFFF >> 20 to get to 0x0000'000F
areal<8,2>                                'seem'fffu   fraction = '0000'0000'0000'0000'0000'0000'000h'1110     4    sticky mask = 0x0007'FFFF   raw+hidden 0x00FF'FFFF >> 19 to get to 0x0000'001F
areal<9,2>                               s'eemf'fffu   fraction = '0000'0000'0000'0000'0000'0000'00h1'1110     5    sticky mask = 0x0003'FFFF   raw+hidden 0x00FF'FFFF >> 18 to get to 0x0000'003F
areal<10,2>                             se'emff'fffu   fraction = '0000'0000'0000'0000'0000'0000'0h11'1110     6    sticky mask = 0x0001'FFFF   raw+hidden 0x00FF'FFFF >> 17 to get to 0x0000'007F
areal<11,2>                            see'mfff'fffu   fraction = '0000'0000'0000'0000'0000'0000'h111'1110     7    sticky mask = 0x0000'FFFF   raw+hidden 0x00FF'FFFF >> 16 to get to 0x0000'00FF
areal<12,2>                          'seem'ffff'fffu   fraction = '0000'0000'0000'0000'0000'000h'1111'1110     8    sticky mask = 0x0000'7FFF   raw+hidden 0x00FF'FFFF >> 15 to get to 0x0000'01FF
areal<13,2>                         s'eemf'ffff'fffu   fraction = '0000'0000'0000'0000'0000'00h1'1111'1110     9    sticky mask = 0x0000'3FFF   raw+hidden 0x00FF'FFFF >> 14 to get to 0x0000'03FF
areal<14,2>                        se'emff'ffff'fffu   fraction = '0000'0000'0000'0000'0000'0h11'1111'1110    10    sticky mask = 0x0000'1FFF   raw+hidden 0x00FF'FFFF >> 13 to get to 0x0000'07FF
areal<15,2>                       see'mfff'ffff'fffu   fraction = '0000'0000'0000'0000'0000'h111'1111'1110    11    sticky mask = 0x0000'0FFF   raw+hidden 0x00FF'FFFF >> 12 to get to 0x0000'0FFF
areal<16,2>                     'seem'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'000h'1111'1111'1110    12    sticky mask = 0x0000'07FF   raw+hidden 0x00FF'FFFF >> 11 to get to 0x0000'1FFF
areal<17,2>                    s'eemf'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'00h1'1111'1111'1110    13    sticky mask = 0x0000'03FF   raw+hidden 0x00FF'FFFF >> 10 to get to 0x0000'3FFF
areal<18,2>                   se'emff'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'0h11'1111'1111'1110    14    sticky mask = 0x0000'01FF   raw+hidden 0x00FF'FFFF >>  9 to get to 0x0000'7FFF
areal<19,2>                  see'mfff'ffff'ffff'fffu   fraction = '0000'0000'0000'0000'h111'1111'1111'1110    15    sticky mask = 0x0000'00FF   raw+hidden 0x00FF'FFFF >>  8 to get to 0x0000'FFFF
areal<20,2>                'seem'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'000h'1111'1111'1111'1110    16    sticky mask = 0x0000'007F   raw+hidden 0x00FF'FFFF >>  7 to get to 0x0001'FFFF
areal<21,2>               s'eemf'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'00h1'1111'1111'1111'1110    17    sticky mask = 0x0000'003F   raw+hidden 0x00FF'FFFF >>  6 to get to 0x0003'FFFF
areal<22,2>              se'emff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'0h11'1111'1111'1111'1110    18    sticky mask = 0x0000'001F   raw+hidden 0x00FF'FFFF >>  5 to get to 0x0007'FFFF
areal<23,2>             see'mfff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0000'h111'1111'1111'1111'1110    19    sticky mask = 0x0000'000F   raw+hidden 0x00FF'FFFF >>  4 to get to 0x000F'FFFF
areal<24,2>           'seem'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'000h'1111'1111'1111'1111'1110    20    sticky mask = 0x0000'0007   raw+hidden 0x00FF'FFFF >>  3 to get to 0x001F'FFFF
areal<25,2>          s'eemf'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'00h1'1111'1111'1111'1111'1110    21    sticky mask = 0x0000'0003   raw+hidden 0x00FF'FFFF >>  2 to get to 0x003F'FFFF
areal<26,2>         se'emff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'0h11'1111'1111'1111'1111'1110    22    sticky mask = 0x0000'0001   raw+hidden 0x00FF'FFFF >>  1 to get to 0x007F'FFFF
areal<27,2>        see'mfff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0000'h111'1111'1111'1111'1111'1110    23    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >>  0 to get to 0x00FF'FFFF
areal<28,2>      'seem'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'000h'1111'1111'1111'1111'1111'1110    24    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -1 to get to 0x01FF'FFFF
areal<29,2>     s'eemf'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'00h1'1111'1111'1111'1111'1111'1110    25    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -2 to get to 0x03FF'FFFF
areal<30,2>    se'emff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'0h11'1111'1111'1111'1111'1111'1110    26    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -3 to get to 0x07FF'FFFF
areal<31,2>   see'mfff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '0000'h111'1111'1111'1111'1111'1111'1110    27    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -4 to get to 0x0FFF'FFFF
areal<32,2> 'seem'ffff'ffff'ffff'ffff'ffff'ffff'fffu   fraction = '000h'1111'1111'1111'1111'1111'1111'1110    28    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -5 to get to 0x1FFF'FFFF

areal<4,1>  s-e-f-u         fraction = 0-0-h-0, sticky mask = 0x007F'FFFF
double      s-eee'eeee'eeee-ffff'...'ffff  (52 fraction bits, 1 hidden bit

*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "AREAL assignment: ";

	{
		using Real = sw::universal::areal<5, 1>;
		Real a = 0.5;
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;
	}
	{
		using Real = sw::universal::areal<6, 1>;
		Real a = 0.5;
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;
	}


#if MANUAL_TESTING

/*
* subnormals
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:        b00000000       0      -5             b00           b0000       0                             0       8.2x0x00r
   2:        b00000010       0      -4             b00           b0001       0                        0.0625       8.2x0x02r
   4:        b00000100       0      -3             b00           b0010       0                         0.125       8.2x0x04r
   6:        b00000110       0      -3             b00           b0011       0                        0.1875       8.2x0x06r
   8:        b00001000       0      -2             b00           b0100       0                          0.25       8.2x0x08r
  10:        b00001010       0      -2             b00           b0101       0                        0.3125       8.2x0x0Ar
  12:        b00001100       0      -2             b00           b0110       0                         0.375       8.2x0x0Cr
  14:        b00001110       0      -2             b00           b0111       0                        0.4375       8.2x0x0Er
  16:        b00010000       0      -1             b00           b1000       0                           0.5       8.2x0x10r
  18:        b00010010       0      -1             b00           b1001       0                        0.5625       8.2x0x12r
  20:        b00010100       0      -1             b00           b1010       0                         0.625       8.2x0x14r
  22:        b00010110       0      -1             b00           b1011       0                        0.6875       8.2x0x16r
  24:        b00011000       0      -1             b00           b1100       0                          0.75       8.2x0x18r
  26:        b00011010       0      -1             b00           b1101       0                        0.8125       8.2x0x1Ar
  28:        b00011100       0      -1             b00           b1110       0                         0.875       8.2x0x1Cr
  30:        b00011110       0      -1             b00           b1111       0                        0.9375       8.2x0x1Er

* normals
  60:        b00111100       0       0             b01           b1110       0                         1.875       8.2x0x3Cr
  62:        b00111110       0       0             b01           b1111       0                        1.9375       8.2x0x3Er
  64:        b01000000       0       1             b10           b0000       0                             2       8.2x0x40r
  66:        b01000010       0       1             b10           b0001       0                         2.125       8.2x0x42r
  68:        b01000100       0       1             b10           b0010       0                          2.25       8.2x0x44r

* supernormals 
 110:        b01101110       0       2             b11           b0111       0                          5.75       8.2x0x6Er
 112:        b01110000       0       2             b11           b1000       0                             6       8.2x0x70r
 114:        b01110010       0       2             b11           b1001       0                          6.25       8.2x0x72r


NEGATIVE
* subnormals
 134:        b10000110       1      -3             b00           b0011       0                       -0.1875       8.2x0x86r
 136:        b10001000       1      -2             b00           b0100       0                         -0.25       8.2x0x88r
 138:        b10001010       1      -2             b00           b0101       0                       -0.3125       8.2x0x8Ar

* normals
 188:        b10111100       1       0             b01           b1110       0                        -1.875       8.2x0xBCr
 190:        b10111110       1       0             b01           b1111       0                       -1.9375       8.2x0xBEr
 192:        b11000000       1       1             b10           b0000       0                            -2       8.2x0xC0r
 194:        b11000010       1       1             b10           b0001       0                        -2.125       8.2x0xC2r
 196:        b11000100       1       1             b10           b0010       0                         -2.25       8.2x0xC4r

* supernormals
 238:        b11101110       1       2             b11           b0111       0                         -5.75       8.2x0xEEr
 240:        b11110000       1       2             b11           b1000       0                            -6       8.2x0xF0r
 242:        b11110010       1       2             b11           b1001       0                         -6.25       8.2x0xF2r
*/


	using Real = sw::universal::areal<8, 2>;

	Real a;
	double da;


	// test sNaN
	a.setnan(NAN_TYPE_SIGNALLING);
	da = double(a);
	std::cout << to_binary(da) << " " << da << std::endl;
	a = da;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;

	// test qNaN
	a.setnan(NAN_TYPE_QUIET);
	da = double(a);
	std::cout << to_binary(da) << " " << da << std::endl;
	a = da;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;

	// test +inf
	a.setinf(false); // +inf
	da = double(a);
	std::cout << to_binary(da) << " " << da << std::endl;	
	a = da;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;

	// test -inf
	a.setinf(true); // -inf
	da = double(a);
	std::cout << to_binary(da) << " " << da << std::endl;
	a = da;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;

	std::cout << "+0.0 = " << to_binary(+0.0) << " " << 0.0 << std::endl;
	std::cout << "-0.0 = " << to_binary(-0.0) << " " << -0.0 << std::endl;

	// test 0.0
	std::cout << "Test positive 0.0\n";
	a.set_raw_bits(0x00);
	std::cout << "double(a)    = " << double(a) << std::endl;
	da = double(a);
	std::cout << "reference  a = " << a << " " << to_binary(da) << " " << da << std::endl;
	a = da;
	std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;
	if (a.iszero()) std::cout << "PASS +0 == iszero()\n"; else std::cout << "FAIL +0 != iszero()\n";

	// Testing problem: the optimizer might destroy the sign of a copy of a -0.0
	// test -0.0
	std::cout << "Test negative 0.0\n";
	a.set_raw_bits(0x80);
	std::cout << "double(a)    = " << double(a) << std::endl;
	da = double(a);
	std::cout << "reference  a = " << a << " " << to_binary(da) << " " << da << std::endl;
	a = da;
	std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << std::endl;
	if (a.iszero()) std::cout << "PASS -0 == iszero()\n"; else std::cout << "FAIL -0 != iszero()\n";

	bool bVerbose = false;
	// es = 1 encodings
	// 1 block representations
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 4, 1, uint8_t>(tag, bReportIndividualTestCases, bVerbose), "areal<4,1,uint8_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 5, 1, uint8_t>(tag, bReportIndividualTestCases, bVerbose), "areal<5,1,uint8_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 6, 1, uint8_t>(tag, bReportIndividualTestCases, bVerbose), "areal<6,1,uint8_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 7, 1, uint8_t>(tag, bReportIndividualTestCases, bVerbose), "areal<7,1,uint8_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 8, 1, uint8_t>(tag, bReportIndividualTestCases, bVerbose), "areal<8,1,uint8_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 9, 1, uint16_t>(tag, bReportIndividualTestCases, bVerbose), "areal<9,1,uint16_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<10, 1, uint16_t>(tag, bReportIndividualTestCases, bVerbose), "areal<10,1,uint16_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<12, 1, uint16_t>(tag, bReportIndividualTestCases, bVerbose), "areal<12,1,uint16_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<14, 1, uint16_t>(tag, bReportIndividualTestCases, bVerbose), "areal<14,1,uint16_t>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<16, 1, uint16_t>(tag, bReportIndividualTestCases, bVerbose), "areal<16,1,uint16_t>", "=");

	// 2 block representations
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 9, 1, uint8_t>(tag, false, bVerbose), "areal<9,1,uint8_t>", "=");
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<10, 1, uint8_t>(tag, false, bVerbose), "areal<10,1,uint8_t>", "=");
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<16, 1, uint8_t>(tag, false, bVerbose), "areal<16,1,uint8_t>", "=");



	// es = 2 encodings
	// 1 block representations
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<5, 2>(tag, bReportIndividualTestCases, bVerbose), "areal<5,2>", "=");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<6, 2>(tag, bReportIndividualTestCases, bVerbose), "areal<6,2>", "=");
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<7, 2>(tag, true, bVerbose), "areal<7,2>", "=");
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<8, 2>(tag, true, bVerbose), "areal<8,2>", "=");

	// 2 block representations
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<9, 2>(tag, bReportIndividualTestCases, bVerbose), "areal<9,2>", "=");
//	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<9, 3>(tag, bReportIndividualTestCases, bVerbose), "areal<9,3>", "=");

	
	//	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<sw::universal::areal<5, 2,  uint8_t>, float >(bReportIndividualTestCases), tag, "areal<5,2,uint8_t>");
	
#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "AREAL assignment validation" << endl;

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<4, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<4,1,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<6, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<6,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<6, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<6,2,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,3,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,3,uint8_t>");


#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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


/*
Generate table for a class sw::universal::areal<5,1,unsigned char> in TXT format
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:           b00000       0      -2              b0             b00       0                             0       5.1x0x00r
   2:           b00010       0      -1              b0             b01       0                           0.5       5.1x0x02r
   4:           b00100       0       0              b0             b10       0                             1       5.1x0x04r
   6:           b00110       0       0              b0             b11       0                           1.5       5.1x0x06r
   8:           b01000       0       1              b1             b00       0                             2       5.1x0x08r
  10:           b01010       0       1              b1             b01       0                           2.5       5.1x0x0Ar
  12:           b01100       0       1              b1             b10       0                             3       5.1x0x0Cr
  14:           b01110       0       1              b1             b11       0                           inf       5.1x0x0Er
  16:           b10000       1      -2              b0             b00       0                             0       5.1x0x10r
  18:           b10010       1      -1              b0             b01       0                          -0.5       5.1x0x12r
  20:           b10100       1       0              b0             b10       0                            -1       5.1x0x14r
  22:           b10110       1       0              b0             b11       0                          -1.5       5.1x0x16r
  24:           b11000       1       1              b1             b00       0                            -2       5.1x0x18r
  26:           b11010       1       1              b1             b01       0                          -2.5       5.1x0x1Ar
  28:           b11100       1       1              b1             b10       0                            -3       5.1x0x1Cr
  30:           b11110       1       1              b1             b11       0                          -inf       5.1x0x1Er

Generate table for a class sw::universal::areal<5,2,unsigned char> in TXT format
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:           b00000       0      -2             b00              b0       0                             0       5.2x0x00r
   2:           b00010       0      -1             b00              b1       0                           0.5       5.2x0x02r
   4:           b00100       0       0             b01              b0       0                             1       5.2x0x04r
   6:           b00110       0       0             b01              b1       0                           1.5       5.2x0x06r
   8:           b01000       0       1             b10              b0       0                             2       5.2x0x08r
  10:           b01010       0       1             b10              b1       0                             3       5.2x0x0Ar
  12:           b01100       0       2             b11              b0       0                             4       5.2x0x0Cr
  14:           b01110       0       2             b11              b1       0                           inf       5.2x0x0Er
  16:           b10000       1      -2             b00              b0       0                             0       5.2x0x10r
  18:           b10010       1      -1             b00              b1       0                          -0.5       5.2x0x12r
  20:           b10100       1       0             b01              b0       0                            -1       5.2x0x14r
  22:           b10110       1       0             b01              b1       0                          -1.5       5.2x0x16r
  24:           b11000       1       1             b10              b0       0                            -2       5.2x0x18r
  26:           b11010       1       1             b10              b1       0                            -3       5.2x0x1Ar
  28:           b11100       1       2             b11              b0       0                            -4       5.2x0x1Cr
  30:           b11110       1       2             b11              b1       0                          -inf       5.2x0x1Er

Generate table for a class sw::universal::areal<6,1,unsigned char> in TXT format
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:          b000000       0      -3              b0            b000       0                             0       6.1x0x00r
   2:          b000010       0      -2              b0            b001       0                          0.25       6.1x0x02r
   4:          b000100       0      -1              b0            b010       0                           0.5       6.1x0x04r
   6:          b000110       0      -1              b0            b011       0                          0.75       6.1x0x06r
   8:          b001000       0       0              b0            b100       0                             1       6.1x0x08r
  10:          b001010       0       0              b0            b101       0                          1.25       6.1x0x0Ar
  12:          b001100       0       0              b0            b110       0                           1.5       6.1x0x0Cr
  14:          b001110       0       0              b0            b111       0                          1.75       6.1x0x0Er
  16:          b010000       0       1              b1            b000       0                             2       6.1x0x10r
  18:          b010010       0       1              b1            b001       0                          2.25       6.1x0x12r
  20:          b010100       0       1              b1            b010       0                           2.5       6.1x0x14r
  22:          b010110       0       1              b1            b011       0                          2.75       6.1x0x16r
  24:          b011000       0       1              b1            b100       0                             3       6.1x0x18r
  26:          b011010       0       1              b1            b101       0                          3.25       6.1x0x1Ar
  28:          b011100       0       1              b1            b110       0                           3.5       6.1x0x1Cr
  30:          b011110       0       1              b1            b111       0                           inf       6.1x0x1Er
  32:          b100000       1      -3              b0            b000       0                             0       6.1x0x20r
  34:          b100010       1      -2              b0            b001       0                         -0.25       6.1x0x22r
  36:          b100100       1      -1              b0            b010       0                          -0.5       6.1x0x24r
  38:          b100110       1      -1              b0            b011       0                         -0.75       6.1x0x26r
  40:          b101000       1       0              b0            b100       0                            -1       6.1x0x28r
  42:          b101010       1       0              b0            b101       0                         -1.25       6.1x0x2Ar
  44:          b101100       1       0              b0            b110       0                          -1.5       6.1x0x2Cr
  46:          b101110       1       0              b0            b111       0                         -1.75       6.1x0x2Er
  48:          b110000       1       1              b1            b000       0                            -2       6.1x0x30r
  50:          b110010       1       1              b1            b001       0                         -2.25       6.1x0x32r
  52:          b110100       1       1              b1            b010       0                          -2.5       6.1x0x34r
  54:          b110110       1       1              b1            b011       0                         -2.75       6.1x0x36r
  56:          b111000       1       1              b1            b100       0                            -3       6.1x0x38r
  58:          b111010       1       1              b1            b101       0                         -3.25       6.1x0x3Ar
  60:          b111100       1       1              b1            b110       0                          -3.5       6.1x0x3Cr
  62:          b111110       1       1              b1            b111       0                          -inf       6.1x0x3Er
Generate table for a class sw::universal::areal<6,2,unsigned char> in TXT format
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:          b000000       0      -3             b00             b00       0                             0       6.2x0x00r
   2:          b000010       0      -2             b00             b01       0                          0.25       6.2x0x02r
   4:          b000100       0      -1             b00             b10       0                           0.5       6.2x0x04r
   6:          b000110       0      -1             b00             b11       0                          0.75       6.2x0x06r
   8:          b001000       0       0             b01             b00       0                             1       6.2x0x08r
  10:          b001010       0       0             b01             b01       0                          1.25       6.2x0x0Ar
  12:          b001100       0       0             b01             b10       0                           1.5       6.2x0x0Cr
  14:          b001110       0       0             b01             b11       0                          1.75       6.2x0x0Er
  16:          b010000       0       1             b10             b00       0                             2       6.2x0x10r
  18:          b010010       0       1             b10             b01       0                           2.5       6.2x0x12r
  20:          b010100       0       1             b10             b10       0                             3       6.2x0x14r
  22:          b010110       0       1             b10             b11       0                           3.5       6.2x0x16r
  24:          b011000       0       2             b11             b00       0                             4       6.2x0x18r
  26:          b011010       0       2             b11             b01       0                             5       6.2x0x1Ar
  28:          b011100       0       2             b11             b10       0                             6       6.2x0x1Cr
  30:          b011110       0       2             b11             b11       0                           inf       6.2x0x1Er
  32:          b100000       1      -3             b00             b00       0                             0       6.2x0x20r
  34:          b100010       1      -2             b00             b01       0                         -0.25       6.2x0x22r
  36:          b100100       1      -1             b00             b10       0                          -0.5       6.2x0x24r
  38:          b100110       1      -1             b00             b11       0                         -0.75       6.2x0x26r
  40:          b101000       1       0             b01             b00       0                            -1       6.2x0x28r
  42:          b101010       1       0             b01             b01       0                         -1.25       6.2x0x2Ar
  44:          b101100       1       0             b01             b10       0                          -1.5       6.2x0x2Cr
  46:          b101110       1       0             b01             b11       0                         -1.75       6.2x0x2Er
  48:          b110000       1       1             b10             b00       0                            -2       6.2x0x30r
  50:          b110010       1       1             b10             b01       0                          -2.5       6.2x0x32r
  52:          b110100       1       1             b10             b10       0                            -3       6.2x0x34r
  54:          b110110       1       1             b10             b11       0                          -3.5       6.2x0x36r
  56:          b111000       1       2             b11             b00       0                            -4       6.2x0x38r
  58:          b111010       1       2             b11             b01       0                            -5       6.2x0x3Ar
  60:          b111100       1       2             b11             b10       0                            -6       6.2x0x3Cr
  62:          b111110       1       2             b11             b11       0                          -inf       6.2x0x3Er

Generate table for a class sw::universal::areal<8,2,unsigned char> in TXT format
   #           Binary    sign   scale        exponent        fraction    ubit                         value      hex_format
   0:        b00000000       0      -5             b00           b0000       0                             0       8.2x0x00r
   2:        b00000010       0      -4             b00           b0001       0                        0.0625       8.2x0x02r
   4:        b00000100       0      -3             b00           b0010       0                         0.125       8.2x0x04r
   6:        b00000110       0      -3             b00           b0011       0                        0.1875       8.2x0x06r
   8:        b00001000       0      -2             b00           b0100       0                          0.25       8.2x0x08r
  10:        b00001010       0      -2             b00           b0101       0                        0.3125       8.2x0x0Ar
  12:        b00001100       0      -2             b00           b0110       0                         0.375       8.2x0x0Cr
  14:        b00001110       0      -2             b00           b0111       0                        0.4375       8.2x0x0Er
  16:        b00010000       0      -1             b00           b1000       0                           0.5       8.2x0x10r
  18:        b00010010       0      -1             b00           b1001       0                        0.5625       8.2x0x12r
  20:        b00010100       0      -1             b00           b1010       0                         0.625       8.2x0x14r
  22:        b00010110       0      -1             b00           b1011       0                        0.6875       8.2x0x16r
  24:        b00011000       0      -1             b00           b1100       0                          0.75       8.2x0x18r
  26:        b00011010       0      -1             b00           b1101       0                        0.8125       8.2x0x1Ar
  28:        b00011100       0      -1             b00           b1110       0                         0.875       8.2x0x1Cr
  30:        b00011110       0      -1             b00           b1111       0                        0.9375       8.2x0x1Er
  32:        b00100000       0       0             b01           b0000       0                             1       8.2x0x20r
  34:        b00100010       0       0             b01           b0001       0                        1.0625       8.2x0x22r
  36:        b00100100       0       0             b01           b0010       0                         1.125       8.2x0x24r
  38:        b00100110       0       0             b01           b0011       0                        1.1875       8.2x0x26r
  40:        b00101000       0       0             b01           b0100       0                          1.25       8.2x0x28r
  42:        b00101010       0       0             b01           b0101       0                        1.3125       8.2x0x2Ar
  44:        b00101100       0       0             b01           b0110       0                         1.375       8.2x0x2Cr
  46:        b00101110       0       0             b01           b0111       0                        1.4375       8.2x0x2Er
  48:        b00110000       0       0             b01           b1000       0                           1.5       8.2x0x30r
  50:        b00110010       0       0             b01           b1001       0                        1.5625       8.2x0x32r
  52:        b00110100       0       0             b01           b1010       0                         1.625       8.2x0x34r
  54:        b00110110       0       0             b01           b1011       0                        1.6875       8.2x0x36r
  56:        b00111000       0       0             b01           b1100       0                          1.75       8.2x0x38r
  58:        b00111010       0       0             b01           b1101       0                        1.8125       8.2x0x3Ar
  60:        b00111100       0       0             b01           b1110       0                         1.875       8.2x0x3Cr
  62:        b00111110       0       0             b01           b1111       0                        1.9375       8.2x0x3Er
  64:        b01000000       0       1             b10           b0000       0                             2       8.2x0x40r
  66:        b01000010       0       1             b10           b0001       0                         2.125       8.2x0x42r
  68:        b01000100       0       1             b10           b0010       0                          2.25       8.2x0x44r
  70:        b01000110       0       1             b10           b0011       0                         2.375       8.2x0x46r
  72:        b01001000       0       1             b10           b0100       0                           2.5       8.2x0x48r
  74:        b01001010       0       1             b10           b0101       0                         2.625       8.2x0x4Ar
  76:        b01001100       0       1             b10           b0110       0                          2.75       8.2x0x4Cr
  78:        b01001110       0       1             b10           b0111       0                         2.875       8.2x0x4Er
  80:        b01010000       0       1             b10           b1000       0                             3       8.2x0x50r
  82:        b01010010       0       1             b10           b1001       0                         3.125       8.2x0x52r
  84:        b01010100       0       1             b10           b1010       0                          3.25       8.2x0x54r
  86:        b01010110       0       1             b10           b1011       0                         3.375       8.2x0x56r
  88:        b01011000       0       1             b10           b1100       0                           3.5       8.2x0x58r
  90:        b01011010       0       1             b10           b1101       0                         3.625       8.2x0x5Ar
  92:        b01011100       0       1             b10           b1110       0                          3.75       8.2x0x5Cr
  94:        b01011110       0       1             b10           b1111       0                         3.875       8.2x0x5Er
  96:        b01100000       0       2             b11           b0000       0                             4       8.2x0x60r
  98:        b01100010       0       2             b11           b0001       0                          4.25       8.2x0x62r
 100:        b01100100       0       2             b11           b0010       0                           4.5       8.2x0x64r
 102:        b01100110       0       2             b11           b0011       0                          4.75       8.2x0x66r
 104:        b01101000       0       2             b11           b0100       0                             5       8.2x0x68r
 106:        b01101010       0       2             b11           b0101       0                          5.25       8.2x0x6Ar
 108:        b01101100       0       2             b11           b0110       0                           5.5       8.2x0x6Cr
 110:        b01101110       0       2             b11           b0111       0                          5.75       8.2x0x6Er
 112:        b01110000       0       2             b11           b1000       0                             6       8.2x0x70r
 114:        b01110010       0       2             b11           b1001       0                          6.25       8.2x0x72r
 116:        b01110100       0       2             b11           b1010       0                           6.5       8.2x0x74r
 118:        b01110110       0       2             b11           b1011       0                          6.75       8.2x0x76r
 120:        b01111000       0       2             b11           b1100       0                             7       8.2x0x78r
 122:        b01111010       0       2             b11           b1101       0                          7.25       8.2x0x7Ar
 124:        b01111100       0       2             b11           b1110       0                           7.5       8.2x0x7Cr
 126:        b01111110       0       2             b11           b1111       0                           inf       8.2x0x7Er
 128:        b10000000       1      -5             b00           b0000       0                             0       8.2x0x80r
 130:        b10000010       1      -4             b00           b0001       0                       -0.0625       8.2x0x82r
 132:        b10000100       1      -3             b00           b0010       0                        -0.125       8.2x0x84r
 134:        b10000110       1      -3             b00           b0011       0                       -0.1875       8.2x0x86r
 136:        b10001000       1      -2             b00           b0100       0                         -0.25       8.2x0x88r
 138:        b10001010       1      -2             b00           b0101       0                       -0.3125       8.2x0x8Ar
 140:        b10001100       1      -2             b00           b0110       0                        -0.375       8.2x0x8Cr
 142:        b10001110       1      -2             b00           b0111       0                       -0.4375       8.2x0x8Er
 144:        b10010000       1      -1             b00           b1000       0                          -0.5       8.2x0x90r
 146:        b10010010       1      -1             b00           b1001       0                       -0.5625       8.2x0x92r
 148:        b10010100       1      -1             b00           b1010       0                        -0.625       8.2x0x94r
 150:        b10010110       1      -1             b00           b1011       0                       -0.6875       8.2x0x96r
 152:        b10011000       1      -1             b00           b1100       0                         -0.75       8.2x0x98r
 154:        b10011010       1      -1             b00           b1101       0                       -0.8125       8.2x0x9Ar
 156:        b10011100       1      -1             b00           b1110       0                        -0.875       8.2x0x9Cr
 158:        b10011110       1      -1             b00           b1111       0                       -0.9375       8.2x0x9Er
 160:        b10100000       1       0             b01           b0000       0                            -1       8.2x0xA0r
 162:        b10100010       1       0             b01           b0001       0                       -1.0625       8.2x0xA2r
 164:        b10100100       1       0             b01           b0010       0                        -1.125       8.2x0xA4r
 166:        b10100110       1       0             b01           b0011       0                       -1.1875       8.2x0xA6r
 168:        b10101000       1       0             b01           b0100       0                         -1.25       8.2x0xA8r
 170:        b10101010       1       0             b01           b0101       0                       -1.3125       8.2x0xAAr
 172:        b10101100       1       0             b01           b0110       0                        -1.375       8.2x0xACr
 174:        b10101110       1       0             b01           b0111       0                       -1.4375       8.2x0xAEr
 176:        b10110000       1       0             b01           b1000       0                          -1.5       8.2x0xB0r
 178:        b10110010       1       0             b01           b1001       0                       -1.5625       8.2x0xB2r
 180:        b10110100       1       0             b01           b1010       0                        -1.625       8.2x0xB4r
 182:        b10110110       1       0             b01           b1011       0                       -1.6875       8.2x0xB6r
 184:        b10111000       1       0             b01           b1100       0                         -1.75       8.2x0xB8r
 186:        b10111010       1       0             b01           b1101       0                       -1.8125       8.2x0xBAr
 188:        b10111100       1       0             b01           b1110       0                        -1.875       8.2x0xBCr
 190:        b10111110       1       0             b01           b1111       0                       -1.9375       8.2x0xBEr
 192:        b11000000       1       1             b10           b0000       0                            -2       8.2x0xC0r
 194:        b11000010       1       1             b10           b0001       0                        -2.125       8.2x0xC2r
 196:        b11000100       1       1             b10           b0010       0                         -2.25       8.2x0xC4r
 198:        b11000110       1       1             b10           b0011       0                        -2.375       8.2x0xC6r
 200:        b11001000       1       1             b10           b0100       0                          -2.5       8.2x0xC8r
 202:        b11001010       1       1             b10           b0101       0                        -2.625       8.2x0xCAr
 204:        b11001100       1       1             b10           b0110       0                         -2.75       8.2x0xCCr
 206:        b11001110       1       1             b10           b0111       0                        -2.875       8.2x0xCEr
 208:        b11010000       1       1             b10           b1000       0                            -3       8.2x0xD0r
 210:        b11010010       1       1             b10           b1001       0                        -3.125       8.2x0xD2r
 212:        b11010100       1       1             b10           b1010       0                         -3.25       8.2x0xD4r
 214:        b11010110       1       1             b10           b1011       0                        -3.375       8.2x0xD6r
 216:        b11011000       1       1             b10           b1100       0                          -3.5       8.2x0xD8r
 218:        b11011010       1       1             b10           b1101       0                        -3.625       8.2x0xDAr
 220:        b11011100       1       1             b10           b1110       0                         -3.75       8.2x0xDCr
 222:        b11011110       1       1             b10           b1111       0                        -3.875       8.2x0xDEr
 224:        b11100000       1       2             b11           b0000       0                            -4       8.2x0xE0r
 226:        b11100010       1       2             b11           b0001       0                         -4.25       8.2x0xE2r
 228:        b11100100       1       2             b11           b0010       0                          -4.5       8.2x0xE4r
 230:        b11100110       1       2             b11           b0011       0                         -4.75       8.2x0xE6r
 232:        b11101000       1       2             b11           b0100       0                            -5       8.2x0xE8r
 234:        b11101010       1       2             b11           b0101       0                         -5.25       8.2x0xEAr
 236:        b11101100       1       2             b11           b0110       0                          -5.5       8.2x0xECr
 238:        b11101110       1       2             b11           b0111       0                         -5.75       8.2x0xEEr
 240:        b11110000       1       2             b11           b1000       0                            -6       8.2x0xF0r
 242:        b11110010       1       2             b11           b1001       0                         -6.25       8.2x0xF2r
 244:        b11110100       1       2             b11           b1010       0                          -6.5       8.2x0xF4r
 246:        b11110110       1       2             b11           b1011       0                         -6.75       8.2x0xF6r
 248:        b11111000       1       2             b11           b1100       0                            -7       8.2x0xF8r
 250:        b11111010       1       2             b11           b1101       0                         -7.25       8.2x0xFAr
 252:        b11111100       1       2             b11           b1110       0                          -7.5       8.2x0xFCr
 254:        b11111110       1       2             b11           b1111       0                          -inf       8.2x0xFEr
 */
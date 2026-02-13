// raw_bit_patterns.cpp : generate regime patterns and showcase the raw bit pattern set API of the posit.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

template<unsigned nbits, unsigned es>
void EnumerateRegimePatterns() {
	sw::universal::posit<nbits, es> p;
	uint64_t raw = 0;

	std::cout << "posit<" << nbits << ", " << es << ">" << std::endl;

	std::cout << std::setprecision(34);
	// start with NaR
	raw = uint64_t(1) << (nbits - 1);
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << std::endl;
	// move to maxpos
	raw = (uint64_t(1) << nbits) - 1 - raw;  // flip all bits
	p.setbits(raw);		std::cout << sw::universal::to_binary(p) << "      " << p << std::endl;
	// enumerate to 1
	for (int i = 0; i < int(nbits) - 2; i++) {
		raw &= ~(uint64_t(1) << i);  // clear bit i
		p.setbits(raw);		std::cout << sw::universal::to_binary(p) << "      " << p << std::endl;
	}

	// enumerate from 1.0 to minpos and finally 0
	for (int i = int(nbits) - 3; i >= 0; --i) {
		raw &= ~(uint64_t(1) << (i + 1));  // clear bit i+1
		raw |= (uint64_t(1) << i);         // set bit i
		p.setbits(raw);		std::cout << sw::universal::to_binary(p) << "      " << p << std::endl;
	}
	// and the last pattern, encoding 0
	raw &= ~uint64_t(1);  // clear bit 0
	p.setbits(raw);		std::cout << sw::universal::to_binary(p) << "      " << p << std::endl;
}

int main() 
try {
	// generate regime patterns
	EnumerateRegimePatterns<4, 0>();
	EnumerateRegimePatterns<8, 0>();
	EnumerateRegimePatterns<16, 1>();
	EnumerateRegimePatterns<24, 1>();
	EnumerateRegimePatterns<32, 2>();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

/*
        OUTPUT
		1000000000000000      NaR
		0111111111111111      72057594037927936
		0111111111111110      4503599627370496
		0111111111111100      281474976710656
		0111111111111000      17592186044416
		0111111111110000      1099511627776
		0111111111100000      68719476736
		0111111111000000      4294967296
		0111111110000000      268435456
		0111111100000000      16777216
		0111111000000000      1048576
		0111110000000000      65536
		0111100000000000      4096
		0111000000000000      256
		0110000000000000      16
		0100000000000000      1
		0010000000000000      0.0625
		0001000000000000      0.00390625
		0000100000000000      0.000244140625
		0000010000000000      1.52587890625e-05
		0000001000000000      9.5367431640625e-07
		0000000100000000      5.9604644775390625e-08
		0000000010000000      3.7252902984619140625e-09
		0000000001000000      2.3283064365386962890625e-10
		0000000000100000      1.4551915228366851806640625e-11
		0000000000010000      9.094947017729282379150390625e-13
		0000000000001000      5.684341886080801486968994140625e-14
		0000000000000100      3.552713678800500929355621337890625e-15
		0000000000000010      2.220446049250313080847263336181641e-16
		0000000000000001      1.387778780781445675529539585113525e-17
		0000000000000000      0
*/
void ManualPatternSet() {
	constexpr unsigned nbits = 16;
	constexpr unsigned es = 2;
	sw::universal::posit<nbits, es> p;
	uint64_t raw = 0;

	// 1152921504606846976
	// positive regime infinity - 1

	std::cout << std::setprecision(34);
	raw = uint64_t(1) << 15;				// NaR (Not a Real)
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = (uint64_t(1) << nbits) - 1 - raw;	// flip: 72057594037927936
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~uint64_t(1);					// 4503599627370496
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 1);			// 281474976710656
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 2);			// 17592186044416
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 3);			// 1099511627776
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 4);			// 68719476736
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 5);			// 4294967296
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 6);			// 268435456
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 7);			// 16777216
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 8);			// 1048576
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 9);			// 65536
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 10);			// 4096
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 11);			// 256
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 12);			// 16
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw &= ~(uint64_t(1) << 13);			// 1
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';

	// positive fractional regime 1 - 0
	raw = uint64_t(1) << 13;	// 1
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 12;	// 1/16
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 11;	// 1/256
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 10;	// 1/4096
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 9;	// 1/65536
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 8;	// 1/1048576
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 7;	// 1/16777216
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 6;	// 1/268435456
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 5;	// 1/4294967296
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 4;	// 1/68719476736
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 3;	// 1/1099511627776
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 2;	// 1/17592186044416
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = uint64_t(1) << 1;	// 1/281474976710656
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << '\n';
	raw = 0;					// 0
	p.setbits(raw); 	std::cout << sw::universal::to_binary(p) << "      " << p << " 0\n";
}

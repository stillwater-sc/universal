// raw_bit_patterns.cpp : generate regime patterns and showcase the raw bit pattern set API of the posit.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/posit1/posit1.hpp>

template<unsigned nbits, unsigned es>
void EnumerateRegimePatterns() {
	sw::universal::posit<nbits, es> p;
	sw::universal::bitblock<nbits> raw;

	std::cout << "posit<" << nbits << ", " << es << ">" << std::endl;

	std::cout << std::setprecision(34);
	// start with NaR
	raw.reset();
	raw[nbits - 1] = true;
	p.setBitblock(raw); 	std::cout << raw << "      " << p << std::endl;
	// move to maxpos
	raw.flip();		
	p.setBitblock(raw);		std::cout << raw << "      " << p << std::endl;
	// enumerate to 1
	for (int i = 0; i < int(nbits) - 2; i++) {
		raw[i] = false;
		p.setBitblock(raw);		std::cout << raw << "      " << p << std::endl;
	}

	// enumerate to 0from 1.0 to minpos and finally 0
	for (int i = int(nbits) - 3; i >= 0; --i) {
		raw[i + 1] = false;
		raw[i] = true;
		p.setBitblock(raw);		std::cout << raw << "      " << p << std::endl;
	}
	// and the last pattern, encoding 0
	raw[0] = false;
	p.setBitblock(raw);		std::cout << raw << "      " << p << std::endl;
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
	sw::universal::bitblock<nbits> raw;

	// 1152921504606846976
	raw.reset();
	// positive regime infinity - 1

	std::cout << std::setprecision(34);
	raw[15] = 1;							// NaR (Not a Real)
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.flip();								// 72057594037927936			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[0] = false;							// 4503599627370496			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[1] = false;							// 281474976710656			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[2] = false;							// 17592186044416			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[3] = false;							// 1099511627776			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[4] = false;							// 68719476736			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[5] = false;							// 4294967296			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[6] = false;							// 268435456			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[7] = false;							// 16777216			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[8] = false;							// 1048576			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[9] = false;							// 65536			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[10] = false;						// 4096			
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[11] = false;						// 256
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[12] = false;						// 16
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[13] = false;						// 1
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';

	raw.reset();
	// positive fractional regime 1 - 0
	raw[13] = true;			// 1
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[12] = true;			// 1/16
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[11] = true;			// 1/256
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[10] = true;			// 1/4096
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[9] = true;			// 1/65536
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[8] = true;			// 1/1048576
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[7] = true;			// 1/16777216
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[6] = true;			// 1/268435456
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[5] = true;			// 1/4294967296
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[4] = true;			// 1/68719476736
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[3] = true;			// 1/1099511627776
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[2] = true;			// 1/17592186044416
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw.reset();
	raw[1] = true;			// 1/281474976710656
	p.setBitblock(raw); 	std::cout << raw << "      " << p << '\n';
	raw[1] = false;			// 0
	p.setBitblock(raw); 	std::cout << raw << "      " << p << " 0\n";
}

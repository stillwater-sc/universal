// lookup_arithmetic.cpp: generate tables for small posit lookup arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <universal/number/posit/posit.hpp>

enum BINARY_ARITHMETIC_OPERATOR {
	ADD = 0,
	SUB = 1,
	MUL = 2,
	DIV = 3
};

enum BINARY_LOGIC_OPERATOR {
	LT  = 0,
	LTE = 1,
	GT  = 2,
	GTE = 3
};

enum UNARY_ARITHMETIC_OPERATOR {
	RECIPROCAL	= 4,
	SQRT		= 5
};

// generate a look-up table for arithmetic
template<size_t nbits, size_t es>
void GenerateLookupTable(BINARY_ARITHMETIC_OPERATOR op) {
	constexpr size_t nr_of_posits = (1 << nbits);
	sw::universal::posit<nbits, es> pa, pb, presult;
	for (size_t i = 0; i < nr_of_posits; i++) {
		pa.setbits(i);
		for (size_t j = 0; j < nr_of_posits; j++) {
			pb.setbits(j);
			switch (op) {
			case ADD:
				presult = pa + pb;
				break;
			case SUB:
				presult = pa - pb;
				break;
			case MUL:
				presult = pa * pb;
				break;
			case DIV:
				presult = pa / pb;
				break;
			}

			//unsigned long long base = pa.get().to_ullong() << nbits | pb.get().to_ullong();
			//std::cout << std::hex << base << " " << presult.get() << std::endl;
			std::cout << presult.get().to_ulong() << ",";
		}
		std::cout << std::endl;
	}
}

// generate a look-up table for logic operator
template<size_t nbits, size_t es>
void GenerateLookupTable(BINARY_LOGIC_OPERATOR op) {
	constexpr size_t nr_of_posits = (1 << nbits);
	sw::universal::posit<nbits, es> pa, pb;
	bool result;
	for (size_t i = 0; i < nr_of_posits; i++) {
		pa.setbits(i);
		for (size_t j = 0; j < nr_of_posits; j++) {
			pb.setbits(j);
			switch (op) {
			case LT:
				result = pa < pb;
				break;
			case LTE:
				result = pa <= pb;
				break;
			case GT:
				result = pa > pb;
				break;
			case GTE:
				result = pa >= pb;
				break;
			}

			//unsigned long long base = pa.get().to_ullong() << nbits | pb.get().to_ullong();
			//std::cout << std::hex << base << " " << presult.get() << std::endl;
			std::cout << result << ",";
		}
		std::cout << std::endl;
	}
}

// generate a look-up table for uniary operators
template<size_t nbits, size_t es>
void GenerateLookupTable(UNARY_ARITHMETIC_OPERATOR op) {
	constexpr size_t nr_of_posits = (1 << nbits);
	sw::universal::posit<nbits, es> pa, presult;

	std::cout << std::hex;
	for (size_t i = 0; i < nr_of_posits; i += 8) {
		for (int j = 0; j < 8; ++j) {
			size_t index = i + j;
			//std::cout << "index[" << index << "]";
			pa.setbits(index);
			switch (op) {
			case RECIPROCAL:
				presult = 1.0 / pa;
				std::cout << "0x" << std::hex << presult.get().to_ulong() << ",";
				break;
			case SQRT:
				if (pa.ispos() || pa.iszero()) {
					presult = sw::universal::sqrt(pa);
					std::cout << "0x" << std::hex << presult.get().to_ulong() << ",";
				}
				break;
			}

			//std::cout << std::hex << base << " " << presult.get() << std::endl;
		}
		std::cout << std::dec << std::endl;
	}
}

namespace sw {
	namespace spec {

		template<size_t nbits, size_t es>
		class posit {
		public:
			posit() { bits = 0;	}
			posit(const posit&) = default;
			posit(posit&&) = default;
			posit& operator=(const posit&) = default;
			posit& operator=(posit&&) = default;

			posit(int initial_value) { bits = initial_value & 0xff;	}

			posit& operator+=(const posit& b) {
				uint16_t index = (bits << 8) & b.bits;
				std::cout << index << std::endl;
			}
		private:
			uint8_t bits;
		};

		constexpr uint8_t lookup[1024] = {
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
			1,2,3,4,5,6,7,8,8,10,10,12,12,13,14,15,16,17,18,19,20,22,22,24,25,26,27,28,29,30,31,0,
			2,3,4,5,6,7,8,8,9,10,11,12,12,13,14,15,16,17,18,19,21,22,23,24,26,27,28,29,30,31,0,1,
			3,4,5,6,7,8,8,9,10,10,12,12,12,13,14,15,16,17,18,19,22,22,24,25,27,28,29,30,31,0,1,2,
			4,5,6,7,8,8,9,10,10,11,12,12,12,14,14,15,16,17,18,20,22,23,24,26,28,29,30,31,0,1,2,3,
			5,6,7,8,8,9,10,10,10,12,12,12,13,14,14,15,16,17,19,20,22,24,25,27,29,30,31,0,1,2,3,4,
			6,7,8,8,9,10,10,10,11,12,12,12,13,14,14,15,16,17,19,20,23,24,26,28,30,31,0,1,2,3,4,5,
			7,8,8,9,10,10,10,11,12,12,12,13,13,14,14,15,16,17,19,20,24,25,27,29,31,0,1,2,3,4,5,6,
			8,8,9,10,10,10,11,12,12,12,12,13,13,14,14,15,16,17,19,20,24,26,28,30,0,1,2,3,4,5,6,7,
			9,10,10,10,11,12,12,12,12,12,13,13,13,14,14,15,16,17,19,21,26,28,30,0,2,3,4,5,6,7,8,8,
			10,10,11,12,12,12,12,12,12,13,13,13,14,14,14,15,16,17,20,22,28,30,0,2,4,5,6,7,8,8,9,10,
			11,12,12,12,12,12,12,13,13,13,13,14,14,14,14,15,16,17,20,23,30,0,2,4,6,7,8,8,9,10,10,10,
			12,12,12,12,12,13,13,13,13,13,14,14,14,14,14,15,16,18,20,24,0,2,4,6,8,8,9,10,10,10,11,12,
			13,13,13,13,14,14,14,14,14,14,14,14,14,14,15,15,16,18,24,0,8,9,10,11,12,12,12,12,12,13,13,13,
			14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,16,18,0,8,12,12,12,13,13,13,13,13,14,14,14,14,
			15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,0,14,14,14,15,15,15,15,15,15,15,15,15,15,15,
			16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
			17,17,17,17,17,17,17,17,17,17,17,17,18,18,18,0,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
			18,18,18,18,18,19,19,19,19,19,20,20,20,24,0,14,16,17,17,17,18,18,18,18,18,18,18,18,18,18,18,18,
			19,19,19,19,20,20,20,20,20,21,22,23,24,0,8,14,16,17,17,18,18,18,18,18,18,18,18,18,18,19,19,19,
			20,20,21,22,22,22,23,24,24,26,28,30,0,8,12,14,16,17,18,18,18,18,18,19,19,19,19,19,20,20,20,20,
			21,22,22,22,23,24,24,25,26,28,30,0,2,9,12,15,16,17,18,18,18,18,19,19,19,19,20,20,20,20,20,20,
			22,22,23,24,24,25,26,27,28,30,0,2,4,10,12,15,16,17,18,18,18,19,19,19,20,20,20,20,20,20,21,22,
			23,24,24,25,26,27,28,29,30,0,2,4,6,11,13,15,16,17,18,18,19,19,19,20,20,20,20,20,21,22,22,22,
			24,25,26,27,28,29,30,31,0,2,4,6,8,12,13,15,16,17,18,18,19,19,20,20,20,20,21,22,22,22,23,24,
			25,26,27,28,29,30,31,0,1,3,5,7,8,12,13,15,16,17,18,18,19,19,20,20,20,21,22,22,22,23,24,24,
			26,27,28,29,30,31,0,1,2,4,6,8,9,12,13,15,16,17,18,18,19,20,20,20,21,22,22,22,23,24,24,25,
			27,28,29,30,31,0,1,2,3,5,7,8,10,12,13,15,16,17,18,18,19,20,20,20,22,22,22,23,24,24,25,26,
			28,29,30,31,0,1,2,3,4,6,8,9,10,12,14,15,16,17,18,18,20,20,20,21,22,22,23,24,24,25,26,27,
			29,30,31,0,1,2,3,4,5,7,8,10,10,13,14,15,16,17,18,19,20,20,20,22,22,23,24,24,25,26,27,28,
			30,31,0,1,2,3,4,5,6,8,9,10,11,13,14,15,16,17,18,19,20,20,21,22,23,24,24,25,26,27,28,29,
			31,0,1,2,3,4,5,6,7,8,10,10,12,13,14,15,16,17,18,19,20,20,22,22,24,24,25,26,27,28,29,30,
		};

		template<>
		class posit<5,0> {
		public:
			posit() { _bits = 0; }
			posit(const posit&) = default;
			posit(posit&&) = default;
			posit& operator=(const posit&) = default;
			posit& operator=(posit&&) = default;

			posit(int initial_value) { _bits = uint8_t(initial_value & 0xff); }

			posit<5, 0>& setbits(uint64_t value) { _bits = uint8_t(value & 0xff); return *this;  }
			posit<5,0>& operator+=(const posit& b) {
				uint16_t index = (_bits << 5) | b._bits;
				_bits = lookup[index];
				return *this;
			}

			sw::universal::bitblock<5> get() const { sw::universal::bitblock<5> bb; bb = int(_bits); return bb; }
		private:
			uint8_t _bits;

			// I/O operators
			friend std::ostream& operator<< (std::ostream& ostr, const posit<5, 0>& p);
			friend std::istream& operator>> (std::istream& istr, posit<5, 0>& p);

			// posit - posit logic functions
			friend bool operator==(const posit<5, 0>& lhs, const posit<5, 0>& rhs);
			friend bool operator!=(const posit<5, 0>& lhs, const posit<5, 0>& rhs);
			friend bool operator< (const posit<5, 0>& lhs, const posit<5, 0>& rhs);
			friend bool operator> (const posit<5, 0>& lhs, const posit<5, 0>& rhs);
			friend bool operator<=(const posit<5, 0>& lhs, const posit<5, 0>& rhs);
			friend bool operator>=(const posit<5, 0>& lhs, const posit<5, 0>& rhs);

		};

		// posit I/O operators
		inline std::ostream& operator<<(std::ostream& ostr, const posit<5, 0>& p) {
			return ostr << p.get();
		}
		// posit - posit binary logic operators
		inline bool operator==(const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return lhs._bits == rhs._bits;
		}
		inline bool operator!=(const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return !operator==(lhs, rhs);
		}
		inline bool operator< (const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return lhs._bits < rhs._bits;
		}
		inline bool operator> (const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return operator< (rhs, lhs);
		}
		inline bool operator<=(const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return operator< (lhs, rhs) || operator==(lhs, rhs);
		}
		inline bool operator>=(const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			return !operator< (lhs, rhs);
		}

		inline posit<5, 0> operator+(const posit<5, 0>& lhs, const posit<5, 0>& rhs) {
			posit<5, 0> sum = lhs;
			sum += rhs;
			return sum;
		}


	}
}

int Validate5_0_Lookup() {
	constexpr size_t nbits = 5;
	constexpr size_t es = 0;
	constexpr size_t nr_of_posits = 32;
	int nrOfFailures = 0;

	sw::universal::posit<nbits, es> pa, pb, psum;
	sw::spec::posit<nbits, es> sa, sb, ssum;
	for (size_t i = 0; i < nr_of_posits; i++) {
		pa.setbits(i);
		sa.setbits(i);
		for (size_t j = 0; j < nr_of_posits; j++) {
			pb.setbits(j);
			sb.setbits(j);
			psum = pa + pb;
			ssum = sa + sb;

			if (psum.get().to_ulong() != ssum.get().to_ulong()) {
				std::cerr << "failing equivalence test: " << psum << " != " << ssum << std::endl;
				nrOfFailures++;
			}
		}
	}
	return nrOfFailures;
}

static constexpr int NR_TEST_CASES = 1000000;

// measure performance of arithmetic addition
template<size_t nbits, size_t es>
int MeasureAdditionPerformance(int &positives, int &negatives) {
	sw::spec::posit<nbits, es> pa(1), pb, psum;

	positives = 0; negatives = 0;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pb.setbits(i);
		psum = pa + pb;
		psum >= 0 ? positives++ : negatives++;
	}
	return positives + negatives;
}

// receive a float and print the components of a double representation
int main()
try {
	using namespace sw::universal;
	using namespace std::chrono;
	int positives, negatives;

	steady_clock::time_point begin, end;
	duration<double> time_span;
	double elapsed;

#ifdef NOW
	//Validate5_0_Lookup();
	cout << "constexpr uint8_t posit_3_0_addition_lookup[64] = {\n";
	GenerateLookupTable<3, 0>(ADD);
	cout << "};\n";
	cout << "constexpr uint8_t posit_3_0_subtraction_lookup[64] = {\n";
	GenerateLookupTable<3, 0>(SUB);
	cout << "};\n";
	cout << "constexpr uint8_t posit_3_0_multiplication_lookup[64] = {\n";
	GenerateLookupTable<3, 0>(MUL);
	cout << "};\n";
	cout << "constexpr uint8_t posit_3_0_division_lookup[64] = {\n";
	GenerateLookupTable<3, 0>(DIV);
	cout << "};\n";
	cout << "constexpr bool posit_3_0_less_than_lookup[64] = {\n";
	GenerateLookupTable<3, 0>(LT);
	cout << "};\n";

	cout << "constexpr uint8_t posit_4_0_addition_lookup[256] = {\n";
	GenerateLookupTable<4, 0>(ADD);
	cout << "};\n";
	cout << "constexpr uint8_t posit_4_0_subtraction_lookup[256] = {\n";
	GenerateLookupTable<4, 0>(SUB);
	cout << "};\n";
	cout << "constexpr uint8_t posit_4_0_multiplication_lookup[256] = {\n";
	GenerateLookupTable<4, 0>(MUL);
	cout << "};\n";
	cout << "constexpr uint8_t posit_4_0_division_lookup[256] = {\n";
	GenerateLookupTable<4, 0>(DIV);
	cout << "};\n";
#endif

	begin = steady_clock::now();
	MeasureAdditionPerformance<5, 0>(positives, negatives);
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);
	elapsed = time_span.count();
	float pops = float((positives + negatives) / elapsed);
	std::cout << "Performance = " << pops << "POPS\n";
	std::cout << elapsed << '\n';
	std::cout << positives << " " << negatives << '\n';

	GenerateLookupTable<8, 1>(SQRT);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

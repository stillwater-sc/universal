// lookup_arithmetic.cpp: generate tables for small posit lookup arithmetic
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <chrono>
#include <posit>

// generate a look-up table for addition
template<size_t nbits, size_t es>
void GenerateAddLookupTable() {
	constexpr size_t nr_of_posits = (1 << nbits);
	sw::unum::posit<nbits, es> pa, pb, psum;
	unsigned long long mask = nr_of_posits - 1;
	unsigned long long base;
	for (size_t i = 0; i < nr_of_posits; i++) {
		pa.set_raw_bits(i);
		for (size_t j = 0; j < nr_of_posits; j++) {
			pb.set_raw_bits(j);
			psum = pa + pb;
			base = pa.get().to_ullong() << nbits | pb.get().to_ullong();
			//std::cout << std::hex << base << " " << psum.get() << std::endl;
			std::cout << psum.get().to_ulong() << ",";
		}
		std::cout << std::endl;
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

			posit<5, 0>& set_raw_bits(uint64_t value) { _bits = uint8_t(value & 0xff); return *this;  }
			posit<5,0>& operator+=(const posit& b) {
				uint16_t index = (_bits << 5) | b._bits;
				_bits = lookup[index];
				return *this;
			}

			sw::unum::bitblock<5> get() const { sw::unum::bitblock<5> bb; bb = int(_bits); return bb; }
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

	sw::unum::posit<5, 0> pa, pb, psum;
	sw::spec::posit<5, 0> sa, sb, ssum;
	for (size_t i = 0; i < nr_of_posits; i++) {
		pa.set_raw_bits(i);
		sa.set_raw_bits(i);
		for (size_t j = 0; j < nr_of_posits; j++) {
			pb.set_raw_bits(j);
			sb.set_raw_bits(j);
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
		pb.set_raw_bits(i);
		psum = pa + pb;
		psum >= 0 ? positives++ : negatives++;
	}
	return positives + negatives;
}

// receive a float and print the components of a double representation
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace std::chrono;
	int positives, negatives;


	steady_clock::time_point begin, end;
	duration<double> time_span;
	double elapsed;

	//Validate5_0_Lookup();
	//GenerateAddLookupTable<5, 0>();

	begin = steady_clock::now();
	MeasureAdditionPerformance<5, 0>(positives, negatives);
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);
	elapsed = time_span.count();
	float pops = float((positives + negatives) / elapsed);
	cout << "Performance = " << pops << "POPS" << std::endl;
	cout << elapsed << endl;
	cout << positives << " " << negatives << endl;


	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

//  performance_runner.cpp : functions to aid in performance testing and reporting
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

namespace sw::universal {

	// Generic workload for testing construction/destruction performance
	template<typename Scalar>
	void ConstructionPerformanceWorkload(uint64_t NR_OPS) {
		int positives{ 0 }, negatives{ 0 };
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			Scalar a; // don't initialize with i as that is a conversion operation
			a.setbits(i);
			if (a.sign()) ++positives; else ++negatives;
		}
		if (positives == negatives) std::cout << "positives and negatives are identical (unlikely event to select)\n";
	}

	// Generic workload for testing shift operations on a given number system type that supports operator>> and operator<<
	template<typename Scalar>
	void ShiftPerformanceWorkload(uint64_t NR_OPS) {
		Scalar a;
		a.setbits(0xFFFFFFFFFFFFFFFFull);
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			a >>= 13;
			a <<= 37;
		}
	}

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(uint64_t NR_OPS) {
		Scalar a, b, c, d;
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			a = c - b;
		}
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(uint64_t NR_OPS) {
		Scalar a, b, c, d;
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(uint64_t NR_OPS) {
		Scalar a, b, c, d;
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c = a / b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}

	// Generic set of remainder calculations for a given number system type that supports the % operator
	template<typename Scalar>
	void RemainderWorkload(uint64_t NR_OPS) {
		Scalar a, b, c, d;
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c = a % b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}

	// convert a floating point value to a power-of-ten string
	template<typename Ty>
	std::string toPowerOfTen(Ty value) {
		const char* scales[] = { " ", "K", "M", "G", "T", "P", "E", "Z" };
		Ty lower_bound = Ty(1);
		Ty scale_factor = 1.0;
		int integer_value = 0;
		size_t scale = 0;
		for (size_t i = 0; i < sizeof(scales); ++i) {
			if (value > lower_bound && value < 1000 * lower_bound) {
				integer_value = static_cast<int>(value / scale_factor);
				scale = i;
				break;
			}
			lower_bound *= 1000;
			scale_factor *= 1000.0;
		}
		std::stringstream ss;
		ss << std::setw(3) << std::right << integer_value << ' ' << scales[scale];
		return ss.str();
	}

	// generic test runner, takes a function that enumerates an operator NR_OPS time, and measures elapsed time
	void PerformanceRunner(const std::string& tag, void (f)(uint64_t), uint64_t NR_OPS) {
		using namespace std;
		using namespace std::chrono;

		steady_clock::time_point begin = steady_clock::now();
		f(NR_OPS);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();

		cout << tag << ' ' << setw(10) << NR_OPS << " per " << setw(15) << elapsed_time << "sec -> " << toPowerOfTen(double(NR_OPS) / elapsed_time) << "ops/sec" << endl;
	}

} // namespace sw::universal

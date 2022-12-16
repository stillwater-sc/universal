// scale_tracking.cpp: tracking scales of a computation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <random>
// #define FIXPNT_SCALE_TRACKING
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/utility/scale_tracker.hpp>

void GenerateRandomScales(int lowerbound = -8, int upperbound = 7)
{
	using namespace sw::universal;

	scaleTracker s(lowerbound, upperbound);

	// Use random_device to generate a seed for Mersenne twister engine.
	std::random_device rd{};
	// Use Mersenne twister engine to generate pseudo-random numbers.
	std::mt19937 engine{ rd() };
	// "Filter" MT engine's output to generate pseudo-random double values,
	// **uniformly distributed** on the closed interval [lowerbound, upperbound].
	// (Note that the range is [inclusive, inclusive].)
	std::uniform_int_distribution<int> dist{ lowerbound - 1, upperbound + 1 };

	// generate and insert random scales
	for (size_t i = 0; i < (1ull << 10); ++i) {
		int scale = dist(engine);
		s.incr(scale);
	}
	s.report(std::cout);
	s.clear();
}

#ifdef FIXPNT_SCALE_TRACKING

namespace sw::universal {
	template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
	class FixpntScaleTracker : public scaleTracker {
	public:
		FixpntScaleTracker(FixpntScaleTracker& fst) = delete; // can't be clonable
		void operator=(const FixpntScaleTracker& fst) = delete; // can't be assignable

		static FixpntScaleTracker* instance() {
			if (pInstance == nullptr) {
				constexpr fixpnt<nbits, rbits, arithmetic, bt> a(SpecificValue::minpos), b(SpecificValue::maxpos);
				int lb = scale(a);
				int ub = scale(b);
				pInstance = new FixpntScaleTracker(lb, ub);
			}
			return pInstance;
		}
	public:
		FixpntScaleTracker(int minScale, int maxScale) : scaleTracker(minScale, maxScale) {}
		static FixpntScaleTracker* pInstance;
	};

	//template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
	//static FixpntScaleTracker<nbits, rbits, arithmetic, bt>* pInstance = nullptr;

	static FixpntScaleTracker<16, 8, sw::universal::Modulo, uint8_t>* pInstance = nullptr;
}

#endif


int main()
try {
	using namespace sw::universal;

	// GenerateRandomScales();
	
	{
		fixpnt<16, 8> a(SpecificValue::minpos), b(SpecificValue::maxpos), c;
		int lowerbound = scale(a);
		int upperbound = scale(b);
		c.setbits(0x8000);
		int bla = scale(c);
		std::cout << "minScale = " << lowerbound << " maxScale = " << upperbound << " maxnegScale = " << bla << '\n';
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		std::cout << to_binary(c) << " : " << c << '\n';
	}

	{
		fixpnt<16, 8> a(SpecificValue::minpos), b(SpecificValue::maxneg), one(1.0f);
		int lb = scale(a);
		int ub = scale(b);
		scaleTracker s(lb, ub);
		s.incr(scale(a));
		int v = scale(a);
		std::cout << to_binary(a) << " : " << a << " scale = " << v << '\n';
		a = one / a;
		s.incr(scale(a));
		v = scale(a);
		std::cout << to_binary(a) << " : " << a << " scale = " << v << '\n';

		a.setbits(0x0002);
		for (size_t i = 0; i < 10; ++i) {
			v = scale(a);
			std::cout << to_binary(a) << " : " << a << " scale = " << v << '\n';
			s.incr(v);
			a = one / a;
		}
		s.report(std::cout);
		s.clear();
	}

#ifdef FIXPNT_SCALE_TRACKING
	{
		constexpr size_t nbits = 16;
		constexpr size_t rbits =  8;
		fixpnt<nbits, rbits, Modulo, uint8_t> a;
		FixpntScaleTracker<nbits, rbits, Modulo, uint8_t>* fst = FixpntScaleTracker<nbits, rbits, Modulo, uint8_t>::instance();
		fst->report(std::cout);
	}
#endif

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

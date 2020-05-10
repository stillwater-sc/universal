// lerp.cpp: evaluation of linear interpolation function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <random>  // only valid for native types
#include <array>
#include <algorithm>
#include <universal/posit/posit>
#include <universal/functions/lerp.hpp>

template <typename Rand>
class Seed {
	class seeder {
		std::array < std::random_device::result_type, Rand::state_size > rand_data;
	public:
		seeder() {
			std::random_device rd;
			std::generate(rand_data.begin(), rand_data.end(), std::ref(rd));
		}

		typename std::array < std::random_device::result_type, Rand::state_size >::iterator begin() { return rand_data.begin(); }
		typename std::array < std::random_device::result_type, Rand::state_size >::iterator end() { return rand_data.end(); }
	} seed;

public:
	Seed() : s(seed.begin(), seed.end()) { }

	template <typename I>
	auto generate(I a, I b) { return s.generate(std::forward<I>(a), std::forward<I>(b)); }

private:
	std::seed_seq s;

};

template<typename Real>
void printSamples(std::ostream& ostr, std::vector<Real>& samples) {
	for (auto v : samples) {
		ostr << v << ' ';
	}
	ostr << std::endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::function;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	using Rand = std::mt19937_64;
	using Real = posit<16,2>;

	//constexpr int N = 1'000'000;
	constexpr int N = 10;
	std::vector<Real> samples(N);

	Seed<Rand> seed;
	Rand rng(seed);
	std::uniform_real_distribution<double> uid(-1.0, 1.0);

	for (int j = 0; j < N; ++j) {
		samples[j] = Real(uid(rng));
	}
	sort(samples.begin(), samples.end());
	printSamples(cout, samples);

	for (int i = 1; i < N; ++i) {
		samples[i - 1] = lerp(samples[i - 1], samples[i]);
	}
	samples.pop_back();
	printSamples(cout, samples);

	// restore the previous ostream precision
	cout << setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

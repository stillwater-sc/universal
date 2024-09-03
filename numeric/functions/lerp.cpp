// lerp.cpp: evaluation of linear interpolation function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <random>  // only valid for native types
#include <array>
#include <algorithm>
#include <universal/number/posit/posit.hpp>
#include <universal/math/functions/lerp.hpp>

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
	using namespace sw::universal;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

#if defined(_MSC_VER)
	//using Rand = std::mt19937_64;
	using Rand = std::mt19937;
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
	std::sort(samples.begin(), samples.end());
	printSamples(std::cout, samples);

	for (int i = 1; i < N; ++i) {
		samples[i - 1] = sw::universal::lerp(samples[i - 1], samples[i]);
	}
	samples.pop_back();
	printSamples(std::cout, samples);
#endif

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

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

/* GCC 9 and 10 generate this error:
/usr/src/universal/tests/functions/lerp.cpp: In function 'int main(int, char**)':
/usr/src/universal/tests/functions/lerp.cpp:67:15: error: no matching function for call to 'std::mersenne_twister_engine<long unsigned int, 64, 312, 156, 31
, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005>::mersenne_twister_engine(Seed<s
td::mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 184444734447592
40704, 43, 6364136223846793005> >&)'
   67 |  Rand rng(seed);
	  |               ^
In file included from /usr/local/include/c++/9.3.0/random:49,
				 from /usr/src/universal/tests/functions/lerp.cpp:6:
/usr/local/include/c++/9.3.0/bits/random.h:530:9: note: candidate: 'template<class _Sseq, class> std::mersenne_twister_engine<_UIntType, __w, __n, __m, __r,
 __a, __u, __d, __s, __b, __t, __c, __l, __f>::mersenne_twister_engine(_Sseq&)'
  530 |         mersenne_twister_engine(_Sseq& __q)
	  |         ^~~~~~~~~~~~~~~~~~~~~~~
/usr/local/include/c++/9.3.0/bits/random.h:530:9: note:   template argument deduction/substitution failed:
/usr/local/include/c++/9.3.0/bits/random.h: In substitution of 'template<class _Sseq, class _Engine, class _Res, class _GenerateCheck> using __is_seed_seq =
 std::__and_<std::__not_<std::is_same<typename std::remove_cv<typename std::remove_reference<_Tp>::type>::type, _Engine> >, std::is_unsigned<typename _Sseq:
:result_type>, std::__not_<std::is_convertible<_Sseq, _Res> > > [with _Sseq = Seed<std::mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 1304310
9905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005> >; _Engine = std::mersenne_twister_engin
e<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 636413622384679
3005>; _Res = long unsigned int; _GenerateCheck = void]':
/usr/local/include/c++/9.3.0/bits/random.h:491:8:   required by substitution of 'template<class _UIntType, long unsigned int __w, long unsigned int __n, lon
g unsigned int __m, long unsigned int __r, _UIntType __a, long unsigned int __u, _UIntType __d, long unsigned int __s, _UIntType __b, long unsigned int __t,
 _UIntType __c, long unsigned int __l, _UIntType __f> template<class _Sseq> using _If_seed_seq = typename std::enable_if<std::__detail::__is_seed_seq<_Sseq,
 std::mersenne_twister_engine<_UIntType, __w, __n, __m, __r, __a, __u, __d, __s, __b, __t, __c, __l, __f>, _UIntType>::value>::type [with _Sseq = Seed<std::
mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 1844447344475924070
4, 43, 6364136223846793005> >; _UIntType = long unsigned int; long unsigned int __w = 64; long unsigned int __n = 312; long unsigned int __m = 156; long uns
igned int __r = 31; _UIntType __a = 13043109905998158313; long unsigned int __u = 29; _UIntType __d = 6148914691236517205; long unsigned int __s = 17; _UInt
Type __b = 8202884508482404352; long unsigned int __t = 37; _UIntType __c = 18444473444759240704; long unsigned int __l = 43; _UIntType __f = 63641362238467
93005]'
/usr/local/include/c++/9.3.0/bits/random.h:528:32:   required from here
/usr/local/include/c++/9.3.0/bits/random.h:197:13: error: no type named 'result_type' in 'class Seed<std::mersenne_twister_engine<long unsigned int, 64, 312
, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005> >'
  197 |       using __is_seed_seq = __and_<
	  |             ^~~~~~~~~~~~~
/usr/local/include/c++/9.3.0/bits/random.h:519:7: note: candidate: 'std::mersenne_twister_engine<_UIntType, __w, __n, __m, __r, __a, __u, __d, __s, __b, __t
, __c, __l, __f>::mersenne_twister_engine(std::mersenne_twister_engine<_UIntType, __w, __n, __m, __r, __a, __u, __d, __s, __b, __t, __c, __l, __f>::result_t
ype) [with _UIntType = long unsigned int; long unsigned int __w = 64; long unsigned int __n = 312; long unsigned int __m = 156; long unsigned int __r = 31;
_UIntType __a = 13043109905998158313; long unsigned int __u = 29; _UIntType __d = 6148914691236517205; long unsigned int __s = 17; _UIntType __b = 820288450
8482404352; long unsigned int __t = 37; _UIntType __c = 18444473444759240704; long unsigned int __l = 43; _UIntType __f = 6364136223846793005; std::mersenne
_twister_engine<_UIntType, __w, __n, __m, __r, __a, __u, __d, __s, __b, __t, __c, __l, __f>::result_type = long unsigned int]'
  519 |       mersenne_twister_engine(result_type __sd)
	  |       ^~~~~~~~~~~~~~~~~~~~~~~
/usr/local/include/c++/9.3.0/bits/random.h:519:43: note:   no known conversion for argument 1 from 'Seed<std::mersenne_twister_engine<long unsigned int, 64,
 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005> >' to 'std::mersen
ne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43,
 6364136223846793005>::result_type' {aka 'long unsigned int'}
  519 |       mersenne_twister_engine(result_type __sd)
	  |                               ~~~~~~~~~~~~^~~~
/usr/local/include/c++/9.3.0/bits/random.h:516:7: note: candidate: 'std::mersenne_twister_engine<_UIntType, __w, __n, __m, __r, __a, __u, __d, __s, __b, __t
, __c, __l, __f>::mersenne_twister_engine() [with _UIntType = long unsigned int; long unsigned int __w = 64; long unsigned int __n = 312; long unsigned int
__m = 156; long unsigned int __r = 31; _UIntType __a = 13043109905998158313; long unsigned int __u = 29; _UIntType __d = 6148914691236517205; long unsigned
int __s = 17; _UIntType __b = 8202884508482404352; long unsigned int __t = 37; _UIntType __c = 18444473444759240704; long unsigned int __l = 43; _UIntType _
_f = 6364136223846793005]'
  516 |       mersenne_twister_engine() : mersenne_twister_engine(default_seed) { }
	  |       ^~~~~~~~~~~~~~~~~~~~~~~
/usr/local/include/c++/9.3.0/bits/random.h:516:7: note:   candidate expects 0 arguments, 1 provided
/usr/local/include/c++/9.3.0/bits/random.h:461:11: note: candidate: 'constexpr std::mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905
998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005>::mersenne_twister_engine(const std::mersenne
_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6
364136223846793005>&)'
  461 |     class mersenne_twister_engine
	  |           ^~~~~~~~~~~~~~~~~~~~~~~
/usr/local/include/c++/9.3.0/bits/random.h:461:11: note:   no known conversion for argument 1 from 'Seed<std::mersenne_twister_engine<long unsigned int, 64,
 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005> >' to 'const std::
mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 1844447344475924070
4, 43, 6364136223846793005>&'
/usr/local/include/c++/9.3.0/bits/random.h:461:11: note: candidate: 'constexpr std::mersenne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905
998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005>::mersenne_twister_engine(std::mersenne_twist
er_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136
223846793005>&&)'
/usr/local/include/c++/9.3.0/bits/random.h:461:11: note:   no known conversion for argument 1 from 'Seed<std::mersenne_twister_engine<long unsigned int, 64,
 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43, 6364136223846793005> >' to 'std::mersen
ne_twister_engine<long unsigned int, 64, 312, 156, 31, 13043109905998158313, 29, 6148914691236517205, 17, 8202884508482404352, 37, 18444473444759240704, 43,
 6364136223846793005>&&'

*/

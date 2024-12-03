// harmonic_series.cpp: experiments with mixed-precision representations of the Harmonic Series
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/dd/dd.hpp>
#include <numbers>   // since C++20

namespace sw {
	namespace universal {

		template<typename Scalar>
		Scalar ForwardHarmonicSeries(unsigned long long terms) {
			Scalar one{ 1 }, sum{ 0 };
			for (unsigned long long i = 1; i <= terms; ++i) {
				sum += one / Scalar(i);
			}
			return sum;
		}

		template<typename Scalar>
		Scalar ReverseHarmonicSeries(unsigned long long terms) {
			Scalar one{ 1 }, sum{ 0 };
			for (unsigned long long i = terms; i > 0; --i) {
				sum += one / Scalar(i);
			}
			return sum;
		}


		template<typename Scalar>
		Scalar CompensatedHarmonicSeries(unsigned long long terms) {
			Scalar one{ 1 }, sum{ 0 };
			for (unsigned long long i = terms; i > 0; --i) {
				sum += one / Scalar(i);
			}
			return sum;
		}

		template<typename Scalar>
		void HarmonicSeriesConvergence(unsigned orderOfMagnitude = 6) {
			using std::abs;

			std::cout << "Harmonic Series Convergence for " << type_tag<Scalar>() << '\n';

			std::vector<uint64_t> term;
			uint64_t scale = 100;
			term.push_back(scale);
			for (unsigned i = 3; i < orderOfMagnitude; ++i) {
				scale *= 10;
				term.push_back(scale);
			}

			constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10;
			constexpr unsigned WIDTH = max_digits10 + 8;
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(max_digits10) << std::scientific;

			std::cout
				<< std::setw(WIDTH) << "terms"
				<< std::setw(WIDTH) << "forward"
				<< std::setw(WIDTH) << "reverse"
				<< std::setw(WIDTH) << "difference"
//				<< std::setw(WIDTH) << "compensated"
//				<< std::setw(WIDTH) << "diff"
				<< '\n';
			for (uint64_t terms : term) {
				Scalar forwardSum = ForwardHarmonicSeries<Scalar>(terms);
				Scalar reverseSum = ReverseHarmonicSeries<Scalar>(terms);
//				Scalar compensatedSum = CompensatedHarmonicSeries<Scalar>(terms);
				Scalar diff_fr = abs(forwardSum - reverseSum);
//				Scalar diff_rc = abs(reverseSum - compensatedSum);

				std::cout 
					<< std::setw(WIDTH) << terms
					<< std::setw(WIDTH) << forwardSum
					<< std::setw(WIDTH) << reverseSum
					<< std::setw(WIDTH) << diff_fr
//					<< std::setw(WIDTH) << compensatedSum
//					<< std::setw(WIDTH) << diff_rc
					<< '\n';
			}

			std::cout << std::setprecision(oldPrec) << std::defaultfloat;
		}

	}
}

int main()
try {
	using namespace sw::universal;

	HarmonicSeriesConvergence<float>();
	HarmonicSeriesConvergence<double>();
	HarmonicSeriesConvergence<dd>();

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


/*
Harmonic Series Convergence for float
			terms          forward          reverse       difference
			  100  5.187377930e+00  5.187376976e+00  9.536743164e-07
			 1000  7.485478401e+00  7.485471725e+00  6.675720215e-06
			10000  9.787612915e+00  9.787604332e+00  8.583068848e-06
		   100000  1.209085083e+01  1.209015274e+01  6.980895996e-04
		  1000000  1.435735798e+01  1.439265156e+01  3.529357910e-02
		 10000000  1.540368271e+01  1.668603134e+01  1.282348633e+00
		100000000  1.540368271e+01  1.880791855e+01  3.404235840e+00
	   1000000000  1.540368271e+01  1.880791855e+01  3.404235840e+00
	  10000000000  1.540368271e+01  1.880791855e+01  3.404235840e+00
	 100000000000  1.540368271e+01  1.880791855e+01  3.404235840e+00
Harmonic Series Convergence for double
			terms                  forward                  reverse               difference
			  100  5.18737751763962063e+00  5.18737751763962152e+00  8.88178419700125232e-16
			 1000  7.48547086055034328e+00  7.48547086055034061e+00  2.66453525910037570e-15
			10000  9.78760603604434820e+00  9.78760603604438550e+00  3.73034936274052598e-14
		   100000  1.20901461298633350e+01  1.20901461298634079e+01  7.28306304154102691e-14
		  1000000  1.43927267228649889e+01  1.43927267228657723e+01  7.83373366175510455e-13
		 10000000  1.66953113658572718e+01  1.66953113658599648e+01  2.69295696853077970e-12
		100000000  1.89978964138525548e+01  1.89978964138534465e+01  8.91731133378925733e-13
	   1000000000  2.13004815023485499e+01  2.13004815023461482e+01  2.40163444686913863e-12
	  10000000000  2.36030665949975003e+01  2.36030665948882685e+01  1.09231734768400202e-10
	 100000000000  2.59056516865364301e+01  2.59056516878463441e+01  1.30991395508317510e-09
Harmonic Series Convergence for double-double
			terms                                forward                                reverse                             difference
			 100  5.1873775176396202608051176756582e+00  5.1873775176396202608051176756583e+00  9.8607613152626475676466070660348e-32
			1000  7.4854708605503449126565182043340e+00  7.4854708605503449126565182043338e+00  1.7256332301709633243381562365561e-31
		   10000  9.7876060360443822641784779048557e+00  9.7876060360443822641784779048520e+00  3.7470892997998060757057106850932e-30
		  100000  1.2090146129863427947363219363515e+01  1.2090146129863427947363219363505e+01  9.7621537021100210919701409953745e-30
		 1000000  1.4392726722865723631381127493198e+01  1.4392726722865723631381127493196e+01  1.9721522630525295135293214132070e-30
	    10000000  1.6695311365859851815399118939716e+01  1.6695311365859851815399118939541e+01  1.7562015902482775317978607184608e-28
	   100000000  1.8997896413853898324417110394294e+01  1.8997896413853898324417110394212e+01  8.1745711303527348335790372577429e-29
	  1000000000  2.1300481502347944016685101850059e+01  2.1300481502347944016685101848909e+01  1.1499619845859299593389473160410e-27
 */

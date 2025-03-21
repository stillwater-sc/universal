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
#include <universal/number/qd/qd.hpp>

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
			Scalar one{ 1 }, sum{ 0 }, residual{ 0 };
			for (unsigned long long i = terms; i > 0; --i) {
				Scalar y = one / Scalar(i) - residual;
				Scalar t = sum + y;
				residual = (t - sum) - y;
				sum = t;
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
			for (unsigned i = 3; i <= orderOfMagnitude; ++i) {
				scale *= 10;
				term.push_back(scale);
			}

			constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10;
			constexpr unsigned WIDTH = max_digits10 + 8;
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(max_digits10) << std::scientific;

			std::cout
				<< std::setw(15) << "terms"
				<< std::setw(WIDTH) << "forward"
				<< std::setw(WIDTH) << "reverse"
				<< std::setw(WIDTH) << "compensated"
				<< std::setw(WIDTH) << "abs(forw - reverse)"
				<< std::setw(WIDTH) << "abs(reverse - compensated)"
				<< '\n';
			for (uint64_t terms : term) {
				Scalar forwardSum = ForwardHarmonicSeries<Scalar>(terms);
				Scalar reverseSum = ReverseHarmonicSeries<Scalar>(terms);
				Scalar compensatedSum = CompensatedHarmonicSeries<Scalar>(terms);
				Scalar diff_fr = abs(forwardSum - reverseSum);
				Scalar diff_rc = abs(reverseSum - compensatedSum);

				std::cout 
					<< std::setw(15) << terms
					<< std::setw(WIDTH) << forwardSum
					<< std::setw(WIDTH) << reverseSum
					<< std::setw(WIDTH) << compensatedSum
					<< std::setw(WIDTH) << diff_fr
					<< std::setw(WIDTH) << diff_rc
					<< '\n';
			}

			std::cout << std::setprecision(oldPrec) << std::defaultfloat;
		}

		/// <summary>
		/// Generate 10 segments of the Harmonic Series
		/// </summary>
		/// <typeparam name="Scalar"></typeparam>
		/// <param name="terms">must be an order of magnitude, such as, 10'000, or 1'000'000</param>
		/// <returns>ten equal-sized segments of the Harmonic Series</returns>
		template<typename Scalar>
		std::vector<Scalar> ReverseSegmentedHarmonicSeries(unsigned long long terms) {
			Scalar one{ 1 }, sum{ 0 };
			unsigned long long decimal_segment = terms / 10;
			unsigned long long part{ 0 };
			std::vector<Scalar> segments;
			for (unsigned long long i = terms; i > 0; --i) {
				if (part++ < decimal_segment) {
					sum += one / Scalar(i);
					//std::cout << i << " : " << part << " : " << sum << '\n';
				}
				else {
					part = 1;
					//std::cout << i << "                      : " << sum << '\n';
					segments.push_back(sum);
					sum = one / Scalar(i);
					//std::cout << i << " : " << part << " : " << sum << '\n';
				}
			}
			segments.push_back(sum);
			return segments;
		}

		template<typename Scalar>
		void SegmentedHarmonicSeries(unsigned orderOfMagnitude = 6) {

			constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10;
			constexpr unsigned WIDTH = max_digits10 + 8;
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(max_digits10) << std::scientific;
			uint64_t terms = static_cast<uint64_t>(ipow(10, orderOfMagnitude));
			uint64_t decimal_segment = terms / 10;
			uint64_t upperbound = terms;
			uint64_t lowerbound = terms - decimal_segment + 1;
			auto segments = ReverseSegmentedHarmonicSeries<Scalar>(terms);
			std::cout << "Harmonic Series Segments for " << type_tag<Scalar>() << " and " << terms << " terms\n";
			for (auto segment : segments) {
				std::cout << "[ " << std::setw(12) << upperbound << ", " << std::setw(12) << lowerbound << "] = " << std::setw(WIDTH) << segment << '\n';
				upperbound = lowerbound - 1;
				lowerbound = upperbound - decimal_segment + 1;
			}
			std::cout << std::setprecision(oldPrec) << std::defaultfloat;
		}
	}
}

int main()
try {
	using namespace sw::universal;

	constexpr bool CONVERGENCE_TEST = true;

	if constexpr (CONVERGENCE_TEST) {
		HarmonicSeriesConvergence<float>(5);
		HarmonicSeriesConvergence<double>(5);
		HarmonicSeriesConvergence<dd>(5);
		HarmonicSeriesConvergence<qd>(5);
	}

	// compare the value of equal segments of the Harmonic Series
	std::cout << "Values of ten segments of the reverse Harmonic Series\n";
	SegmentedHarmonicSeries<double>(6);
	SegmentedHarmonicSeries<double>(7);
	SegmentedHarmonicSeries<double>(8);

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


Harmonic Series Convergence for float
		  terms          forward          reverse      compensatedabs(forw - reverse)abs(reverse - compensated)
			100  5.187377930e+00  5.187376976e+00  5.187376976e+00  9.536743164e-07  0.000000000e+00
		   1000  7.485478401e+00  7.485471725e+00  7.485471725e+00  6.675720215e-06  0.000000000e+00
		  10000  9.787612915e+00  9.787604332e+00  9.787604332e+00  8.583068848e-06  0.000000000e+00
		 100000  1.209085083e+01  1.209015274e+01  1.209015274e+01  6.980895996e-04  0.000000000e+00
		1000000  1.435735798e+01  1.439265156e+01  1.439265156e+01  3.529357910e-02  0.000000000e+00
Harmonic Series Convergence for double
		  terms                  forward                  reverse              compensated      abs(forw - reverse)abs(reverse - compensated)
			100  5.18737751763962063e+00  5.18737751763962152e+00  5.18737751763962152e+00  8.88178419700125232e-16  0.00000000000000000e+00
		   1000  7.48547086055034328e+00  7.48547086055034061e+00  7.48547086055034061e+00  2.66453525910037570e-15  0.00000000000000000e+00
		  10000  9.78760603604434820e+00  9.78760603604438550e+00  9.78760603604438550e+00  3.73034936274052598e-14  0.00000000000000000e+00
		 100000  1.20901461298633350e+01  1.20901461298634079e+01  1.20901461298634079e+01  7.28306304154102691e-14  0.00000000000000000e+00
		1000000  1.43927267228649889e+01  1.43927267228657723e+01  1.43927267228657723e+01  7.83373366175510455e-13  0.00000000000000000e+00
Harmonic Series Convergence for double-double
		  terms                                forward                                reverse                            compensated                    abs(forw - reverse)             abs(reverse - compensated)
			100  5.1873775176396202608051176756582e+00  5.1873775176396202608051176756583e+00  5.1873775176396202608051176756583e+00  9.8607613152626475676466070660348e-32  0.0000000000000000000000000000000e+00
		   1000  7.4854708605503449126565182043340e+00  7.4854708605503449126565182043338e+00  7.4854708605503449126565182043339e+00  1.7256332301709633243381562365561e-31  9.8607613152626475676466070660348e-32
		  10000  9.7876060360443822641784779048557e+00  9.7876060360443822641784779048520e+00  9.7876060360443822641784779048516e+00  3.7470892997998060757057106850932e-30  3.9443045261050590270586428264139e-31
		 100000  1.2090146129863427947363219363515e+01  1.2090146129863427947363219363505e+01  1.2090146129863427947363219363504e+01  9.7621537021100210919701409953745e-30  1.1832913578315177081175928479242e-30
		1000000  1.4392726722865723631381127493198e+01  1.4392726722865723631381127493196e+01  1.4392726722865723631381127493189e+01  1.9721522630525295135293214132070e-30  7.1983557601417327243820231582054e-30
Harmonic Series Convergence for quad-double
		  terms                                                                forward																  reverse                                                            compensated										abs(forw - reverse)                                             abs(reverse - compensated)
			100  5.187377517639620260805117675658253157908972126708451653176533957e+00  5.187377517639620260805117675658253157908972126708451653176533957e+00  5.187377517639620260805117675658253158645722258377999562277504485e+00  1.139468129491175849696789072347434858506515900034393395953573599e-64  7.367501316695479091009704982983456205177381283527213529792001890e-37
		   1000  7.485470860550344912656518204333912288211763757312524340613351418e+00  7.485470860550344912656518204333912288211763757312524340613351418e+00  7.485470860550344912656518204333900176529641491022878562465131718e+00  2.611281130083944655555141612696505473448710303239569086704054512e-64  1.211168212226628964577814891623631684779632410920952206480023915e-32
		  10000  9.787606036044382264178477904851618431556651337983996447357779474e+00  9.787606036044382264178477904851618431556651337983996447357779477e+00  9.787606036044382264178477904851605334859421921046437616658668660e+00  2.430865342914508479353149458754769665707290714967501543949590034e-63  1.309669722941693755883069919766420545022922041537973111840587023e-32
		 100000  1.209014612986342794736321936350424565105115759726661007624512786e+01  1.209014612986342794736321936350424565105115759726661007624512786e+01  1.209014612986342794736321936350421950079369515482094470913372897e+01  3.038581678643135599191437512955933641831226534678771300891990704e-64  2.615025746244244566536711212586594778392519248876896505112693774e-32
		1000000  1.439272672286572363138112749318852538366686486363373048959008936e+01  1.439272672286572363138112749318852538366686486363373048959008938e+01  1.439272672286572363138112749318858767664479996908977961859022854e+01  1.792763190399450003522947725831642628459126902288532388662822650e-62  6.229297793510545604912899986060474396840690183047488519229854930e-32

 */

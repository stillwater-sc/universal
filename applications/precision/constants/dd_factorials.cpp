// constants.cpp: test suite runner for creating and verifying double-double constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/utility/error.hpp>

namespace sw {
	namespace universal {

		template<typename Scalar>
		Scalar factorial(unsigned n) {
			Scalar result = 1;
			for (unsigned i = 2; i <= n; ++i) {
				result *= Scalar(i);
			}
			return result;
        }

		const dd_cascade ddc_inverse_factorials[] = {
			dd_cascade("1.0"),											//	1/0!
			dd_cascade("1.0"),											//	1/1!
			dd_cascade("0.5"),											//	1/2!
			dd_cascade("1.66666666666666666666666666666666667E-1"),		//	1/3!
			dd_cascade("4.16666666666666666666666666666666667E-2"),		//	1/4!
			dd_cascade("8.33333333333333333333333333333333333E-3"),		//	1/5!
			dd_cascade("1.38888888888888888888888888888888889E-3"),		//	1/6!
			dd_cascade("1.98412698412698412698412698412698413E-4"),		//	1/7!
			dd_cascade("2.48015873015873015873015873015873016E-5"),		//	1/8!
			dd_cascade("2.75573192239858906525573192239858907E-6"),		//	1/9!
			dd_cascade("2.75573192239858906525573192239858907E-7"),		//	1/10!
			dd_cascade("2.50521083854417187750521083854417188E-8"),		//	1/11!
			dd_cascade("2.08767569878680989792100903212014323E-9"),		//	1/12!
			dd_cascade("1.60590438368216145993923771701549479E-10"),	//	1/13!
			dd_cascade("1.14707455977297247138516979786821057E-11"),	//	1/14!
			dd_cascade("7.64716373181981647590113198578807044E-13"),	//	1/15!
			dd_cascade("4.77947733238738529743820749111754403E-14"),	//	1/16!
			dd_cascade("2.81145725434552076319894558301032002E-15"),	//	1/17!
			dd_cascade("1.56192069685862264622163643500573334E-16"),	//	1/18!
			dd_cascade("8.22063524662432971695598123687228075E-18"),	//	1/19!
			dd_cascade("4.11031762331216485847799061843614037E-19"),	//	1/20!
			dd_cascade("1.95729410633912612308475743735054304E-20"),	//	1/21!
			dd_cascade("8.89679139245057328674889744250246834E-22"),	//	1/22!
			dd_cascade("3.86817017063068403771691193152281232E-23"),	//	1/23!
			dd_cascade("1.61173757109611834904871330480117180E-24"),	//	1/24!
			dd_cascade("6.44695028438447339619485321920468721E-26"),	//	1/25!
			dd_cascade("2.47959626322479746007494354584795662E-27"),	//	1/26!
			dd_cascade("9.18368986379554614842571683647391340E-29"),	//	1/27!
			dd_cascade("3.27988923706983791015204172731211193E-30"),	//	1/28!
			dd_cascade("1.13099628864477169315587645769383170E-31"),	//	1/29!
			dd_cascade("3.76998762881590564385292152564610566E-33"),	//	1/30!
			dd_cascade("1.21612504155351794962997468569229215E-34"),	//	1/31!
			dd_cascade("3.80039075485474359259367089278841297E-36"),	//	1/32!
			dd_cascade("1.15163356207719502805868814932982211E-37"),	//	1/33!
		};

	}
}

int main()
try {
	using namespace sw::universal;

	{
		unsigned   entry     = 3;
		dd_cascade inv_fact  = ddc_inverse_factorials[entry];
		dd_cascade reference = ddc_one / factorial<dd_cascade>(entry);
		if (inv_fact != reference) {
			std::cout << "inv_fact  : " << to_binary(inv_fact) << " : " << inv_fact << '\n';
			std::cout << "reference : " << to_binary(reference) << " : " << reference << '\n';
			return EXIT_FAILURE;
		}
	}

	// check that dd_cascade can represent the factorials accurately enough
	unsigned entries = sizeof(ddc_inverse_factorials) / sizeof(dd_cascade);
	for (unsigned i = 0; i < entries; ++i) {
		dd_cascade inv_fact = ddc_inverse_factorials[i];
		dd_cascade reference = ddc_one / factorial<dd_cascade>(i);
		if (inv_fact != reference) {
			std::cout << "inv_fact  : " << to_binary(inv_fact) << " : " << inv_fact << '\n';
			std::cout << "reference : " << to_binary(reference) << " : " << reference << '\n';
			std::cout << "relative error: " << RelativeError(inv_fact, reference) << '\n';
			std::cout << "log relative error: " << LogRelativeError(inv_fact, reference) << '\n';
			std::cout << "difference: " << inv_fact - reference << '\n';
		}
	}

	// create code that we can paste into a constant definition header file
	// constexpr dd_cascade ddc_pi_4     (0.785398163397448279,  3.061616997868383018e-17);  // pi/4
	std::cout << "generating dd_cascade factorials\n";
	int i = 0;
	for (auto dd : ddc_inverse_factorials) {
		std::string name   = "ddc_1_" + std::to_string(i) + "_factorial";
		std::string symbol = "1/" + std::to_string(i) + "!";
		std::cout << "constexpr dd_cascade " << name << to_pair(dd) << "; // " << symbol << '\n';
		++i;
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

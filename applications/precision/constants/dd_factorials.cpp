// constants.cpp: test suite runner for creating and verifying double-double constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>

namespace sw {
	namespace universal {

		const dd_cascade ddc_inv_int[] = {
			dd_cascade(std::numeric_limits< dd_cascade >::infinity()),	//	1/0
			dd_cascade("1.0"),											//	1/1
			dd_cascade("0.5"),											//	1/2
			dd_cascade("0.3333333333333333333333333333333333333"),		//	1/3
			dd_cascade("0.25"),											//	1/4
			dd_cascade("0.2"),											//	1/5
			dd_cascade("0.1666666666666666666666666666666666667"),		//	1/6
			dd_cascade("0.1428571428571428571428571428571428571"),		//	1/7
			dd_cascade("0.125"),										//	1/8
			dd_cascade("0.1111111111111111111111111111111111111"),		//	1/9
			dd_cascade("0.1"),											//	1/10
			dd_cascade("0.0909090909090909090909090909090909091"),		//	1/11
			dd_cascade("0.0833333333333333333333333333333333333"),		//	1/12
			dd_cascade("0.0769230769230769230769230769230769231"),		//	1/13
			dd_cascade("0.0714285714285714285714285714285714286"),		//	1/14
			dd_cascade("0.0666666666666666666666666666666666667"),		//	1/15
			dd_cascade("0.0625"),										//	1/16
			dd_cascade("0.0588235294117647058823529411764705882"),		//	1/17
			dd_cascade("0.0555555555555555555555555555555555556"),		//	1/18
			dd_cascade("0.0526315789473684210526315789473684211"),		//	1/19
			dd_cascade("0.05"),											//	1/20
			dd_cascade("0.0476190476190476190476190476190476190"),		//	1/21
			dd_cascade("0.0454545454545454545454545454545454545"),		//	1/22
			dd_cascade("0.0434782608695652173913043478260869565"),		//	1/23
			dd_cascade("0.0416666666666666666666666666666666667"),		//	1/24
			dd_cascade("0.04"),											//	1/25
			dd_cascade("0.0384615384615384615384615384615384615"),		//	1/26
			dd_cascade("0.0370370370370370370370370370370370370"),		//	1/27
			dd_cascade("0.0357142857142857142857142857142857143"),		//	1/28
			dd_cascade("0.0344827586206896551724137931034482759"),		//	1/29
			dd_cascade("0.0333333333333333333333333333333333333"),		//	1/30
			dd_cascade("0.0322580645161290322580645161290322581"),		//	1/31
			dd_cascade("0.03125"),										//	1/32
			dd_cascade("0.0303030303030303030303030303030303030"),		//	1/33
			dd_cascade("0.0294117647058823529411764705882352941"),		//	1/34
			dd_cascade("0.0285714285714285714285714285714285714"),		//	1/35
			dd_cascade("0.0277777777777777777777777777777777778"),		//	1/36
			dd_cascade("0.0270270270270270270270270270270270270"),		//	1/37
			dd_cascade("0.0263157894736842105263157894736842105"),		//	1/38
			dd_cascade("0.0256410256410256410256410256410256410"),		//	1/39
			dd_cascade("0.025"),										//	1/40
			dd_cascade("0.0243902439024390243902439024390243902")		//	1/41
		};

		const dd_cascade ddc_inv_fact[] = {
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

	// create code that we can paste into a constant definition header file
	// constexpr dd_cascade ddc_pi_4     (0.785398163397448279,  3.061616997868383018e-17);  // pi/4
	std::cout << "generating dd_cascade factorials\n";
	int i = 0;
	for (auto dd : ddc_inv_fact) {
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

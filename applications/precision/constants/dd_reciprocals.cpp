// constants.cpp: test suite runner for creating and verifying double-double constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/dd_cascade/dd_cascade.hpp>

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

	}
}

int main()
try {
	using namespace sw::universal;

	// create code that we can paste into a constant definition header file
	// constexpr dd_cascade ddc_pi_4     (0.785398163397448279,  3.061616997868383018e-17);  // pi/4
	std::cout << "generating dd_cascade reciprocals\n";
	int i = 0;
	for (auto dd : ddc_inv_int) {
		std::string name = "ddc_1_" + std::to_string(i);
		std::string symbol = "1/" + std::to_string(i);
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

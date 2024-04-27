//  time_precision_lyapunov.cpp : example of the relationship between execution time, floating point precision
//                                and the Lyaponov exponent in a chaotic system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
//#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

/*
On the relation between reliable computation time, float-point precision and the Lyapunov exponent in chaotic systems
Wang PengFei (1,2) and Li JianPing (3)
1 Center for Monsoon System Research, Institute of Atmospheric Physics, 
  Chinese Academy of Sciences, Beijing 100190, China
2 State Key Laboratory of Numerical Modeling for Atmospheric Sciences 
  and Geophysical Fluid Dynamics, 
  Institute of Atmospheric Physics, Chinese Academy of Sciences, 
  Beijing 100029, China
3 College of Global Change and Earth System Science, 
  Beijing Normal University, 100875, China
Corresponding author: wpf@mail.iap.ac.cn

Abstract
 The relation among reliable computation time, Tc, float-point precision, K, and the
Lyapunov exponent, λ, is obtained as Tc= (lnB/λ)K+C, where B is the base of the float-point
system and C is a constant dependent only on the chaotic equation. The equation shows good
agreement with numerical experimental results, especially the scale factors.

Keywords: reliable computation time, Lyapunov exponent, float precision
 */

int main()
try {
	std::cout << "Time-Precision Trade-off for Lyaponov exponent\n";

    
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

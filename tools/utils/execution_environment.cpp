// execution_environment.cpp: cli to show the execution environment: compiler and arch
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/compiler.hpp>
#include <universal/common/number_traits_reports.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/verification/test_reporters.hpp>

void report_architecture() {
#ifdef UNIVERSAL_ARCH_X86_64
	std::cout << "Intel/AMD x86-64\n";
#endif

#ifdef UNIVERSAL_ARCH_POWER
	std::cout << "IBM POWER\n";
#endif

#ifdef UNIVERSAL_ARCH_ARM
	std::cout << "ARM64\n";
#endif

#ifdef UNIVERSAL_ARCH_RISCV
	std::cout << "RISC-V\n";
#endif

}

template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
int VerifyFloatingPointScales(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	int largestScale = std::numeric_limits<Real>::max_exponent - 1;
	Real r = sw::universal::ipow<Real>(static_cast<size_t>(largestScale));
	for (int i = 0; i < largestScale + 1; ++i) {
		if (largestScale - i != scale(r)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL : " << std::setw(4) << largestScale - i << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		}
		r /= 2.0;
	}

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	report_compiler();
	report_architecture();

	bool reportTestCases = true;

    ReportTestSuiteHeader("execution environment", reportTestCases);

	numberTraits<float>(std::cout);
	numberTraits<double>(std::cout);
	numberTraits<long double>(std::cout);

	VerifyFloatingPointScales<double>(reportTestCases);

	double d{ 1.0 };
	std::cout << color_print(d) << '\n';
	
	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

/*
  Clang on Apple M1 does not support long double: long double is mapped to double
 std::numeric_limits< f >
min exponent                                           -125
max exponent                                            128
radix                                                     2
radix digits                                             24
min                                             1.17549e-38
max                                             3.40282e+38
lowest                                         -3.40282e+38
epsilon (1+1ULP-1)                              1.19209e-07
round_error                                             0.5
smallest value                                   1.4013e-45
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

std::numeric_limits< d >
min exponent                                          -1021
max exponent                                           1024
radix                                                     2
radix digits                                             53
min                                            2.22507e-308
max                                            1.79769e+308
lowest                                        -1.79769e+308
epsilon (1+1ULP-1)                              2.22045e-16
round_error                                             0.5
smallest value                                 4.94066e-324
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

std::numeric_limits< e >
min exponent                                          -1021
max exponent                                           1024
radix                                                     2
radix digits                                             53
min                                            2.22507e-308
max                                            1.79769e+308
lowest                                        -1.79769e+308
epsilon (1+1ULP-1)                              2.22045e-16
round_error                                             0.5
smallest value                                 4.94066e-324
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan 
*/
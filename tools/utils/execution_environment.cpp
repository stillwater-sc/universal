// execution_environment.cpp: cli to show the execution environment: compiler and arch
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/compiler.hpp>

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

int main()
try {
	using namespace sw::universal;

	report_compiler();
	report_architecture();

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

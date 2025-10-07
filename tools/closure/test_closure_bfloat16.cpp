// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// test_closure.cpp: test closure plots for configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/native/ieee754.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

int main() {
	using namespace sw::universal;

	ClosurePlotPNG<bfloat16> plotter;
	double error = 0.0;
	bfloat16 a, b, c;

	a.setbits(0x0001); // smallest positive
	b.setbits(0x0001); // smallest positive

	std::cout << to_binary(a) << " : " << a << '\n';

	for (unsigned int i = 0; i < 16; ++i) {
		c = a * b;
		std::cout << std::setw(5) << i << " : " << to_binary(c) << " : " << c << '\n';
		std::cout << plotter.cr(a, b, c, 0.0, error) << '\n';
		float fc = float(a) * float(b);
		std::cout << std::setw(5) << i << " : " << to_binary(fc) << " : " << fc << '\n';
		++b;
	}


	a.setbits(0x8001); // smallest negative
	std::cout << to_binary(a) << " : " << a << '\n';
	for (unsigned int i = 0; i < 16; ++i) {
		c = a * b;
		std::cout << std::setw(5) << i << " : " << to_binary(c) << " : " << c << '\n';
		ClosureResult result = plotter.cr(a, b, c, 0.0, error);
		std::cout << result << '\n';
		float fc = float(a) * float(b);
		std::cout << std::setw(5) << i << " : " << to_binary(fc) << " : " << fc << '\n';
		++b;
	}

}

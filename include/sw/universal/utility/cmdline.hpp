#pragma once
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>

namespace sw { namespace universal {

	/// <summary>
	/// print the cmd line if there is one or more parameters provided
	/// </summary>
	/// <param name="argc">number of arguments</param>
	/// <param name="argv">array of char* representing the arguments</param>
	void print_cmd_line(int argc, char* argv[]) {
		if (argc > 1) {
			for (int i = 0; i < argc; ++i) {
				std::cout << argv[i] << ' ';
			}
			std::cout << std::endl;
		}
	}

}} // namespace sw::universal

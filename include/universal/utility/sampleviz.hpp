#pragma once
// sampleviz.hpp: utility
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
namespace sw {
	namespace universal {

		template<typename Real, typename NumberSystem, typename EnvelopingNumberSystem>
		void sampleviz(NumberSystem start, NumberSystem stop, Real sample) {
			using namespace sw::universal;
			NumberSystem a{}, s(sample);
			EnvelopingNumberSystem b{};
			std::string tag1 = type_tag(a);
			std::string tag2 = type_tag(b);
			size_t twidth = tag2.size() + 5;
			size_t vwidth = to_binary(b).size() + 2;
			a = start;
			if (a < 0.0f) {
				while (a > stop) {
					std::cout << std::setw(twidth) << tag1 << to_binary(a) << "   " << a << '\n';
					b = Real(a);
					--b; // intermediate sample
					if (Real(a) > sample && sample > Real(b)) {
						std::cout << std::setw(twidth) << "----->  sample " << std::setw(vwidth) << " " << sample << " round down to: " << s << '\n';
					}
					std::cout << std::setw(twidth) << tag2 << to_binary(b) << "  " << b << '\n';
					--a;
					if (Real(b) > sample && sample > Real(a)) {
						std::cout << std::setw(twidth) << "----->  sample " << std::setw(vwidth) << " " << sample << " round up   to: " << s << '\n';
					}
				}
			}
			else {
				while (a < stop) {
					std::cout << std::setw(twidth) << tag1 << to_binary(a) << "   " << a << '\n';
					b = Real(a);
					++b; // intermediate sample
					if (Real(a) < sample && sample < Real(b)) {
						std::cout << std::setw(twidth) << "----->  sample " << std::setw(vwidth) << " " << sample << " round down to: " << s << '\n';
					}
					std::cout << std::setw(twidth) << tag2 << to_binary(b) << "  " << b << '\n';
					++a;
					if (Real(b) < sample && sample < Real(a)) {
						std::cout << std::setw(twidth) << "----->  sample " << std::setw(vwidth) << " " << sample << " round up   to: " << s << '\n';
					}
				}
			}
		}

} }  // namespace sw::universal

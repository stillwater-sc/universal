#pragma once
// occurrence.hpp: utility object to track arithmetic operation counts during execution of a specific number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	template<typename NumberSystem>
	struct occurrence {
		using value_type = NumberSystem;
		size_t load;
		size_t store;
		size_t add;
		size_t sub;
		size_t mul;
		size_t div;
		size_t rem;
		size_t sqrt;

		occurrence() : load{ 0 }, store{ 0 }, add{ 0 }, sub{ 0 }, mul{ 0 }, div{ 0 }, rem{ 0 }, sqrt{ 0 } {};
		void reset() {
			load = 0;
			store = 0;
			add = 0;
			sub = 0;
			mul = 0;
			div = 0;
			rem = 0;
			sqrt = 0;
		}
		void report(std::ostream& ostr) {
			ostr << "Load    : " << load << '\n';
			ostr << "Store   : " << store << '\n';
			ostr << "Add     : " << add << '\n';
			ostr << "Sub     : " << sub << '\n';
			ostr << "Mul     : " << mul << '\n';
			ostr << "Div     : " << div << '\n';
			ostr << "Rem     : " << rem << '\n';
			ostr << "Sqrt    : " << sqrt << '\n';
		}
	};

}} // namespace sw::universal

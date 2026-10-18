#pragma once
// table.hpp: encoding tables of lindex configurations (#1349 part 6, #1697)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// One row per encoding: the pattern, the sign, the level l and index f of |value|, psi(|value|)
// = l + f, and the value.  A value past double prints as phi(psi), the level-index map itself.
//
// The columns are the ones slindex (#1350) will mirror, with its reciprocal bit added
// between sign and level, so the two tables read side by side (the takums.cpp and
// takum_logs.cpp pattern, #1348).
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <universal/number/lindex/iostream.hpp>
#include <universal/number/lindex/manipulators.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits = 3, typename BlockType = std::uint8_t>
void GenerateLindexTable(std::ostream& ostr, bool csvFormat = false) {
	using L = lindex<nbits, lbits, BlockType>;
	static_assert(nbits <= 16, "an encoding table of more than 2^16 rows is not useful");
	const unsigned size = (1u << nbits);
	const std::streamsize precision = ostr.precision();
	ostr << std::setprecision(10);
	L v;
	if (csvFormat) {
		ostr << "\"Generate Value table for a lindex<" << nbits << "," << lbits << "> in CSV format\"" << '\n';
		ostr << "#, Binary, sign, level, index, psi, value\n";
		for (unsigned i = 0; i < size; i++) {
			v.setbits(i);
			ostr << i << "," << to_binary(v) << "," << v.sign() << ",";
			if (v.isnar()) ostr << ",,,NaR\n";
			else ostr << v.level() << "," << v.index() << "," << v.psi() << "," << v << '\n';
		}
		ostr << '\n';
	}
	else {
		ostr << "Generate Value table for a lindex<" << nbits << "," << lbits << "> in TXT format" << '\n';
		const unsigned index_column = 5;
		const unsigned bin_column = nbits + 6;   // to_binary adds the "0b" and two dots
		const unsigned sign_column = 6;
		const unsigned level_column = 7;
		const unsigned f_column = 14;
		const unsigned psi_column = 14;
		const unsigned value_column = 24;
		ostr << std::setw(index_column) << " # "
		     << std::setw(bin_column) << "Binary"
		     << std::setw(sign_column) << "sign"
		     << std::setw(level_column) << "level"
		     << std::setw(f_column) << "index"
		     << std::setw(psi_column) << "psi"
		     << std::setw(value_column) << "value"
		     << '\n';
		for (unsigned i = 0; i < size; i++) {
			v.setbits(i);
			ostr << std::setw(4) << i << ": " << std::setw(bin_column) << to_binary(v) << std::setw(sign_column) << v.sign();
			if (v.isnar()) {
				ostr << std::setw(level_column) << " " << std::setw(f_column) << " " << std::setw(psi_column) << " "
				     << std::setw(value_column) << "NaR" << '\n';
			}
			else {
				ostr << std::setw(level_column) << v.level() << std::setw(f_column) << v.index()
				     << std::setw(psi_column) << v.psi() << std::setw(value_column) << v << '\n';
			}
		}
	}
	ostr << std::setprecision(precision);
}

}} // namespace sw::universal

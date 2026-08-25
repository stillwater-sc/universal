#pragma once
// iostream.hpp: stream insertion and extraction for blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of blocktriple's text layer (#1334). manipulators.hpp is
// the <iomanip> half; operator<< delegates to blocktriple::to_string and the enum
// printer to to_string(BlockTripleOperator), both defined there.
#include <ios>
#include <ostream>
#include <istream>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/internal/blocktriple/manipulators.hpp>

namespace sw { namespace universal {

// the operator specialization tag, streamed
inline std::ostream& operator<<(std::ostream& ostr, const BlockTripleOperator& op) {
	return ostr << to_string(op);
}

// blocktriple ostream operator: honors all standard formatting flags
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<fbits, op, bt>& a) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool bShowpos    = fmt & std::ios_base::showpos;
	bool bUppercase  = fmt & std::ios_base::uppercase;
	bool bFixed      = fmt & std::ios_base::fixed;
	bool bScientific = fmt & std::ios_base::scientific;
	bool bInternal   = fmt & std::ios_base::internal;
	bool bLeft       = fmt & std::ios_base::left;
	return ostr << a.to_string(precision, width, bFixed, bScientific,
	                            bInternal, bLeft, bShowpos, bUppercase, fillChar);
}

template<unsigned fbits, BlockTripleOperator op, typename bt>
inline std::istream& operator>> (std::istream& istr, blocktriple<fbits, op, bt>& a) {
	double v{};
	istr >> v;
	a = blocktriple<fbits, op, bt>(v);
	return istr;
}

}} // namespace sw::universal

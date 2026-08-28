#pragma once
// iostream.hpp: stream insertion and extraction for e8m0
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the e8m0 headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half. This header includes only core.hpp -- the operators convert to
// a native float and stream that, so they need nothing from the text layer.
#include <iostream>
#include <istream>
#include <ostream>

#include <universal/number/e8m0/core.hpp>

namespace sw { namespace universal {

inline std::ostream& operator<<(std::ostream& ostr, e8m0 v) {
	if (v.isnan()) return ostr << "NaN";
	return ostr << float(v);
}

inline std::istream& operator>>(std::istream& istr, e8m0& v) {
	float f;
	if (!(istr >> f)) {
		// extraction failed (already-bad stream, EOF, or a non-numeric token). On an
		// already-bad stream operator>> never assigns, so f stays indeterminate --
		// constructing from it would be UB and would clobber v with garbage.
		return istr;
	}
	v = e8m0(f);
	return istr;
}

}} // namespace sw::universal

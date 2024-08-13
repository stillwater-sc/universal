#pragma once
// attributes.hpp: information functions for quad-double floating-point type and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about the properties of a quad-double (qd) configuration
	inline bool sign(const qd& a) {
		return a.sign();
	}

	inline int scale(const qd& a) {
		return a.scale();
	}

	// generate the maxneg through maxpos value range of a quad-double configuration
	std::string qd_range() {
		qd v;
		std::stringstream s;
		s << std::setw(80) << type_tag(v) << " : [ "
			<< v.maxneg() << " ... "
			<< v.minneg() << " "
			<< "0 "
			<< v.minpos() << " ... "
			<< v.maxpos() << " ]";
		return s.str();
	}

	/*
	// report dynamic range of a type, specialized for a quad-double
	std::string dynamic_range(const qd& a) {
		std::stringstream s;
		qd b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << type_tag(a) << ": ";
		s << "minpos scale " << std::setw(10) << d.scale() << "     ";
		s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
		s << "[" << b << " ... " << c << ", -0, +0, " << d << " ... " << e << "]\n";
		s << "[" << to_binary(b) << " ... " << to_binary(c) << ", -0, +0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
		qd ninf(SpecificValue::infneg), pinf(SpecificValue::infpos);
		s << "inclusive range = (" << to_binary(ninf) << ", " << to_binary(pinf) << ")\n";
		s << "inclusive range = (" << ninf << ", " << pinf << ")\n";
		return s.str();
	}
	*/

	int minpos_scale(const qd& b) {
		qd c(b);
		return c.minpos().scale();
	}

	int maxpos_scale(const qd& b) {
		qd c(b);
		return c.maxpos().scale();
	}

	int max_negative_scale(const qd& b) {
		qd c(b);
		return c.maxneg().scale();
	}

}} // namespace sw::universal

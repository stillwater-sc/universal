#pragma once
// manipulators.hpp: definitions of helper functions for the manipulation of SORN numbers 
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>   // std::size_t
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>  // for typeid()

#include <universal/native/manipulators.hpp>
#include <universal/utility/color_print.hpp>  // pull in the color printing for shells utility
#include <universal/number/sorn/core.hpp>
#include <universal/number/sorn/sorn_type_tag.hpp>

namespace sw { namespace universal {

// The text members of sornInterval and sorn: declared in their classes, defined here
// because they build their text with a stringstream (#1334, #1467).

// sornInterval::getInt: the interval as text, "v" for a point, "(a,b]" etc otherwise
template<typename Real>
std::string sornInterval<Real>::getInt() {
	std::stringstream configStream;
	if ((this->lowerBound == this->upperBound) && (not this->lowerIsOpen && not this->upperIsOpen)) {
		configStream << this->lowerBound;
	}
	else {
		configStream << (this->lowerIsOpen ? '(' : '[') << this->lowerBound << ',' << this->upperBound << (this->upperIsOpen ? ')' : ']');
	}
	return configStream.str();
}

// sorn::getConfig: all configuration parameters and flags as text
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
std::string sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>::getConfig() {
	std::stringstream configStream;
	configStream << "-- configuration parameters:" << '\t' << "start: " << start << ", stop: " << stop << ", steps: " << steps << ", stepSize: " << stepSize << '\n';
	configStream << "-- configuration flags:" << "\t\t";
	if (flagLin) configStream << "Lin, "; else if (flagLog) configStream << "Log, ";
	if (flagHalfopen) configStream << "Halfopen, "; else if (flagOpen) configStream << "Open, ";
	if (flagNeg) configStream << "Neg, ";
	if (flagInf) configStream << "Inf, ";
	if (flagZero) configStream << "Zero";
	configStream << '\n';
	return configStream.str();
}

// sorn::getDT: the SORN datatype configuration as text
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
std::string sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>::getDT() {
	std::stringstream DTstream;
	DTstream << "-- SORN datatype:" << "\t\t";
	for (std::size_t b = 0; b < sornDT.size(); b++) {
		DTstream << sornDT[b].getInt() << ' ';
	}
	DTstream << '\n';
	return DTstream.str();
}

#ifdef LATER
	// report dynamic range of a type, specialized for sorn
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline std::string dynamic_range(const sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& a) {
		std::stringstream s;
		sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << type_tag(a) << ": ";
		s << "minpos scale " << std::setw(10) << d.scale() << "     ";
		s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		s << "[" << to_binary(b) << " ... " << to_binary(c) << ", 0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
		return s.str();
	}

	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline std::string range() {
		std::stringstream s;
		sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}
#endif

	// report if a native floating-point value is within the dynamic range of the sorn configuration
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline bool isInRange(double v) {
		using SORN = sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>;
		SORN a;

		bool inRange = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inRange = false;
		return inRange;
	}

	// transform sorn to a binary representation
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline std::string to_binary(const sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& number, bool nibbleMarker) {
		std::stringstream s;

//		using Real = sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>::Real;
//		auto lowerBound = number.minVal();
//		auto upperBound = number.maxVal();

//		s << "[ " << to_binary(float(lowerBound), nibbleMarker) << ", " << to_binary(float(upperBound), nibbleMarker) << "]";

		return s.str();
	}

#ifdef LATER
	template<typename Real>
	inline std::string color_print(const sorn<Real>& l, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (l.sign() ? "1" : "0");
	
		// integer bits
		for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(rbits); --i) {
			s << cyan << (l.at(static_cast<size_t>(i)) ? '1' : '0');
			if ((i - rbits) > 0 && ((i - rbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// fraction bits
		if constexpr (rbits > 0) {
			s << magenta << '.';
			for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
				s << magenta << (l.at(static_cast<size_t>(i)) ? '1' : '0');
				if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
			}
		}
		s << def;
		return s.str();
	}
#endif

}} // namespace sw::universal

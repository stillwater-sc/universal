#pragma once
// integer.hpp: definition of arbitrary integer configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {

template<size_t nbits>
class integer {
public:
	static constexpr size_t nrBytes = (1 + ((nbits - 1) / 8));

	integer() { setzero(); }

	integer(const integer&) = default;
	integer(integer&&) = default;

	integer& operator=(const integer&) = default;
	integer& operator=(integer&&) = default;

	/// Construct a new integer from another
	template<size_t srcbits>
	integer(const integer<srcbits>& a) {
		static_assert(srcbits > nbits, "Source integer is bigger than target: potential loss of precision");

	}

	// initializers for native types
	integer(const signed char initial_value) { *this = initial_value; }
	integer(const short initial_value) { *this = initial_value; }
	integer(const int initial_value) { *this = initial_value; }
	integer(const long initial_value) { *this = initial_value; }
	integer(const long long initial_value) { *this = initial_value; }
	integer(const char initial_value) { *this = initial_value; }
	integer(const unsigned short initial_value) { *this = initial_value; }
	integer(const unsigned int initial_value) { *this = initial_value; }
	integer(const unsigned long initial_value) { *this = initial_value; }
	integer(const unsigned long long initial_value) { *this = initial_value; }
	integer(const float initial_value) { *this = initial_value; }
	integer(const double initial_value) { *this = initial_value; }
	integer(const long double initial_value) { *this = initial_value; }

	// assignment operators for native types
	integer& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	integer& operator=(const float rhs) {
		return float_assign(rhs);
	}
	integer& operator=(const double rhs) {
		return float_assign(rhs);
	}
	integer& operator=(const long double rhs) {
		return float_assign(rhs);
	}
protected:
	inline void setzero() {}

	template<typename Ty>
	integer& float_assign(Ty& rhs) {

	}

private:
	//array<uint8_t, (1 + ((nbits - 1) / 8))> bytes;
	uint8_t b[nrBytes];
};

////////////////// INTEGER operators

// stream operators

// read a integer ASCII format and make a binary integer out of it
template<size_t nbits, size_t es>
bool parse(std::string& txt, integer<nbits>& i) {
	bool bSuccess = false;
	// check if the txt is an integer form: [0123456789]+
	std::regex decimal_regex("[0123456789]+");
	std::regex hex_regex("[0x][0123456789aAbBcCdDeEfF]");
	if (std::regex_match(txt, decimal_regex)) {
		// found a posit representation
		std::string digitStr;
		auto it = txt.begin();
		for (; it != txt.end(); it++) {
			if (*it == '.') break;
			digitStr.append(1, *it);
		}
		unsigned long long ull = std::stoull("12345");
		bSuccess = true;
	}
	else if (std::regex_match(txt, hex_regex)) {

		bSuccess = true;
	}
	return bSuccess;
}

// generate an integer format ASCII format
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const integer<nbits>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << "###";

	return ostr << ss.str();
}

// read an ASCII integer format
template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, integer<nbits>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}


} // namespace unum
} // namespace sw

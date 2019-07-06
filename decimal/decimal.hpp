#pragma once
// decimal.hpp: definition of arbitrary decimal integer configurations
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <sstream>
#include <vector>
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

// Arbitrary precision decimal number
class decimal : public std::vector<char> {
public:
	decimal() { setzero(); }

	decimal(const decimal&) = default;
	decimal(decimal&&) = default;

	decimal& operator=(const decimal&) = default;
	decimal& operator=(decimal&&) = default;

	// initializers for native types
	decimal(const signed char initial_value) { *this = initial_value; }
	decimal(const short initial_value) { *this = initial_value; }
	decimal(const int initial_value) { *this = initial_value; }
	decimal(const long initial_value) { *this = initial_value; }
	decimal(const long long initial_value) { *this = initial_value; }
	decimal(const char initial_value) { *this = initial_value; }
	decimal(const unsigned short initial_value) { *this = initial_value; }
	decimal(const unsigned int initial_value) { *this = initial_value; }
	decimal(const unsigned long initial_value) { *this = initial_value; }
	decimal(const unsigned long long initial_value) { *this = initial_value; }
	decimal(const float initial_value) { *this = initial_value; }
	decimal(const double initial_value) { *this = initial_value; }
	decimal(const long double initial_value) { *this = initial_value; }

	// assignment operators for native types
	decimal& operator=(const std::string& digits) {
		parse(digits);
		return *this;
	}
	decimal& operator=(const signed char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const char rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned short rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned int rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const unsigned long long rhs) {
		if (0 == rhs) {
			setzero();
			return *this;
		}
		else {
			//convert(v, *this);
		}
		return *this;
	}
	decimal& operator=(const float rhs) {
		return float_assign(rhs);
	}
	decimal& operator=(const double rhs) {
		return float_assign(rhs);
	}
	decimal& operator=(const long double rhs) {
		return float_assign(rhs);
	}

	// selectors
	inline bool isneg() const { return negative; }
	inline bool ispos() const { return !negative; }

	// modifiers
	inline void setzero() { clear(); negative = false; }
	inline void setneg() { negative = true; }
	inline void setpos() { negative = false; }

	// read a decimal ASCII format and make a decimal type out of it
	bool parse(const std::string& digits) {
		bool bSuccess = false;
		// check if the txt is an decimal form: [0123456789]+
		std::regex decimal_regex("[+-]*[0123456789]+");
		if (std::regex_match(digits, decimal_regex)) {
			// found a decimal representation
			setzero();
			auto it = digits.begin();
			bool neg = false;
			if (*it == '-') {
				setneg();
				++it;
			}
			else if (*it == '+') {
				++it;
			}
			for (; it != digits.end(); ++it) {
				push_back(*it);
			}
			std::reverse(begin(), end());
			bSuccess = true;
		}
		return bSuccess;
	}

protected:

	template<typename Ty>
	decimal& float_assign(Ty& rhs) {

	}

private:
	bool negative;

	friend std::ostream& operator<<(std::ostream& ostr, const decimal& d);
	friend std::istream& operator>>(std::istream& istr, decimal& d);
};

////////////////// INTEGER operators

// stream operators


// generate an integer format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, const decimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.isneg()) ss << '-';
	for (int i = (int)d.size() - 1; i >= 0; --i) {
		ss << d[i];
	}
	return ostr << ss.str();
}

// read an ASCII integer format
inline std::istream& operator>> (std::istream& istr, decimal& p) {
	std::string txt;
	istr >> txt;
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
	}
	return istr;
}

} // namespace unum
} // namespace sw

#pragma once
// posito_parse.hpp: parsing a posito in posit format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <universal/number/posito/posito_fwd.hpp>

namespace sw { namespace universal {

// read a posito ASCII format and make a memory posit out of it
template<unsigned nbits, unsigned es>
bool parse(std::string& txt, posito<nbits, es>& p) {
	bool bSuccess = false;
	// Detect nan / inf / infinity tokens (case-insensitive, optional sign)
	// before the regex / stod path; posito has only a single NaR encoding
	// for any non-finite, so all spellings collapse to setnar().
	{
		std::string t;
		t.reserve(txt.size());
		for (char c : txt) {
			t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
		}
		std::string body = t;
		if (!body.empty() && (body.front() == '+' || body.front() == '-')) body.erase(0, 1);
		if (body == "nan" || body == "inf" || body == "infinity") {
			p.setnar();
			return true;
		}
	}
	// check if the txt is of the native posit form: nbits.esXhexvalue
	std::regex posit_regex("[\\d]+\\.[0123456789][xX][\\w]+[p]*");
	if (std::regex_match(txt, posit_regex)) {
		// found a posit representation
		std::string nbitsStr, esStr, bitStr;
		auto it = txt.begin();
		for (; it != txt.end(); it++) {
			if (*it == '.') break;
			nbitsStr.append(1, *it);
		}
		for (it++; it != txt.end(); it++) {
			if (*it == 'x' || *it == 'X') break;
			esStr.append(1, *it);
		}
		for (it++; it != txt.end(); it++) {
			if (*it == 'p') break;
			bitStr.append(1, *it);
		}
		unsigned nbits_in = nbits;
		{
			std::istringstream ss(nbitsStr);
			ss >> nbits_in;
		}
		uint64_t raw;
		std::istringstream ss(bitStr);
		ss >> std::hex >> raw;
		//std::cout << "[" << nbitsStr << "] [" << esStr << "] [" << bitStr << "] = " << raw << std::endl;
		// if not aligned, setbits takes the least significant nbits, so we need to shift to pick up the most significant nbits
		if (nbits < nbits_in) {
			raw >>= (nbits_in - nbits);
		}
		p.setbits(raw);  
		bSuccess = true;
	}
	else {
		// assume it is a float/double/long double representation
		std::istringstream ss(txt);
		double d;
		ss >> d;
		if (ss.fail()) return false;
		ss >> std::ws;
		if (!ss.eof()) return false;
		p = d;
		bSuccess = true;
	}
	return bSuccess;
}

}} // namespace sw::universal

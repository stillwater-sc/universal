#pragma once
// posit_parse.hpp: parsing a posit in posit format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <regex>
#include <sstream>
#include <universal/number/posit/posit_fwd.hpp>

namespace sw { namespace universal {

// read a posit ASCII format and make a memory posit out of it
template<unsigned nbits, unsigned es>
bool parse(std::string& txt, posit<nbits, es>& p) {
	bool bSuccess = false;
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
		p = d;
		bSuccess = true;
	}
	return bSuccess;
}

}} // namespace sw::universal

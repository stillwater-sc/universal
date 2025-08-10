#pragma once
// strmanip.hpp: string manipulation helpers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <locale>
namespace sw { namespace universal {

// remove white space from the left side of the string
std::string& ltrim(std::string& s)
{
	auto it = std::find_if(s.begin(), s.end(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(s.begin(), it);
	return s;
}

// remove white space at the right side of the string
std::string& rtrim(std::string& s) {
	auto it = std::find_if(s.rbegin(), s.rend(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(it.base(), s.end());
	return s;
}

// remove white space on left and right side of the string
inline std::string& trim(std::string& s) { return ltrim(rtrim(s)); }

}} // namespace sw::universal

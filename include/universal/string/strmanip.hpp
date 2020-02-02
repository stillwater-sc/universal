#pragma once
// strmanip.hpp: string manipulation helpers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace unum {

std::string& ltrim(std::string& s)
{
	auto it = std::find_if(s.begin(), s.end(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(s.begin(), it);
	return s;
}

std::string& rtrim(std::string& s)
{
	auto it = std::find_if(s.rbegin(), s.rend(),
		[](char c) {
		return !std::isspace<char>(c, std::locale::classic());
	});
	s.erase(it.base(), s.end());
	return s;
}

std::string& trim(std::string& s)
{
	return ltrim(rtrim(s));
}

} // namespace unum
} // namespace sw

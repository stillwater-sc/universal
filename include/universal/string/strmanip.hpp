#pragma once
// strmanip.hpp: string manipulation helpers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


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

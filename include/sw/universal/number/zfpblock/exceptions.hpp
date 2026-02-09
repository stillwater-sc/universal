#pragma once
// exceptions.hpp: definition of zfpblock exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for zfpblock arithmetic exceptions
struct zfpblock_arithmetic_exception : public universal_arithmetic_exception
{
	explicit zfpblock_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("zfpblock arithmetic exception: ") + error) {};
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for zfpblock internal exceptions
struct zfpblock_internal_exception : public universal_internal_exception
{
	explicit zfpblock_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("zfpblock internal exception: ") + error) {};
};

struct zfpblock_codec_error : public zfpblock_internal_exception {
	explicit zfpblock_codec_error(const std::string& error = "zfpblock codec error")
		: zfpblock_internal_exception(error) {}
};

struct zfpblock_invalid_mode : public zfpblock_internal_exception {
	explicit zfpblock_invalid_mode(const std::string& error = "zfpblock invalid compression mode")
		: zfpblock_internal_exception(error) {}
};

}} // namespace sw::universal

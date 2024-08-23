#pragma once
// string_utils.hpp: utilities to work with std::string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

	std::string centered(const std::string& label, unsigned columnWidth) {
		unsigned length = static_cast<unsigned>(label.length());
		if (columnWidth < length) return label;

		unsigned padding = columnWidth - length;
		unsigned leftPadding = (padding >> 1);
		unsigned rightPadding = padding - leftPadding;
		return std::string(leftPadding, ' ') + label + std::string(rightPadding, ' ');
	}

}} // namespace sw::universal

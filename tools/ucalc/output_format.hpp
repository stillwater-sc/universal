#pragma once
// output_format.hpp: structured output utilities for ucalc
//
// JSON escape, CSV quoting, and JSON-safe number rendering.
// Shared between ucalc.cpp (the REPL) and regression.cpp (tests).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <limits>

namespace sw { namespace ucalc {

// Output format
enum class OutputFormat { plain, json, csv, quiet };

// JSON string escaping
inline std::string json_escape(const std::string& s) {
	std::string out;
	out.reserve(s.size() + 8);
	for (char c : s) {
		switch (c) {
		case '"':  out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n";  break;
		case '\r': out += "\\r";  break;
		case '\t': out += "\\t";  break;
		default:
			if (static_cast<unsigned char>(c) < 0x20) {
				char buf[8];
				std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
				out += buf;
			} else {
				out += c;
			}
		}
	}
	return out;
}

// CSV field quoting: quote if contains comma, quote, or newline
inline std::string csv_quote(const std::string& s) {
	if (s.find_first_of(",\"\n") == std::string::npos) return s;
	std::string out = "\"";
	for (char c : s) {
		if (c == '"') out += "\"\"";
		else out += c;
	}
	out += "\"";
	return out;
}

// JSON-safe number rendering: inf/nan are not valid JSON, emit as strings
inline std::string json_number(double v) {
	if (std::isinf(v)) return v > 0 ? "\"inf\"" : "\"-inf\"";
	if (std::isnan(v)) return "\"nan\"";
	std::ostringstream ss;
	ss << std::setprecision(17) << v;
	return ss.str();
}

}} // namespace sw::ucalc

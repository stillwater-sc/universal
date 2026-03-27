#pragma once
// data_loader.hpp: CSV/text file reader for loading tensor data into ucalc
//
// Reads floating-point values from text files. Supported formats:
// - One value per line
// - Comma-separated values (single or multiple lines)
// - Mixed whitespace/comma delimiters
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace sw { namespace ucalc {

// Load floating-point values from a CSV/text file.
// Handles: one-per-line, comma-separated, mixed whitespace.
// Skips blank lines and lines starting with '#' (comments).
inline std::vector<double> load_csv(const std::string& path) {
	std::ifstream fin(path);
	if (!fin.is_open()) {
		throw std::runtime_error("cannot open file: " + path);
	}
	std::vector<double> data;
	std::string line;
	while (std::getline(fin, line)) {
		// Skip comments and blank lines
		size_t first = line.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) continue;
		if (line[first] == '#') continue;

		// Replace commas with spaces for uniform parsing
		for (auto& c : line) {
			if (c == ',') c = ' ';
		}
		std::istringstream iss(line);
		double v;
		while (iss >> v) {
			data.push_back(v);
		}
	}
	if (data.empty()) {
		throw std::runtime_error("no numeric data found in file: " + path);
	}
	return data;
}

// Parse an inline vector literal: [1.0, -2.5, 3.14]
// Returns the values as doubles.
inline std::vector<double> parse_vector_literal(const std::string& text) {
	std::string s = text;
	// Strip brackets
	size_t start = s.find('[');
	size_t end = s.rfind(']');
	if (start == std::string::npos || end == std::string::npos || end <= start) {
		throw std::runtime_error("expected vector literal [a, b, c, ...], got: " + text);
	}
	s = s.substr(start + 1, end - start - 1);

	// Replace commas with spaces
	for (auto& c : s) {
		if (c == ',') c = ' ';
	}
	std::istringstream iss(s);
	std::vector<double> data;
	double v;
	while (iss >> v) {
		data.push_back(v);
	}
	if (data.empty()) {
		throw std::runtime_error("empty vector literal");
	}
	return data;
}

}} // namespace sw::ucalc

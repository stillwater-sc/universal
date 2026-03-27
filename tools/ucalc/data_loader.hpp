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

// Distinct exception for file-open failures (maps to EXIT_FILE_NOT_FOUND)
struct file_not_found : std::runtime_error {
	using std::runtime_error::runtime_error;
};

// Parse a single token as a double, rejecting partial matches like "1.5xyz".
inline double parse_double_strict(const std::string& token) {
	size_t pos = 0;
	double v = std::stod(token, &pos);
	if (pos != token.size()) {
		throw std::runtime_error("malformed number: '" + token + "'");
	}
	return v;
}

// Split a line by whitespace into tokens (commas pre-replaced with spaces).
inline std::vector<std::string> tokenize_line(const std::string& line) {
	std::vector<std::string> tokens;
	std::istringstream iss(line);
	std::string tok;
	while (iss >> tok) {
		tokens.push_back(tok);
	}
	return tokens;
}

// Load floating-point values from a CSV/text file.
// Handles: one-per-line, comma-separated, mixed whitespace.
// Skips blank lines and lines starting with '#' (comments).
// Throws file_not_found on open failure, runtime_error on parse failure.
inline std::vector<double> load_csv(const std::string& path) {
	std::ifstream fin(path);
	if (!fin.is_open()) {
		throw file_not_found("cannot open file: " + path);
	}
	std::vector<double> data;
	std::string line;
	int line_num = 0;
	while (std::getline(fin, line)) {
		++line_num;
		// Skip comments and blank lines
		size_t first = line.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) continue;
		if (line[first] == '#') continue;

		// Replace commas with spaces for uniform tokenizing
		for (auto& c : line) {
			if (c == ',') c = ' ';
		}
		for (const auto& tok : tokenize_line(line)) {
			try {
				data.push_back(parse_double_strict(tok));
			} catch (const std::exception&) {
				throw std::runtime_error("malformed value '" + tok
				    + "' at line " + std::to_string(line_num) + " in " + path);
			}
		}
	}
	if (data.empty()) {
		throw std::runtime_error("no numeric data found in file: " + path);
	}
	return data;
}

// Parse an inline vector literal: [1.0, -2.5, 3.14]
// Returns the values as doubles. Rejects malformed tokens.
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
	std::vector<double> data;
	for (const auto& tok : tokenize_line(s)) {
		data.push_back(parse_double_strict(tok));
	}
	if (data.empty()) {
		throw std::runtime_error("empty vector literal");
	}
	return data;
}

}} // namespace sw::ucalc

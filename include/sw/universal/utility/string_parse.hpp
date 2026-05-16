#pragma once
// string_parse.hpp: constexpr primitives for parsing value strings into
// Universal number systems.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Phase A of issue #835: a shared, constexpr-friendly foundation for the
// string-parsing APIs that each number system will adopt in later phases.
// -----------------------------------------------------------------------------
//
// Why this exists
// ---------------
// Several Universal number systems (posit, cfloat, integer, fixpnt, ...) ship
// independently-implemented string parsers today, with divergent format
// coverage: lns refuses decimal value strings, fixpnt accepts them, posit
// accepts a custom nbits.esXhexvalue native form, etc. As more types add
// parse(), the divergence compounds.
//
// This header is the shared foundation. It provides constexpr primitives for
// the low-level scanning work -- prefix detection, sign extraction, base-N
// bit-pattern accumulation, decimal-float component scanning -- that every
// number system can build on without re-inventing.
//
// The primitives operate on `std::string_view` only; no allocation, no
// dependencies on the rest of Universal. That makes them callable from
// constexpr ctors (a Phase-B-and-later goal: `constexpr posit<32,2>("3.14")`).
//
// Scope of Phase A
// ----------------
// This file ONLY provides scanning. It does not perform value reconstruction
// (i.e., turning decimal digits into a number type's bit representation) --
// that step depends on the target type's precision and rounding rules and so
// belongs in each type's `parse()`. The Phase A primitives return string_views
// pointing into the original input plus integer summaries; callers iterate.
//
// Conventions
// -----------
//   - All primitives are `constexpr`.
//   - All take `std::string_view`; lifetime is the caller's responsibility.
//   - Returned `string_view` results point into the input string.
//   - "valid" means the parser reached the natural end of input without
//     encountering an invalid character. Partial parses report `digits`
//     consumed so callers can locate the bad character.

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace sw { namespace universal { namespace string_parse {

// ============================================================================
// number_base: which numerical base the input string is in.
// ============================================================================

enum class number_base : std::uint8_t {
	unknown = 0,
	binary  = 2,
	octal   = 8,
	decimal = 10,
	hex     = 16
};

// ============================================================================
// scan_prefix: detect a numerical-base prefix and return the body without it.
// ============================================================================
//
// Recognized prefixes (case-insensitive):
//   0b -> binary,  0o -> octal,  0x -> hex.
//   No prefix -> decimal (body unchanged).
//
// Sign is NOT handled here; callers strip leading +/- with `scan_sign` before
// detecting the base prefix. This keeps the primitives composable.

struct prefix_scan {
	number_base       base;
	std::string_view  body;
};

constexpr prefix_scan scan_prefix(std::string_view s) noexcept {
	if (s.size() >= 2 && s[0] == '0') {
		char c = s[1];
		if (c == 'b' || c == 'B') return {number_base::binary, s.substr(2)};
		if (c == 'o' || c == 'O') return {number_base::octal,  s.substr(2)};
		if (c == 'x' || c == 'X') return {number_base::hex,    s.substr(2)};
	}
	return {number_base::decimal, s};
}

// ============================================================================
// scan_sign: detect a leading '+' or '-'.
// ============================================================================
//
// No sign character is treated as positive. Returns `rest` advanced past
// whatever sign was consumed.

struct sign_scan {
	bool              negative;
	std::string_view  rest;
};

constexpr sign_scan scan_sign(std::string_view s) noexcept {
	if (!s.empty()) {
		if (s.front() == '-') return {true,  s.substr(1)};
		if (s.front() == '+') return {false, s.substr(1)};
	}
	return {false, s};
}

// ============================================================================
// Character classification (constexpr, locale-independent ASCII).
// ============================================================================

constexpr bool is_binary_digit (char c) noexcept { return c == '0' || c == '1'; }
constexpr bool is_octal_digit  (char c) noexcept { return c >= '0' && c <= '7'; }
constexpr bool is_decimal_digit(char c) noexcept { return c >= '0' && c <= '9'; }
constexpr bool is_hex_digit    (char c) noexcept {
	return (c >= '0' && c <= '9')
	    || (c >= 'a' && c <= 'f')
	    || (c >= 'A' && c <= 'F');
}

// Decode a hex digit to its integer value. Returns 0 for non-hex input;
// callers gate with `is_hex_digit` first.
constexpr unsigned hex_digit_value(char c) noexcept {
	if (c >= '0' && c <= '9') return static_cast<unsigned>(c - '0');
	if (c >= 'a' && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return static_cast<unsigned>(c - 'A' + 10);
	return 0u;
}

// ============================================================================
// Bit-pattern parsers: accumulate base-N digits into a uint64_t.
// ============================================================================
//
// All three take an input that is a contiguous run of base-N digits (no
// prefix, no sign, no whitespace). They scan MSB-first (left-to-right) and
// stop at the first non-digit character.
//
// Return fields:
//   value    -- accumulated bits in the low order of a uint64_t
//   digits   -- count of characters consumed
//   overflow -- true if the accumulator would shift past 64 bits
//   valid    -- true iff every input character was a valid base-N digit
//
// For widths exceeding 64 bits, callers stride the input in chunks of
// (64 / log2(base)) digits and shift their own multi-limb accumulator. This
// keeps the primitive simple and avoids any allocation.

struct bit_pattern_result {
	std::uint64_t value;
	std::size_t   digits;
	bool          overflow;
	bool          valid;
};

constexpr bit_pattern_result parse_binary(std::string_view s) noexcept {
	std::uint64_t v = 0;
	std::size_t i = 0;
	bool overflow = false;
	for (; i < s.size(); ++i) {
		char c = s[i];
		if (!is_binary_digit(c)) break;
		if ((v >> 63) != 0u) overflow = true;
		v = (v << 1) | static_cast<std::uint64_t>(c - '0');
	}
	return {v, i, overflow, i == s.size() && i > 0};
}

constexpr bit_pattern_result parse_octal(std::string_view s) noexcept {
	std::uint64_t v = 0;
	std::size_t i = 0;
	bool overflow = false;
	for (; i < s.size(); ++i) {
		char c = s[i];
		if (!is_octal_digit(c)) break;
		if ((v >> 61) != 0u) overflow = true;
		v = (v << 3) | static_cast<std::uint64_t>(c - '0');
	}
	return {v, i, overflow, i == s.size() && i > 0};
}

constexpr bit_pattern_result parse_hex(std::string_view s) noexcept {
	std::uint64_t v = 0;
	std::size_t i = 0;
	bool overflow = false;
	for (; i < s.size(); ++i) {
		char c = s[i];
		if (!is_hex_digit(c)) break;
		if ((v >> 60) != 0u) overflow = true;
		v = (v << 4) | static_cast<std::uint64_t>(hex_digit_value(c));
	}
	return {v, i, overflow, i == s.size() && i > 0};
}

// ============================================================================
// scan_decimal_float: tokenize a decimal floating-point literal.
// ============================================================================
//
// Grammar:
//     [-+]? ( int_part )? ( '.' frac_part )? ( [eE] [-+]? int32 )?
// with at least one digit somewhere in int_part or frac_part.
//
// Returns:
//   valid    -- the whole input parsed cleanly (no trailing garbage)
//   negative -- leading '-' detected
//   int_part -- string_view of integer digits (no sign, no '.', no 'e')
//   frac_part -- string_view of fractional digits (no '.')
//   exp10    -- signed decimal exponent from the [eE] suffix; 0 if absent
//
// The function does NOT perform any base-10 -> bits conversion. Callers
// iterate `int_part` and `frac_part` digit-by-digit, accumulating into
// whatever native or extended-precision representation they target.
//
// The returned views point into the input; lifetime is the caller's.

struct decimal_float_scan {
	bool             valid;
	bool             negative;
	std::string_view int_part;
	std::string_view frac_part;
	std::int32_t     exp10;
};

namespace detail {

// Parse a signed int32 from the start of `s`. The whole input must be a
// well-formed integer; trailing characters cause failure. Used internally
// for the exponent field of scan_decimal_float.
struct int32_parse_result {
	bool        valid;
	std::int32_t value;
};

constexpr int32_parse_result parse_int32(std::string_view s) noexcept {
	if (s.empty()) return {false, 0};
	bool neg = false;
	std::size_t i = 0;
	if (s[0] == '-') { neg = true;  i = 1; }
	else if (s[0] == '+') {          i = 1; }
	if (i >= s.size()) return {false, 0};
	std::int64_t v = 0;
	for (; i < s.size(); ++i) {
		char c = s[i];
		if (!is_decimal_digit(c)) return {false, 0};
		v = v * 10 + static_cast<std::int64_t>(c - '0');
		// Clamp at int32 max; anything bigger is rejected.
		if (v > 2147483647LL) return {false, 0};
	}
	std::int32_t result = static_cast<std::int32_t>(neg ? -v : v);
	return {true, result};
}

}  // namespace detail

constexpr decimal_float_scan scan_decimal_float(std::string_view s) noexcept {
	decimal_float_scan out{false, false, {}, {}, 0};

	auto sg = scan_sign(s);
	out.negative = sg.negative;
	std::string_view body = sg.rest;
	if (body.empty()) return out;

	// Integer part: zero or more digits up to '.' / 'e' / 'E' / end.
	std::size_t i = 0;
	while (i < body.size() && is_decimal_digit(body[i])) ++i;
	out.int_part = body.substr(0, i);

	// Optional fractional part.
	if (i < body.size() && body[i] == '.') {
		++i;
		std::size_t frac_start = i;
		while (i < body.size() && is_decimal_digit(body[i])) ++i;
		out.frac_part = body.substr(frac_start, i - frac_start);
	}

	// Require at least one digit somewhere.
	if (out.int_part.empty() && out.frac_part.empty()) return out;

	// Optional exponent.
	if (i < body.size() && (body[i] == 'e' || body[i] == 'E')) {
		++i;
		auto exp = detail::parse_int32(body.substr(i));
		if (!exp.valid) return out;
		out.exp10 = exp.value;
		i = body.size();
	}

	// Anything left over means malformed input.
	out.valid = (i == body.size());
	return out;
}

}}}  // namespace sw::universal::string_parse

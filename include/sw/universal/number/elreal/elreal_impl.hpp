#pragma once
// elreal_impl.hpp: Exact Lazy Real (McCleeary 2019) -- foundation skeleton (Phase A of epic #873)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// =============================================================================
// elreal: Exact Lazy Real number system
// =============================================================================
//
// Reference:
//   McCleeary, R. (2019). "Lazy Exact Real Arithmetic Using Floating Point
//   Operations." Ph.D. dissertation, University of Iowa.
//   https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771
//
// This phase (Phase A, issue #874) establishes the type skeleton, storage
// representation, and refinement protocol. No arithmetic is implemented here;
// that lands in Phase C (#876). Comparison and sign determination land in
// Phase D (#877). The lazy machinery (closure-based generator) lights up in
// Phase C; this phase exposes the API surface and a single-component
// initialisation path so other code can compile against the type.
//
// -----------------------------------------------------------------------------
// Design decisions for Phase A (settled per the issue's design-questions list)
// -----------------------------------------------------------------------------
//
// 1. STORAGE REPRESENTATION
//
//    The value is stored as a memoized stream of double-precision components
//    plus a generator that produces successive components on demand:
//
//        mutable std::vector<double>               _components;
//        mutable std::function<double(std::size_t)> _generator;   // (Phase C)
//        mutable std::size_t                        _computed_depth = 0;
//
//    The members are `mutable` because refinement happens in const contexts
//    (notably during comparison and sign determination): the *logical* value
//    does not change, only the *representation precision* deepens.
//
//    Why this and not the alternatives:
//      - Closure-based generator is the natural fit for McCleeary's lazy
//        formulation. It re-uses Universal's existing EFT primitives
//        (`error_free_ops.hpp`): each successive component is the EFT
//        residual from the previous level.
//      - The existing design note `docs/multi-component/exact-lazy-arithmetic.md`
//        sketches this same approach.
//      - An explicit DAG of operation nodes (more powerful for sharing common
//        sub-expressions) is a future possibility, but a strictly larger
//        engineering scope. Phase A's API surface is independent of the
//        choice, so a migration is local to the implementation.
//
//    Trade-offs accepted:
//      - `std::function` type-erasure overhead (~one indirect call per
//        refinement step).
//      - Closures capture operands by value, which can be deep for nested
//        expressions; Phase C will introduce a `std::shared_ptr` for operand
//        sharing if measurements warrant it.
//      - The type is not constexpr-friendly. Phase A is not aiming for
//        compile-time elreal.
//
// 2. APPROXIMATION ELEMENT
//
//    Each element of `_components` is a single `double` interpreted as a
//    Shewchuk-style correction term:
//
//        value = _components[0] + _components[1] + _components[2] + ...
//
//    with the non-overlapping property `|_components[i+1]| < ulp(_components[i])`
//    preserved across operations (the EFTs in `error_free_ops.hpp` are designed
//    to produce this).
//
//    Why this and not (lo, hi) intervals or `floatcascade<2>` per element:
//      - Reuses EFT primitives directly; no conversion layer.
//      - Half the storage of an interval representation.
//      - Sign / comparison are still decidable: once
//        `|sum(first N components)| > 2 * ulp(_components[N-1])`, the sign
//        of the truncated sum equals the sign of the true value. The
//        comparison machinery in Phase D refines until this holds, or until
//        the per-call budget is exhausted.
//
// 3. REFINEMENT PROTOCOL
//
//    Two-tier API:
//      - `at(k)` is the primitive: ensure `_components[k]` is materialised
//        (call the generator if `k >= _computed_depth`) and return it.
//      - `refine_to(precision_bits)` is the user-facing wrapper:
//        call `at(0), at(1), ..., at(ceil(precision_bits / 53))` so the
//        prefix of the stream is materialised to at least `precision_bits`
//        bits of cumulative precision.
//
//    The primitive is `at`; everything else is convenience. This shape is
//    independent of the storage choice (the alternative DAG implementation
//    would expose the same `at(k)` entry point).
//
// -----------------------------------------------------------------------------
// What ships in Phase A vs later phases
// -----------------------------------------------------------------------------
//
// Phase A (foundation):
//   - Construct from `double`, `float`, integer types
//   - `at(0)` and `refine_to(precision_bits)` -- entry points in place
//   - `operator double()` returns the sum of materialised components
//
// Phase B (this update, #875):
//   - The closure-based generator slot is now real (mutable std::function).
//     `at(k)` calls the generator on cache miss; `refine_to(p)` iterates
//     `at(0..ceil(p/53))`.
//   - Construction from `p/q` rationals (`elreal(p, q)`). Component 0 is
//     `double(p)/double(q)`. Component 1 is the exact residual via EFTs
//     (two_prod + two_sum). Components 2+ return 0.0 with a documented
//     limitation -- full McCleeary long division for arbitrary depth is
//     deferred to Phase E/F. The acceptance criterion of #875 is that
//     `elreal(1,3).at(1) != 0`, i.e. observably distinct from
//     `elreal(1.0/3.0)`.
//   - Construction from decimal/scientific/rational strings. The parser
//     normalises the input to a rational `(p, q)` form, then delegates to
//     the rational constructor.
//   - Cross-construction from `dd` / `qd` / `ereal<N>` is left as future
//     work (the include-graph cost is non-trivial and these types' Phase B
//     acceptance criterion is not "implicit cross-conversion exists").
//
// Triviality is *not* claimed: the type contains `std::vector` and now also
// `std::function`, neither of which is trivially constructible. Universal's
// library-wide `ReportTrivialityOfType` is reported, not asserted, for
// elastic types -- consistent with `ereal`.
//
// Deferred to later phases:
//   - The four ring operations (Phase C, #876)
//   - Comparison, sign, and the refinement budget policy (Phase D, #877)
//   - Math functions (Phase E, #878)
//   - Cross-construction from dd/qd/ereal (a future PR; #875 marks it optional)
//   - Deep-depth rational refinement (k >= 2) via McCleeary long division
//     (Phase E/F)
//
// =============================================================================

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cctype>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <limits>

#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/numerics/error_free_ops.hpp>

namespace sw { namespace universal {

class elreal {
public:
	// IEEE-754 double precision constants for constructing special values.
	// These mirror ereal's analogous constants and exist so numeric_limits
	// can be specialized without including the entire arithmetic implementation.
	static constexpr int EXP_BIAS         = 1023;
	static constexpr int MAX_EXP          = 1024;
	static constexpr int MIN_EXP_NORMAL   = -1022;

	// ---------------------------------------------------------------------
	// constructors
	// ---------------------------------------------------------------------
	elreal() : _components{}, _computed_depth{0} {
		// canonical zero -- empty stream interpreted as 0
	}

	// Construction from native floating-point types. The stream starts with
	// the input value as its sole component; subsequent refinements (in
	// Phase C onward) extend the stream.
	explicit elreal(double v) : _components{v}, _computed_depth{1} {}
	explicit elreal(float v) : _components{static_cast<double>(v)}, _computed_depth{1} {}

	// Construction from integer types. Integers up to 2^53 are exact in
	// `double`; larger integers are correctly representable only when the
	// rational constructor lands in Phase B. For now, store the rounded
	// double; Phase B will extend the stream with the residual when needed.
	explicit elreal(signed char v)        : elreal(static_cast<double>(v)) {}
	explicit elreal(short v)              : elreal(static_cast<double>(v)) {}
	explicit elreal(int v)                : elreal(static_cast<double>(v)) {}
	explicit elreal(long v)               : elreal(static_cast<double>(v)) {}
	explicit elreal(long long v)          : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned char v)      : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned short v)     : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned int v)       : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned long v)      : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned long long v) : elreal(static_cast<double>(v)) {}

	// Specific-value construction (canonical zero / NaN / inf encodings).
	// Phase A treats NaN / inf as single-component leading values; finer
	// special-value semantics arrive with arithmetic in Phase C.
	explicit elreal(SpecificValue code) : _components{}, _computed_depth{0} {
		switch (code) {
		case SpecificValue::infpos:
			_components.push_back(std::numeric_limits<double>::infinity());
			_computed_depth = 1;
			break;
		case SpecificValue::infneg:
			_components.push_back(-std::numeric_limits<double>::infinity());
			_computed_depth = 1;
			break;
		case SpecificValue::qnan:
			_components.push_back(std::numeric_limits<double>::quiet_NaN());
			_computed_depth = 1;
			break;
		case SpecificValue::snan:
			_components.push_back(std::numeric_limits<double>::signaling_NaN());
			_computed_depth = 1;
			break;
		case SpecificValue::maxpos:
			_components.push_back(std::numeric_limits<double>::max());
			_computed_depth = 1;
			break;
		case SpecificValue::maxneg:
			_components.push_back(std::numeric_limits<double>::lowest());
			_computed_depth = 1;
			break;
		case SpecificValue::minpos:
			_components.push_back(std::numeric_limits<double>::min());
			_computed_depth = 1;
			break;
		case SpecificValue::minneg:
			_components.push_back(-std::numeric_limits<double>::min());
			_computed_depth = 1;
			break;
		case SpecificValue::zero:
		default:
			// empty stream == 0
			break;
		}
	}

	// Rational construction: elreal(p, q) represents the exact value p/q.
	// Component 0 is `double(p)/double(q)` (the IEEE-754 rounded division).
	// Component 1 is the exact residual `p/q - component_0` computed via
	// two_prod + two_sum -- this is exact when `p` fits in a double (i.e.
	// `|p| < 2^53`, which is true for any `long long` whose absolute value
	// is < 2^53; for larger magnitudes the residual computation incurs one
	// extra rounding error, documented in Phase E follow-up).
	// Components 2 and beyond return 0.0 in Phase B; full McCleeary
	// long-division refinement to arbitrary depth lands in Phase E/F.
	elreal(long long p, long long q)
		: _components{}, _computed_depth{0}, _generator{}
	{
		if (q == 0) {
			_components.push_back(std::numeric_limits<double>::quiet_NaN());
			_computed_depth = 1;
			return;
		}
		if (p == 0) {
			// canonical zero; leave _components empty
			return;
		}

		double c0 = static_cast<double>(p) / static_cast<double>(q);
		_components.push_back(c0);
		_computed_depth = 1;

		// Generator: produces the k-th correction component on demand.
		// k == 1 -> exact residual via EFTs.
		// k >= 2 -> 0.0 (Phase B limitation; documented above).
		long long p_cap = p;
		long long q_cap = q;
		double    c0_cap = c0;
		_generator = [p_cap, q_cap, c0_cap](std::size_t k) -> double {
			if (k != 1) return 0.0;
			// Compute the residual p - c0 * q exactly using EFTs:
			//   two_prod(c0, q)        -> (prod_hi, prod_err) with c0*q = prod_hi + prod_err
			//   two_sum(p, -prod_hi)   -> (s1,   s1_err)
			//   two_sum(s1, -prod_err) -> (s2,   s2_err)
			//   residual = s2 + s1_err + s2_err  (sum of error terms collapsed to a
			//                                     double; further bits are below
			//                                     double precision in this scope)
			double prod_err;
			double prod_hi  = two_prod(c0_cap, static_cast<double>(q_cap), prod_err);
			double s1_err;
			double s1       = two_sum(static_cast<double>(p_cap), -prod_hi, s1_err);
			double s2_err;
			double s2       = two_sum(s1, -prod_err, s2_err);
			double residual = s2 + s1_err + s2_err;
			return residual / static_cast<double>(q_cap);
		};
	}

	// String construction: parses decimal ("3.14"), scientific ("6.022e23"),
	// rational ("1/3"), and the special tokens "nan" / "inf" / "infinity"
	// (case-insensitive). The parse normalises to a rational (p, q) pair
	// and delegates to the rational constructor. Returns the canonical zero
	// on parse failure (silent-failure pattern matching the dfloat and
	// ereal string constructors); see parse() below for the bool-returning
	// variant.
	explicit elreal(const std::string& str)
		: _components{}, _computed_depth{0}, _generator{}
	{
		if (!parse_into(*this, str)) {
			// parse failed -- leave at canonical zero
			_components.clear();
			_computed_depth = 0;
			_generator      = {};
		}
	}

	// rule-of-five: defaulted, defensible for a vector-holding type
	elreal(const elreal&)            = default;
	elreal(elreal&&) noexcept        = default;
	elreal& operator=(const elreal&) = default;
	elreal& operator=(elreal&&) noexcept = default;
	~elreal()                        = default;

	// ---------------------------------------------------------------------
	// refinement protocol
	// ---------------------------------------------------------------------
	//
	// at(k):           the primitive. Returns the k-th component of the
	//                  lazy stream, generating it if necessary. In Phase A
	//                  the generator is absent, so k >= _computed_depth
	//                  returns 0.0 (the implicit trailing extension).
	//
	// refine_to(p):    the user-facing wrapper. Ensures the stream has at
	//                  least ceil(p / 53) components materialised. No-op
	//                  in Phase A.

	double at(std::size_t k) const {
		// Materialise components up to and including index k by calling the
		// generator on cache misses. When no generator is installed (e.g. for
		// values constructed from a single double), the stream is treated as
		// terminating at _computed_depth and at(k>=depth) returns 0.0.
		while (_computed_depth <= k && _generator) {
			double next = _generator(_computed_depth);
			_components.push_back(next);
			++_computed_depth;
		}
		if (k < _computed_depth) return _components[k];
		return 0.0;
	}

	void refine_to(std::size_t precision_bits) const {
		// Each component carries ~53 bits of precision (one IEEE-754 double
		// significand), so we want ceil(precision_bits / 53) components
		// materialised. Force at least one component for any positive request.
		if (precision_bits == 0) return;
		std::size_t target = (precision_bits + 52) / 53;
		// Touching at(target - 1) walks the generator up to that depth.
		(void)at(target - 1);
	}

	std::size_t computed_depth() const noexcept { return _computed_depth; }

	// ---------------------------------------------------------------------
	// selectors
	// ---------------------------------------------------------------------

	// Sum of the currently-computed components -- the best estimate of the
	// true value at the current refinement depth. Starts the sum at
	// _components[0] (rather than literal 0.0) so that single-component
	// -0.0 round-trips with its IEEE sign bit preserved.
	explicit operator double() const noexcept {
		if (_computed_depth == 0) return 0.0;
		double s = _components[0];
		for (std::size_t i = 1; i < _computed_depth; ++i) s += _components[i];
		return s;
	}

	// Component access for tests and inspection. Returns a copy; the
	// underlying vector is not mutable through this accessor.
	const std::vector<double>& components() const noexcept { return _components; }

	bool iszero() const noexcept { return _computed_depth == 0 || double(*this) == 0.0; }

	bool isnan() const noexcept {
		for (std::size_t i = 0; i < _computed_depth; ++i) {
			if (std::isnan(_components[i])) return true;
		}
		return false;
	}

	bool isinf() const noexcept {
		for (std::size_t i = 0; i < _computed_depth; ++i) {
			if (std::isinf(_components[i])) return true;
		}
		return false;
	}

private:
	// The lazy stream of components. Mutable because refinement is invoked
	// in const contexts (comparison, decode-to-double).
	mutable std::vector<double> _components;

	// The high-water mark of materialised components. Phase A always has
	// _computed_depth == _components.size(); Phase C may pre-allocate
	// _components and grow _computed_depth as the generator fires.
	mutable std::size_t _computed_depth;

	// Generator slot. Empty by default; the rational constructor (and, in
	// Phase C, the arithmetic operators) install a closure here. When the
	// generator is empty, at(k) for k >= _computed_depth returns 0.0
	// (the implicit-zero extension that matches the leading-component-only
	// representation of a constructed-from-double value).
	mutable std::function<double(std::size_t)> _generator;

	// String parser. Lives as a friend free function so it can populate
	// the private state. Public-facing wrappers are the std::string
	// constructor and the namespace-level parse(str, elreal&) declared
	// in elreal_fwd.hpp.
	friend bool parse_into(elreal& out, const std::string& str);
};

// =============================================================================
// String parser
// =============================================================================
//
// Accepts:
//   "123", "-42", "+42"           -- decimal integer
//   "3.14", "-1.25"               -- decimal fraction (normalised to p/q)
//   "6.022e23", "1.5E-10"         -- scientific (normalised to p/q with q a
//                                   power of 10)
//   "1/3", "-22/7"                -- rational
//   "nan", "inf", "-infinity"     -- special tokens (case-insensitive)
//
// Returns true on success, false on parse error. On error, `out` is set to
// canonical zero.
//
// Limitation: the integer accumulators are `long long`, so mantissa digit
// counts beyond ~18 or scientific exponents that produce a denominator
// requiring > 18 digits overflow silently and return false. The phase-B
// acceptance does not include arbitrary-precision string parsing; the
// canonical use cases (representing 1/3, 22/7, pi as a finite decimal, etc.)
// all fit comfortably.

inline bool parse_into(elreal& out, const std::string& str) {
	out._components.clear();
	out._computed_depth = 0;
	out._generator      = {};

	if (str.empty()) return false;
	std::size_t pos = 0;

	// Skip whitespace
	while (pos < str.length() && std::isspace(static_cast<unsigned char>(str[pos]))) ++pos;
	if (pos >= str.length()) return false;

	// Sign
	bool negative = false;
	if (str[pos] == '+' || str[pos] == '-') {
		negative = (str[pos] == '-');
		++pos;
	}

	// nan / inf / infinity tokens
	{
		std::string lower;
		for (std::size_t q = pos; q < str.length(); ++q) {
			lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(str[q]))));
		}
		if (lower == "nan") {
			out._components.push_back(std::numeric_limits<double>::quiet_NaN());
			out._computed_depth = 1;
			return true;
		}
		if (lower == "inf" || lower == "infinity") {
			out._components.push_back(negative
				? -std::numeric_limits<double>::infinity()
				:  std::numeric_limits<double>::infinity());
			out._computed_depth = 1;
			return true;
		}
	}

	// Rational form: look for a '/' separator (no decimal / scientific
	// mixing supported -- rational form is its own track).
	std::size_t slash = str.find('/', pos);
	if (slash != std::string::npos) {
		long long num = 0, den = 0;
		bool found_num = false;
		for (std::size_t i = pos; i < slash; ++i) {
			char c = str[i];
			if (!std::isdigit(static_cast<unsigned char>(c))) return false;
			num = num * 10 + (c - '0');
			found_num = true;
		}
		if (!found_num) return false;
		bool found_den = false;
		for (std::size_t i = slash + 1; i < str.length(); ++i) {
			char c = str[i];
			if (!std::isdigit(static_cast<unsigned char>(c))) return false;
			den = den * 10 + (c - '0');
			found_den = true;
		}
		if (!found_den || den == 0) return false;
		out = elreal(negative ? -num : num, den);
		return true;
	}

	// Decimal / scientific form
	long long mantissa = 0;
	bool found_digit = false;
	bool seen_dot = false;
	int frac_digits = 0;
	while (pos < str.length()) {
		char c = str[pos];
		if (std::isdigit(static_cast<unsigned char>(c))) {
			mantissa = mantissa * 10 + (c - '0');
			if (seen_dot) ++frac_digits;
			found_digit = true;
		}
		else if (c == '.' && !seen_dot) {
			seen_dot = true;
		}
		else if (c == 'e' || c == 'E') {
			++pos;
			break;
		}
		else {
			return false;
		}
		++pos;
	}
	if (!found_digit) return false;

	int exponent = 0;
	bool exp_negative = false;
	if (pos < str.length()) {
		if (str[pos] == '+' || str[pos] == '-') {
			exp_negative = (str[pos] == '-');
			++pos;
		}
		bool found_exp_digit = false;
		while (pos < str.length()) {
			char c = str[pos];
			if (!std::isdigit(static_cast<unsigned char>(c))) return false;
			exponent = exponent * 10 + (c - '0');
			found_exp_digit = true;
			++pos;
		}
		if (!found_exp_digit) return false;
		if (exp_negative) exponent = -exponent;
	}

	// Normalize to (p, q) where p = signed mantissa, q = 10^(frac_digits - exponent).
	// If either integer would overflow long long (typical for scientific
	// notation like "6.022e23"), fall back to a double-precision construction
	// that captures the leading 53 bits accurately. The exact-rational
	// refinement at depth > 0 is then unavailable for that value -- a
	// documented Phase B limitation. The integer-fit case (any decimal whose
	// numerator and denominator individually fit in 18-19 digits) gets
	// full rational refinement.
	int q_pow = frac_digits - exponent;
	long long p = negative ? -mantissa : mantissa;
	long long q = 1;
	bool overflowed = false;
	if (q_pow > 0) {
		for (int i = 0; i < q_pow && !overflowed; ++i) {
			if (q > std::numeric_limits<long long>::max() / 10) overflowed = true;
			else q *= 10;
		}
	}
	else if (q_pow < 0) {
		for (int i = 0; i < -q_pow && !overflowed; ++i) {
			long long pmag = p < 0 ? -p : p;
			if (pmag > std::numeric_limits<long long>::max() / 10) overflowed = true;
			else p *= 10;
		}
	}

	if (overflowed) {
		// Reconstruct via double * 10^q_pow_signed. Loses exact-rational
		// refinement but keeps the leading component faithful.
		double mag = static_cast<double>(mantissa);
		double scaled = mag * std::pow(10.0, static_cast<double>(-q_pow));
		out = elreal(negative ? -scaled : scaled);
		return true;
	}

	out = elreal(p, q);
	return true;
}

// Namespace-level parse() (matches dfloat / ereal convention)
inline bool parse(const std::string& str, elreal& out) {
	return parse_into(out, str);
}

}} // namespace sw::universal

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
//        mutable lazy_component_buffer  _components;
//             (4-double inline + spill via std::vector; Phase K.1 of #905)
//        mutable lazy_generator         _generator;
//             (std::variant of small POD shapes; Phase K.2 of #905)
//        mutable std::size_t            _computed_depth = 0;
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
//      - Phase K.2 (#905) replaced std::function with a `std::variant` of
//        small POD shapes. `std::visit` dispatch lets the compiler inline
//        per-shape evaluators. Operand captures are `std::shared_ptr<const
//        elreal>` (16 bytes), so per-op state stays small.
//      - Atomic refcount bumps when sharing operands. Cheap on modern
//        x86 (a few cycles per make_shared / copy) but still a cost on
//        the common path.
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
// Phase B (#875, merged):
//   - The generator slot is functional (Phase K.2 #905 replaced the
//     original std::function with a std::variant of small POD shapes).
//   - Construction from `p/q` rationals (`elreal(p, q)`).
//   - Construction from decimal/scientific/rational strings.
//
// Phase C (#876, merged):
//   - Ring operations +, -, *, / with depth-1 EFT refinement on +/-/*.
//   - Unary -, abs/fabs, compound assignment +=, -=, *=, /=.
//
// Phase D (this update, #877):
//   - Sign determination via refinement walk: `sign(v, budget)` materialises
//     components until a non-zero one is found (its sign is the value's
//     sign by the non-overlapping property of the expansion) or until
//     `budget` components have been materialised (treat as zero).
//   - `compare(a, b, budget)` returns `sign(a - b, budget)`.
//   - Ordering operators `<`, `<=`, `>`, `>=`, `==`, `!=` use `compare`
//     with a default budget. NaN handling matches IEEE-754: all ordering
//     comparisons return false on NaN operands, `!=` returns true.
//   - The undecidable case (a value mathematically equal to zero, or two
//     values mathematically equal to each other) collapses to "indistinguishable
//     within the supplied budget." Callers needing stronger guarantees raise
//     the budget; there is no global state.
//   - Refinement budget is a per-call argument with a sensible default
//     (`elreal_default_budget = 8` components, ~424 bits cumulative).
//
// Triviality is *not* claimed: the type contains `lazy_component_buffer`
// (with a `std::vector` for spill) and a `std::variant` whose
// alternatives hold `std::shared_ptr<const elreal>`, neither of which
// is trivially constructible. Universal's library-wide
// `ReportTrivialityOfType` is reported, not asserted, for elastic types
// -- consistent with `ereal`.
//
// Deferred to later phases:
//   - Math functions (Phase E, #878)
//   - Cross-construction from dd/qd/ereal (future PR; #875 marks it optional)
//   - Deep-depth refinement (k >= 2) for both rationals and arithmetic
//     results (Phase E/F via McCleeary expansion arithmetic)
//   - Division at depth 1 via Newton-Raphson (Phase E/F)
//
// =============================================================================

#include <cstddef>
#include <cstdint>
#include <cstdlib>   // std::strtod
#include <cerrno>    // errno, ERANGE
#include <cmath>
#include <cctype>
#include <numbers>   // std::numbers::ln2_v, ln10_v (Phase E.3)
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <limits>

#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/lazy_component_buffer.hpp>
#include <universal/number/elreal/elreal_data.hpp>
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
		// k == 1 -> exact residual via EFTs (see gen_rational_residual
		//          evaluator in elreal_data.hpp).
		// k >= 2 -> 0.0 (Phase B limitation; documented above).
		_generator = gen_rational_residual{
			static_cast<double>(p),
			static_cast<double>(q),
			c0
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
		// Materialise components up to and including index k by walking
		// the generator on cache misses. When no generator is installed
		// (std::monostate -- the case for single-double constructed values),
		// the stream is treated as terminating at _computed_depth and
		// at(k >= depth) returns 0.0.
		if (k < _computed_depth) return _components[k];
		if (std::holds_alternative<std::monostate>(_generator)) return 0.0;
		while (_computed_depth <= k) {
			double next = evaluate_generator(_generator, _computed_depth);
			_components.push_back(next);
			++_computed_depth;
		}
		return _components[k];
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
	// from_expansion: construct an elreal from a pre-computed non-overlapping
	// component expansion. Used by the math-constants accessors (elreal_pi,
	// elreal_e, ...) to materialise multi-component values whose corrections
	// were computed offline at high precision. The caller is responsible for
	// ensuring the components satisfy the Shewchuk non-overlapping property
	// (|c_{i+1}| <= ulp(c_i) / 2) and decreasing magnitude order; arithmetic
	// and comparison on the resulting elreal assume both properties hold.
	//
	// No generator is installed: at(k) returns 0.0 for k >= components.size(),
	// matching the behaviour of a single-double constructed value. Deeper
	// refinement past the supplied expansion is deferred to a future
	// enhancement (a generator-based variant of this factory, or per-constant
	// algorithms that pull more bits on demand).
	static elreal from_expansion(std::initializer_list<double> components) {
		elreal r;
		r._components.reserve(components.size());
		for (double c : components) r._components.push_back(c);
		r._computed_depth = r._components.size();
		return r;
	}

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

	// Component access for tests and inspection. The underlying buffer
	// is not mutable through this accessor. Phase K.1 (#905) replaced
	// the storage from std::vector<double> with lazy_component_buffer;
	// the buffer exposes size() and operator[] only, which is what all
	// known callers use.
	const lazy_component_buffer& components() const noexcept { return _components; }

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

	bool isneg() const noexcept {
		// Sign of the materialised sum. For empty streams (canonical zero)
		// the answer is "not negative." For a single -0.0 leading component
		// this honors the IEEE-754 sign bit and returns true.
		if (_computed_depth == 0) return false;
		return std::signbit(double(*this));
	}

	// ---------------------------------------------------------------------
	// arithmetic: unary minus, abs, and compound assignment
	// ---------------------------------------------------------------------
	//
	// Binary +, -, *, / live as friend free functions after the class so
	// they have public-form access while still touching the private storage
	// via friendship. The compound assignment forms below delegate to the
	// binary free functions.

	// Unary minus: negate every materialised component and wrap the
	// generator in a sign-flipping trampoline (gen_unary_neg).
	elreal operator-() const {
		elreal result;
		result._components.reserve(_components.size());
		for (std::size_t i = 0; i < _components.size(); ++i) {
			result._components.push_back(-_components[i]);
		}
		result._computed_depth = _computed_depth;
		if (!std::holds_alternative<std::monostate>(_generator)) {
			result._generator = gen_unary_neg{ std::make_shared<const elreal>(*this) };
		}
		return result;
	}

	elreal& operator+=(const elreal& rhs);
	elreal& operator-=(const elreal& rhs);
	elreal& operator*=(const elreal& rhs);
	elreal& operator/=(const elreal& rhs);

	// Friend so the variant evaluator (defined after the class) can
	// invoke at(k) on the operand handles.
	friend double evaluate_generator(const lazy_generator& g, std::size_t k);

private:
	// The lazy stream of components. Mutable because refinement is invoked
	// in const contexts (comparison, decode-to-double).
	mutable lazy_component_buffer _components;

	// The high-water mark of materialised components. Phase A always has
	// _computed_depth == _components.size(); Phase C may pre-allocate
	// _components and grow _computed_depth as the generator fires.
	mutable std::size_t _computed_depth;

	// Generator slot. Empty by default (std::monostate); operators
	// install a tagged-union alternative here (see elreal_data.hpp).
	// When the generator is monostate, at(k) for k >= _computed_depth
	// returns 0.0 (the implicit-zero extension).
	//
	// Phase K.2 (#905): replaced std::function with std::variant. Operand
	// captures are now shared_ptr<const elreal> (16 bytes each) rather
	// than elreal-by-value (~104 bytes each), eliminating the
	// std::function heap allocation for per-op generator state.
	mutable lazy_generator _generator;

	// String parser. Lives as a friend free function so it can populate
	// the private state. Public-facing wrappers are the std::string
	// constructor and the namespace-level parse(str, elreal&) declared
	// in elreal_fwd.hpp.
	friend bool parse_into(elreal& out, const std::string& str);

	// Binary arithmetic ops live as friend free functions so they can
	// construct the result while touching the private generator slot.
	friend elreal operator+(const elreal& a, const elreal& b);
	friend elreal operator-(const elreal& a, const elreal& b);
	friend elreal operator*(const elreal& a, const elreal& b);
	friend elreal operator/(const elreal& a, const elreal& b);
	friend elreal abs(const elreal& a);

	// Phase E math functions (#878). Same friendship rationale.
	friend elreal sqrt(const elreal& a);
	friend elreal hypot(const elreal& a, const elreal& b);

	// Phase E.3 (#889): exp / log / pow family.
	friend elreal exp(const elreal& a);
	friend elreal exp2(const elreal& a);
	friend elreal expm1(const elreal& a);
	friend elreal log(const elreal& a);
	friend elreal log2(const elreal& a);
	friend elreal log10(const elreal& a);
	friend elreal log1p(const elreal& a);
	friend elreal pow(const elreal& a, const elreal& b);

	// Phase E.4 (#890): hyperbolic functions.
	friend elreal sinh(const elreal& a);
	friend elreal cosh(const elreal& a);
	friend elreal tanh(const elreal& a);
	friend elreal asinh(const elreal& a);
	friend elreal acosh(const elreal& a);
	friend elreal atanh(const elreal& a);

	// Phase E.5 (#891): inverse trigonometric functions.
	friend elreal asin(const elreal& a);
	friend elreal acos(const elreal& a);
	friend elreal atan(const elreal& a);
	friend elreal atan2(const elreal& y, const elreal& x);

	// Phase E.6 (#892): forward trigonometric functions.
	friend elreal sin(const elreal& a);
	friend elreal cos(const elreal& a);
	friend elreal tan(const elreal& a);
};

// =============================================================================
// Generator evaluator (Phase K.2, #905)
// =============================================================================
//
// Dispatches to the per-shape formula for the requested component index k.
// Most generators return 0.0 for k != 1 (depth-1 cap); the unary_neg
// trampoline forwards to its wrapped operand for arbitrary k.
//
// Defined after the elreal class because each variant alternative
// invokes elreal::at() via its operand handle.

inline double evaluate_generator(const lazy_generator& g, std::size_t k) {
	return std::visit([k](const auto& alt) -> double {
		using T = std::decay_t<decltype(alt)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return 0.0;
		}
		else if constexpr (std::is_same_v<T, gen_unary_linear>) {
			if (k != 1) return 0.0;
			return alt.coeff * alt.a->at(1);
		}
		else if constexpr (std::is_same_v<T, gen_binary_linear>) {
			if (k != 1) return 0.0;
			return alt.c0 + alt.ca * alt.a->at(1) + alt.cb * alt.b->at(1);
		}
		else if constexpr (std::is_same_v<T, gen_sqrt>) {
			if (k != 1) return 0.0;
			double prod_err = 0.0;
			double prod_hi = two_prod(alt.c0, alt.c0, prod_err);
			double num = (alt.a->at(alt.lead_idx) - prod_hi) - prod_err
			           + alt.a->at(alt.lead_idx + 1);
			return num / (2.0 * alt.c0);
		}
		else if constexpr (std::is_same_v<T, gen_unary_neg>) {
			return -alt.wrapped->at(k);
		}
		else if constexpr (std::is_same_v<T, gen_rational_residual>) {
			if (k != 1) return 0.0;
			// Matches the original rational ctor's EFT walk:
			//   two_prod(c0, q) -> (prod_hi, prod_err)
			//   two_sum(p, -prod_hi) -> (s1, s1_err)
			//   two_sum(s1, -prod_err) -> (s2, s2_err)
			//   residual = s2 + s1_err + s2_err
			//   return residual / q
			double prod_err = 0.0;
			double prod_hi = two_prod(alt.c0, alt.q, prod_err);
			double s1_err = 0.0;
			double s1 = two_sum(alt.p, -prod_hi, s1_err);
			double s2_err = 0.0;
			double s2 = two_sum(s1, -prod_err, s2_err);
			double residual = s2 + s1_err + s2_err;
			return residual / alt.q;
		}
	}, g);
}

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
			int digit = c - '0';
			// Reject before signed overflow: `num * 10 + digit > LLONG_MAX` ?
			if (num > (std::numeric_limits<long long>::max() - digit) / 10) return false;
			num = num * 10 + digit;
			found_num = true;
		}
		if (!found_num) return false;
		bool found_den = false;
		for (std::size_t i = slash + 1; i < str.length(); ++i) {
			char c = str[i];
			if (!std::isdigit(static_cast<unsigned char>(c))) return false;
			int digit = c - '0';
			if (den > (std::numeric_limits<long long>::max() - digit) / 10) return false;
			den = den * 10 + digit;
			found_den = true;
		}
		if (!found_den || den == 0) return false;
		// Signed zero: "-0/q" preserves the negative sign through the parse
		// even though zero has no sign mathematically; matches the elreal(-0.0)
		// round-trip behavior so the IEEE-754 sign bit is consistent across
		// ctor paths.
		if (num == 0 && negative) {
			out._components.clear();
			out._components.push_back(-0.0);
			out._computed_depth = 1;
			out._generator      = {};
			return true;
		}
		out = elreal(negative ? -num : num, den);
		return true;
	}

	// Decimal / scientific form
	long long mantissa = 0;
	bool found_digit = false;
	bool seen_dot = false;
	bool mantissa_overflow = false;
	int frac_digits = 0;
	while (pos < str.length()) {
		char c = str[pos];
		if (std::isdigit(static_cast<unsigned char>(c))) {
			int digit = c - '0';
			if (!mantissa_overflow) {
				if (mantissa > (std::numeric_limits<long long>::max() - digit) / 10) {
					// Stop updating mantissa once it would overflow, but keep
					// scanning to validate the rest of the literal. The
					// overflow fallback below will produce a faithful leading
					// double via std::stod() on the full input string.
					mantissa_overflow = true;
				}
				else {
					mantissa = mantissa * 10 + digit;
				}
			}
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
	bool exponent_overflow = false;
	if (pos < str.length()) {
		if (str[pos] == '+' || str[pos] == '-') {
			exp_negative = (str[pos] == '-');
			++pos;
		}
		bool found_exp_digit = false;
		while (pos < str.length()) {
			char c = str[pos];
			if (!std::isdigit(static_cast<unsigned char>(c))) return false;
			int digit = c - '0';
			if (!exponent_overflow) {
				if (exponent > (std::numeric_limits<int>::max() - digit) / 10) {
					// Stop updating exponent to avoid signed-int overflow UB;
					// keep scanning to validate the rest of the literal. The
					// mantissa_overflow fallback below also handles this case
					// via std::stod() on the full original string.
					exponent_overflow = true;
				}
				else {
					exponent = exponent * 10 + digit;
				}
			}
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
			// Compute |p| via unsigned arithmetic so LLONG_MIN does not
			// trigger signed-negation UB. With the mantissa overflow guard
			// above, p cannot actually reach LLONG_MIN at this point, but
			// keeping the unsigned form is defensive coding -- a future
			// caller of this constructor with a hand-rolled value could.
			unsigned long long pmag = p < 0
				? static_cast<unsigned long long>(-(p + 1)) + 1u
				: static_cast<unsigned long long>(p);
			constexpr unsigned long long limit =
				static_cast<unsigned long long>(std::numeric_limits<long long>::max()) / 10u;
			if (pmag > limit) overflowed = true;
			else p *= 10;
		}
	}

	if (mantissa_overflow || exponent_overflow) {
		// The mantissa or exponent portion of the literal exceeded its
		// integer range. Use std::strtod (rather than std::stod) on the
		// full original string to get the correctly-rounded leading double.
		// We prefer strtod because it sets errno = ERANGE and returns
		// +/-HUGE_VAL (overflow) or 0.0 (underflow) instead of throwing,
		// which matches IEEE-754 saturation semantics -- the right behavior
		// for inputs the rational track cannot represent anyway.
		// Exact-rational refinement at depth > 0 is unavailable for this
		// path; only the leading double is materialised.
		const char* c_str = str.c_str();
		char* endp = nullptr;
		errno = 0;
		double v = std::strtod(c_str, &endp);
		if (endp == c_str) return false;  // no conversion happened
		// errno == ERANGE is OK -- strtod has already set v to +/-HUGE_VAL or 0.
		out = elreal(v);
		return true;
	}

	if (overflowed) {
		// q_pow scaling overflowed long long even though the mantissa fit.
		// Reconstruct via double * 10^(-q_pow). Loses exact-rational
		// refinement but keeps the leading component faithful.
		double mag = static_cast<double>(mantissa);
		double scaled = mag * std::pow(10.0, static_cast<double>(-q_pow));
		out = elreal(negative ? -scaled : scaled);
		return true;
	}

	// Signed zero preservation: "-0", "-0.0", "-0e5", etc. should produce a
	// negative zero (matching the elreal(-0.0) round-trip). The general
	// rational ctor would collapse this to canonical (sign-less) zero.
	if (p == 0 && negative) {
		out._components.clear();
		out._components.push_back(-0.0);
		out._computed_depth = 1;
		out._generator      = {};
		return true;
	}

	out = elreal(p, q);
	return true;
}

// Namespace-level parse() (matches dfloat / ereal convention)
inline bool parse(const std::string& str, elreal& out) {
	return parse_into(out, str);
}

// =============================================================================
// Arithmetic operators (Phase C)
// =============================================================================
//
// Each binary operator returns a new elreal:
//   - Component 0 is the leading-double result of the operation, which
//     handles NaN / infinity / signed-zero propagation per IEEE-754
//     automatically.
//   - Component 1 (for +, -, *) is the exact residual from the operation
//     on the leading components, captured via EFTs (two_sum / two_diff /
//     two_prod from `numerics/error_free_ops.hpp`), plus depth-1
//     corrections from each operand's stream.
//   - Components 2+ return 0.0 with a documented Phase C limitation;
//     full McCleeary expansion-arithmetic refinement lands in Phase E/F.
//   - Division at depth 1 is deferred to Phase E/F (Newton-Raphson on
//     the reciprocal stream); the depth-0 leading-double result is
//     correct, but the depth-1 correction returns 0.0.

inline elreal operator+(const elreal& a, const elreal& b) {
	double a0 = a.at(0);
	double b0 = b.at(0);
	double sum_err = 0.0;
	double c0 = two_sum(a0, b0, sum_err);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// If the leading result is not finite, skip refinement: the EFT
	// residual is already zero (two_sum's contract) and deeper bits
	// don't make sense for inf / NaN.
	if (!std::isfinite(c0)) return result;

	// Generator: depth 1 = sum_err + a.at(1) + b.at(1). Depth >= 2 returns 0.
	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(a),
		std::make_shared<const elreal>(b),
		sum_err, 1.0, 1.0
	};
	return result;
}

inline elreal operator-(const elreal& a, const elreal& b) {
	double a0 = a.at(0);
	double b0 = b.at(0);
	double diff_err = 0.0;
	double c0 = two_diff(a0, b0, diff_err);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// Depth 1 = diff_err + a.at(1) - b.at(1).
	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(a),
		std::make_shared<const elreal>(b),
		diff_err, 1.0, -1.0
	};
	return result;
}

inline elreal operator*(const elreal& a, const elreal& b) {
	double a0 = a.at(0);
	double b0 = b.at(0);
	double prod_err = 0.0;
	double c0 = two_prod(a0, b0, prod_err);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// Depth 1: (a0*b0 - c0) is captured exactly by prod_err.
	// The leading correction to a*b is:
	//   prod_err + b0 * a.at(1) + a0 * b.at(1)
	// (the a.at(1)*b.at(1) cross-term is O(eps^2) and below the depth-1
	//  precision target; deferred to deeper refinement in Phase E/F).
	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(a),
		std::make_shared<const elreal>(b),
		prod_err, b0, a0
	};
	return result;
}

inline elreal operator/(const elreal& a, const elreal& b) {
	double b0 = b.at(0);
	if (b0 == 0.0) {
#if ELREAL_THROW_ARITHMETIC_EXCEPTION
		throw elreal_divide_by_zero();
#else
		// Fall through: native double divide yields +/-inf or NaN per
		// IEEE-754. The user can disambiguate via isinf() / isnan() at the
		// call site; or they can opt into exception throwing by defining
		// ELREAL_THROW_ARITHMETIC_EXCEPTION before including elreal.hpp.
#endif
	}
	double a0 = a.at(0);
	double c0 = a0 / b0;

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// No depth-1 refinement when the leading is non-finite. Also skip when
	// b0 was zero (handled above) -- the depth-1 formula divides by b0.
	if (!std::isfinite(c0) || b0 == 0.0) return result;

	// Phase L.1: depth-1 refinement via Taylor expansion + IEEE residual.
	// Let r = a/b be the true value. At the leading doubles:
	//   r = c0 + Δa/b0 - (a0/b0^2) Δb + (a - b*c0)/b0 + O(eps^2)
	// where the IEEE residual `a - b*c0` is computable exactly from the
	// leading doubles via EFTs:
	//   two_prod(b0, c0) -> (prod_hi, prod_err) with b0*c0 = prod_hi + prod_err
	//   two_diff(a0, prod_hi) -> (diff_hi, diff_err)
	//   ieee_residual ~= (diff_hi + diff_err) - prod_err
	// The depth-1 component is then
	//   c1 = (ieee_residual + a.at(1) - c0 * b.at(1)) / b0
	// which fits gen_binary_linear with constant = ieee_residual/b0,
	// ca = 1/b0, cb = -c0/b0.
	double prod_err;
	double prod_hi = two_prod(b0, c0, prod_err);
	double diff_err;
	double diff_hi = two_diff(a0, prod_hi, diff_err);
	double ieee_residual = (diff_hi + diff_err) - prod_err;

	double inv_b0  = 1.0 / b0;
	double ca      = inv_b0;
	double cb      = -c0 * inv_b0;
	double cconst  = ieee_residual * inv_b0;

	// If b0 is a denormal whose reciprocal overflows to inf, any of
	// ca / cb / cconst can be non-finite even though c0 = a0/b0 was
	// finite. Installing such a generator would propagate inf/NaN
	// into the depth-1 component (and from there into every refined
	// result that touches at(1)). Bail out to depth-0-only.
	if (!std::isfinite(ca) || !std::isfinite(cb) || !std::isfinite(cconst)) {
		return result;
	}

	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(a),
		std::make_shared<const elreal>(b),
		cconst, ca, cb
	};
	return result;
}

inline elreal abs(const elreal& a) {
	if (a.isneg()) return -a;
	return a;
}

inline elreal& elreal::operator+=(const elreal& rhs) { return *this = *this + rhs; }
inline elreal& elreal::operator-=(const elreal& rhs) { return *this = *this - rhs; }
inline elreal& elreal::operator*=(const elreal& rhs) { return *this = *this * rhs; }
inline elreal& elreal::operator/=(const elreal& rhs) { return *this = *this / rhs; }

// fabs() is the canonical floating-point absolute-value name; provide as a
// thin alias to abs() so generic code templated on a real type can use
// either.
inline elreal fabs(const elreal& a) { return abs(a); }

// =============================================================================
// Phase E.2 math: sqrt + hypot (#888)
// =============================================================================
//
// sqrt
// ----
// Computes the square root of a non-negative elreal.
//
//   Depth 0: c0 = std::sqrt(a.at(0))  (IEEE-754 correctly rounded)
//   Depth 1: Newton-style EFT residual
//
//     (c0 + delta)^2 = a  =>  delta = (a - c0^2) / (2*c0 + delta)
//                      ~=  (a - c0^2) / (2*c0)   for |delta| << c0
//
//     a - c0^2 is computed exactly via two_prod (giving c0^2 = prod_hi +
//     prod_err) and a single double subtraction, then augmented with the
//     operand's own depth-1 correction a.at(1). The division by 2*c0 is
//     plain double arithmetic (no lazy division of elreal needed, so we
//     are *not* blocked by Phase F's reciprocal-stream work).
//   Depth 2+: 0.0 with a documented limitation.
//
// Negative argument handling: when ELREAL_THROW_ARITHMETIC_EXCEPTION is
// set, throws elreal_negative_sqrt_arg. Otherwise the leading double goes
// through std::sqrt which returns NaN (IEEE-754).
//
// hypot
// -----
// hypot(x, y) computed via std::hypot on the leading components, which
// avoids the overflow that a naive sqrt(x*x + y*y) would suffer for large
// |x| or |y|. Depth-1+ refinement is deferred to a follow-up: doing it
// correctly requires either (a) the overflow-protected algebraic form
// max(|x|,|y|) * sqrt(1 + (min/max)^2) walked at depth 1 or
// (b) sqrt of x*x + y*y at depth 1 (only safe when neither operand is
// near the overflow threshold). The depth-0 path covers the canonical
// numerical-geometry use cases.

inline elreal sqrt(const elreal& a) {
	// Find the first non-zero materialised component. The Shewchuk non-
	// overlapping invariant guarantees the value's sign equals sign(leading)
	// and its magnitude is dominated by |leading|. We must walk past leading
	// zeros because arithmetic operations can produce them legitimately --
	// e.g. `elreal(1, 3) - elreal(1.0/3.0)` cancels at depth 0 and leaves a
	// positive depth-1 correction. Relying on `a.at(0)` alone would
	// (a) misclassify a positive-then-negative-correction expansion as zero
	// and (b) misclassify a negative-magnitude value as zero, bypassing
	// the negative-argument handler.
	double leading = 0.0;
	std::size_t lead_idx = 0;
	for (std::size_t k = 0; k < a.computed_depth(); ++k) {
		double c = a.at(k);
		if (c != 0.0) { leading = c; lead_idx = k; break; }
	}
	// If every materialised component is zero we treat the value as zero
	// (indistinguishable from zero within the materialised precision).
	// Phase D's sign/comparison machinery uses the same convention.

	if (leading < 0.0) {
#if ELREAL_THROW_ARITHMETIC_EXCEPTION
		throw elreal_negative_sqrt_arg();
#else
		// fall through: std::sqrt of a negative double returns NaN per
		// IEEE-754; caller can disambiguate via isnan() at the call site.
#endif
	}

	double c0 = std::sqrt(leading);
	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// No depth-1 refinement makes sense for non-finite or zero leading.
	if (!std::isfinite(c0) || c0 == 0.0) return result;

	result._generator = gen_sqrt{
		std::make_shared<const elreal>(a),
		c0, lead_idx
	};
	return result;
}

inline elreal hypot(const elreal& a, const elreal& b) {
	double a0 = a.at(0);
	double b0 = b.at(0);
	// std::hypot handles overflow, signed zero, infinities, and NaN per
	// IEEE-754 conventions (e.g. hypot(inf, NaN) = inf, hypot(0, 0) = 0).
	double c0 = std::hypot(a0, b0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// Depth-1+ deferred: see header docblock above.
	return result;
}

// =============================================================================
// Phase E.3 math: exp / log / pow family (#889)
// =============================================================================
//
// All eight functions ship at the same level of refinement:
//   Depth 0: standard library (std::exp / std::log / etc.) on the leading
//            double, which is IEEE-754 correctly rounded across all
//            platforms we target.
//   Depth 1: derivative-based correction. For f(x), the depth-1 correction
//            is f'(x.at(0)) * x.at(1), capturing the first-order Taylor
//            contribution from the operand's depth-1 limb. For pow(a, b)
//            both operand corrections are captured: dpow/da * a.at(1) +
//            dpow/db * b.at(1).
//   Depth 2+: 0.0. Higher-order corrections (the std::lib rounding error
//            of the depth-0 computation, plus second-order Taylor terms)
//            require either (a) a higher-precision algorithm internally
//            or (b) Newton refinement using lazy division -- both Phase F.
//
// Edge cases: log(0) returns -inf; log(negative) and pow(negative,
// fractional) return NaN. We propagate IEEE behaviour without throwing;
// the result's isnan() / isinf() can be inspected by the caller. No new
// exception types are added in this phase.

inline elreal exp(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::exp(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || c0 == 0.0) return result;

	// d/dx exp(x) = exp(x), so depth-1 = c0 * a.at(1).
	result._generator = gen_unary_linear{ std::make_shared<const elreal>(a), c0 };
	return result;
}

inline elreal exp2(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::exp2(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || c0 == 0.0) return result;

	// d/dx 2^x = 2^x * ln(2), so depth-1 = c0 * ln2 * a.at(1).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		c0 * std::numbers::ln2_v<double>
	};
	return result;
}

inline elreal expm1(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::expm1(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx (exp(x) - 1) = exp(x) = expm1(x) + 1, so depth-1 = (c0+1) * a.at(1).
	result._generator = gen_unary_linear{ std::make_shared<const elreal>(a), c0 + 1.0 };
	return result;
}

inline elreal log(const elreal& a) {
	double a0 = a.at(0);
	// std::log handles all edge cases per IEEE-754:
	//   log(0)        = -inf
	//   log(negative) = NaN
	//   log(+inf)     = +inf
	//   log(NaN)      = NaN
	double c0 = std::log(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// No depth-1 refinement when the leading is non-finite or the operand
	// is zero (the 1/a0 derivative diverges).
	if (!std::isfinite(c0) || a0 == 0.0) return result;

	// d/dx log(x) = 1/x, so depth-1 = a.at(1) / a0.
	result._generator = gen_unary_linear{ std::make_shared<const elreal>(a), 1.0 / a0 };
	return result;
}

inline elreal log2(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::log2(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || a0 == 0.0) return result;

	// d/dx log2(x) = 1 / (x * ln(2)).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / (a0 * std::numbers::ln2_v<double>)
	};
	return result;
}

inline elreal log10(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::log10(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || a0 == 0.0) return result;

	// d/dx log10(x) = 1 / (x * ln(10)).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / (a0 * std::numbers::ln10_v<double>)
	};
	return result;
}

inline elreal log1p(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::log1p(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || (1.0 + a0) == 0.0) return result;

	// d/dx log(1 + x) = 1 / (1 + x).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / (1.0 + a0)
	};
	return result;
}

inline elreal pow(const elreal& a, const elreal& b) {
	double a0 = a.at(0);
	double b0 = b.at(0);
	double c0 = std::pow(a0, b0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// No depth-1 refinement when the leading is non-finite. Also skip when
	// a0 == 0 (the partial derivative with respect to a involves 1/a0 and
	// is undefined / singular) or when a0 < 0 with non-integer b (pow itself
	// is NaN, which is already captured at depth 0).
	if (!std::isfinite(c0) || a0 <= 0.0) return result;

	// Depth-1 correction has TWO sources:
	//   dpow/da = b * a^(b-1) = b * c0 / a
	//   dpow/db = a^b * log(a) = c0 * log(a)
	//
	// Both are evaluated at the leading doubles to avoid extra std lib
	// calls inside the lazy machinery.
	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(a),
		std::make_shared<const elreal>(b),
		0.0,
		b0 * c0 / a0,
		c0 * std::log(a0)
	};
	return result;
}

// =============================================================================
// Phase E.4 math: hyperbolic functions (#890)
// =============================================================================
//
// Six functions (sinh/cosh/tanh and inverses) following the same uniform
// pattern as Phase E.3: std library at depth 0, derivative-based correction
// at depth 1.
//
// Per-function derivatives:
//   d/dx sinh(x)  = cosh(x)                  -> depth-1 = cosh(a0) * a.at(1)
//   d/dx cosh(x)  = sinh(x)                  -> depth-1 = sinh(a0) * a.at(1)
//   d/dx tanh(x)  = 1 - tanh^2(x)            -> depth-1 = (1 - c0*c0) * a.at(1)
//   d/dx asinh(x) = 1 / sqrt(1 + x^2)        -> depth-1 = a.at(1) / sqrt(1+a0*a0)
//   d/dx acosh(x) = 1 / sqrt(x^2 - 1)        -> depth-1 = a.at(1) / sqrt(a0*a0-1)
//   d/dx atanh(x) = 1 / (1 - x^2)            -> depth-1 = a.at(1) / (1 - a0*a0)
//
// Edge cases (per IEEE-754 / std lib):
//   acosh(x < 1)   -> NaN
//   atanh(|x| > 1) -> NaN; atanh(+/-1) -> +/-inf
//   All others well-defined for any real input.

inline elreal sinh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::sinh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx sinh(x) = cosh(x).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		std::cosh(a0)
	};
	return result;
}

inline elreal cosh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::cosh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx cosh(x) = sinh(x).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		std::sinh(a0)
	};
	return result;
}

inline elreal tanh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::tanh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx tanh = 1 - tanh^2 (= sech^2). Using c0 avoids a second std::tanh.
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 - c0 * c0
	};
	return result;
}

inline elreal asinh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::asinh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx asinh(x) = 1 / sqrt(1 + x^2). Always finite and nonzero.
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / std::sqrt(1.0 + a0 * a0)
	};
	return result;
}

inline elreal acosh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::acosh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// Skip refinement when the leading is NaN/inf, or when a0 <= 1 (derivative
	// diverges at a0 = 1; std::acosh returns NaN for a0 < 1 anyway).
	if (!std::isfinite(c0) || a0 <= 1.0) return result;

	// d/dx acosh(x) = 1 / sqrt(x^2 - 1).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / std::sqrt(a0 * a0 - 1.0)
	};
	return result;
}

inline elreal atanh(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::atanh(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// Skip refinement when the leading is NaN/inf or when the derivative
	// diverges at |a0| = 1.
	if (!std::isfinite(c0) || std::abs(a0) >= 1.0) return result;

	// d/dx atanh(x) = 1 / (1 - x^2).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / (1.0 - a0 * a0)
	};
	return result;
}

// =============================================================================
// Phase E.5 math: inverse trigonometric functions (#891)
// =============================================================================
//
// Four functions (asin, acos, atan, atan2) following the same depth-0 +
// depth-1-derivative pattern. The inverse trig functions do NOT need
// pi-based range reduction (unlike forward trig in E.6, which is the
// hardest sub-issue) -- the std lib handles all the work at depth 0,
// and the derivatives are closed-form rational expressions in the
// argument.
//
// Per-function derivatives:
//   d/dx asin(x)  =  1 / sqrt(1 - x^2)        valid for |x| < 1
//   d/dx acos(x)  = -1 / sqrt(1 - x^2)        valid for |x| < 1
//   d/dx atan(x)  =  1 / (1 + x^2)            always finite, always > 0
//   d  atan2(y,x): partials are
//     d/dy =  x / (x^2 + y^2)
//     d/dx = -y / (x^2 + y^2)
//   Both atan2 partials are finite except at the origin (y == 0 && x == 0)
//   where atan2 itself is conventionally defined as 0 with no derivative.
//
// Edge cases:
//   asin / acos at |x| > 1   -> NaN via std lib
//   asin / acos at |x| = 1   -> well-defined, but derivative diverges
//                               (depth-1 skipped)
//   atan(+/-inf)             -> +/- pi/2 (std lib handles; depth-1 = 0 since
//                               the derivative 1/(1+x^2) goes to 0)
//   atan2(0, 0)              -> 0 (C convention; depth-1 skipped)

inline elreal asin(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::asin(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// Skip refinement for NaN/inf, or when the derivative diverges (|a0| = 1).
	if (!std::isfinite(c0) || std::abs(a0) >= 1.0) return result;

	// d/dx asin(x) = 1 / sqrt(1 - x^2).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / std::sqrt(1.0 - a0 * a0)
	};
	return result;
}

inline elreal acos(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::acos(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0) || std::abs(a0) >= 1.0) return result;

	// d/dx acos(x) = -1 / sqrt(1 - x^2).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		-1.0 / std::sqrt(1.0 - a0 * a0)
	};
	return result;
}

inline elreal atan(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::atan(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// atan is well-defined for any real input including +/-inf. Only NaN
	// needs the early exit -- and even for +/-inf the derivative
	// 1/(1+x^2) is finite (= 0), so depth-1 is just zero.
	if (!std::isfinite(c0)) return result;

	// d/dx atan(x) = 1 / (1 + x^2). For huge |a0|, 1 + a0*a0 overflows
	// double; in that regime the derivative is essentially 0 and we just
	// return 0 directly.
	double denom = 1.0 + a0 * a0;
	if (!std::isfinite(denom)) return result;

	// d/dx atan(x) = 1 / (1 + x^2).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 / denom
	};
	return result;
}

inline elreal atan2(const elreal& y, const elreal& x) {
	double y0 = y.at(0);
	double x0 = x.at(0);
	double c0 = std::atan2(y0, x0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	// Skip refinement for NaN/inf and at the singularity (0, 0). The
	// std lib already handled the canonical edge cases (signed zero
	// quadrant, infinity quadrant).
	if (!std::isfinite(c0)) return result;
	double r2 = x0 * x0 + y0 * y0;
	if (r2 == 0.0 || !std::isfinite(r2)) return result;

	// d/dy atan2(y,x) =  x / (x^2 + y^2)
	// d/dx atan2(y,x) = -y / (x^2 + y^2)
	// gen_binary_linear: a=y, b=x, ca=x/r2, cb=-y/r2.
	result._generator = gen_binary_linear{
		std::make_shared<const elreal>(y),
		std::make_shared<const elreal>(x),
		0.0,
		x0 / r2,
		-y0 / r2
	};
	return result;
}

// =============================================================================
// Phase E.6 math: forward trigonometric functions (#892)
// =============================================================================
//
// sin, cos, tan via the depth-0-std-lib + depth-1-derivative pattern.
//
// Per-function derivatives:
//   d/dx sin(x) =  cos(x)              -> cos(a0) * a.at(1)
//   d/dx cos(x) = -sin(x)              -> -sin(a0) * a.at(1)
//   d/dx tan(x) = 1 + tan^2(x)         -> (1 + c0*c0) * a.at(1)
//
// -----------------------------------------------------------------------------
// Range-reduction limitation -- the canonical hard case for lazy reals
// -----------------------------------------------------------------------------
//
// For large-magnitude arguments, std::sin / std::cos / std::tan internally
// reduce the argument modulo 2*pi (or pi/2) before evaluating a polynomial
// approximation. That reduction is done with the IEEE-754 double M_PI, which
// carries only 53 bits of precision. When |x| approaches 2^53, the reduced
// argument has *no precision left* -- std::sin(1e20) is essentially noise.
//
// McCleeary 2019 / Payne-Hanek 1983 solves this by performing the range
// reduction with arbitrarily many bits of pi pulled lazily from the
// constant's stream. Specifically:
//
//   1. Compute k = round(x / (pi/2))                  // integer multiple
//   2. Reduce r = x - k * (pi/2) using EXACT lazy arithmetic
//      (pi/2 carried at enough precision that |r| < ulp(x) is achievable)
//   3. Dispatch by k mod 4:
//        k mod 4 == 0:  sin(r),  cos(r)
//        k mod 4 == 1:  cos(r), -sin(r)
//        k mod 4 == 2: -sin(r), -cos(r)
//        k mod 4 == 3: -cos(r),  sin(r)
//   4. Evaluate sin(r) / cos(r) on the reduced argument via Taylor series.
//
// Phase E.6 ships ONLY the std-lib-based depth-0 + derivative-based depth-1
// computation. This is correct for arguments of "reasonable magnitude" --
// roughly |x| < 2^25 or so, where std::sin/cos retain their full
// double-precision contract. For |x| beyond that, the result is faithful at
// depth 0 only in the IEEE-754 sense (matches std::sin) but loses precision
// as |x| grows; the depth-1 correction adds the operand's contribution but
// cannot recover bits lost in std's internal range reduction.
//
// Implementing the lazy Payne-Hanek path is its own substantial PR that
// would need:
//   - elreal_pi extended past the current 4-component (212-bit) static
//     expansion (a generator-based variant of from_expansion, or a BBP
//     bit-extraction algorithm)
//   - An integer-mod-pi reduction loop that pulls more bits of pi from
//     elreal_pi until |r| < ulp(x) is achievable
//   - Taylor / Chebyshev evaluation of sin/cos on the reduced argument
//     with lazy refinement
//
// This is filed as a Phase-F-or-later follow-up. The acceptance criteria
// of #892 explicitly note that direct cross-validation against std::sin
// for huge-magnitude inputs is not meaningful (lazy real arithmetic is
// where huge-magnitude trig *wins* over double); so the limitation is
// documented rather than enforced as a test gate.
//
// References:
//   - McCleeary, R. (2019). "Lazy Exact Real Arithmetic Using Floating
//     Point Operations." Ph.D. dissertation, University of Iowa.
//   - Payne, M. and Hanek, R. (1983). "Radian Reduction for Trigonometric
//     Functions." SIGNUM Newsletter, 18(1), 19-24.

inline elreal sin(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::sin(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx sin(x) = cos(x).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		std::cos(a0)
	};
	return result;
}

inline elreal cos(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::cos(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx cos(x) = -sin(x).
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		-std::sin(a0)
	};
	return result;
}

inline elreal tan(const elreal& a) {
	double a0 = a.at(0);
	double c0 = std::tan(a0);

	elreal result;
	result._components.push_back(c0);
	result._computed_depth = 1;

	if (!std::isfinite(c0)) return result;

	// d/dx tan(x) = 1 + tan^2(x). Using c0 avoids a second std::tan.
	result._generator = gen_unary_linear{
		std::make_shared<const elreal>(a),
		1.0 + c0 * c0
	};
	return result;
}

// =============================================================================
// Comparison and sign determination (Phase D)
// =============================================================================
//
// The signature operation of any lazy-real implementation: deciding the sign
// of a value (and from it, all binary comparisons) may require pulling
// arbitrarily many bits from the operand streams. McCleeary's correctness
// argument relies on the non-overlapping property of the expansion:
//
//   For a normalized elreal value with components c_0, c_1, ..., c_{N-1}
//   satisfying |c_{i+1}| <= ulp(c_i)/2 and monotonically decreasing in
//   magnitude, the sign of the true value equals the sign of the first
//   non-zero component. The remaining components are corrections smaller
//   than the leading non-zero by at least one bit each, so their sum
//   cannot flip the sign.
//
// The algorithm therefore walks the stream calling `at(k)` until either:
//   (a) `at(k) != 0` is found  -> sign is sign(at(k));
//   (b) `budget` components have been materialised with all zero  -> the
//       value is indistinguishable from zero at the requested precision,
//       treat as zero.
//
// Reference: McCleeary 2019 (PhD dissertation, Univ. of Iowa) Section 4.2
// "Sign Determination and Comparison" -- the argument used here is the
// "first non-zero limb" theorem, which the dissertation proves for
// non-overlapping expansions generated by the EFT primitives.

// Default refinement budget for sign / compare. Each component carries
// ~53 bits, so 8 components = ~424 bits cumulative -- enough for any
// practical signed-comparison query that does not involve hash-collision
// grade equality testing. Callers needing more precision pass a larger
// value to `sign(v, budget)` / `compare(a, b, budget)`.
inline constexpr std::size_t elreal_default_budget = 8;

// sign(v, budget): returns -1, 0, or +1. Result 0 means "within budget,
// the value is indistinguishable from zero." Raising the budget can
// promote 0 to +/-1 if the underlying value is in fact non-zero (e.g.,
// a near-cancellation result with a deep first non-zero component).
//
// Special values:
//   - NaN: returns 0 (NaN is unordered). Callers that want explicit NaN
//     handling should check isnan() before calling sign().
//   - +inf / -inf: returns +1 / -1 respectively, without consulting the
//     budget (inf is unambiguously signed).
inline int sign(const elreal& v, std::size_t budget = elreal_default_budget) {
	if (v.isnan()) return 0;
	if (v.isinf()) return v.isneg() ? -1 : +1;

	for (std::size_t k = 0; k < budget; ++k) {
		double c = v.at(k);
		if (c > 0.0) return +1;
		if (c < 0.0) return -1;
		// c == 0: continue to the next component
	}
	return 0;  // budget exhausted with all-zero components
}

// compare(a, b, budget): returns -1 if a < b, 0 if a ~= b within budget,
// +1 if a > b. Implemented as sign(a - b, budget) for now; a hand-fused
// walk that pulls bits from a and b in lockstep is a Phase I performance
// optimization.
//
// NaN handling: returns 0 (unordered) -- but the ordering operators below
// short-circuit on NaN before reaching here, so this value is only seen
// by direct callers of compare().
inline int compare(const elreal& a, const elreal& b,
                   std::size_t budget = elreal_default_budget) {
	if (a.isnan() || b.isnan()) return 0;
	return sign(a - b, budget);
}

// IEEE-754 ordering semantics: all ordering operators return false when
// either operand is NaN; only `!=` returns true on NaN (a NaN is "not
// equal to" anything, including itself).
inline bool operator<(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return false;
	return compare(a, b) < 0;
}
inline bool operator<=(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return false;
	return compare(a, b) <= 0;
}
inline bool operator>(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return false;
	return compare(a, b) > 0;
}
inline bool operator>=(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return false;
	return compare(a, b) >= 0;
}

// Equality is budgeted: `a == b` means "indistinguishable within the
// default budget." Two elreal values constructed differently (e.g.,
// elreal(1, 3) vs elreal(1.0/3.0)) represent different reals and the
// budgeted equality will return false because their difference has a
// non-zero component within the budget. This is the *point* of lazy
// real arithmetic.
inline bool operator==(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return false;
	return compare(a, b) == 0;
}

// IEEE-754: NaN != x is true for any x (including another NaN). The
// existing comparison operators return false on NaN, so we cannot just
// negate operator==; we have to handle NaN explicitly.
inline bool operator!=(const elreal& a, const elreal& b) {
	if (a.isnan() || b.isnan()) return true;
	return compare(a, b) != 0;
}

}} // namespace sw::universal

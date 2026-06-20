#pragma once
// elreal_impl.hpp: the elreal class facade -- a plug-in arithmetic number system
// over the McCleeary LFPERA lazy block co-list (ZBCL).
//
// elreal<FpType> wraps a ZBCL<FpType> (a lazy, memoised, pull-driven stream of
// blocks) and presents the standard Universal plug-in interface (native ctors,
// conversions, arithmetic + logic operators) so it drops into templated kernels
// like any other number type. Unlike the eager ereal (Priest/Shewchuk expansion),
// elreal is LAZY: arithmetic operators store unforced streams and evaluation
// happens only at a boundary (conversion / comparison / I/O / explicit approx),
// to a runtime-controlled precision (_depth). Because the underlying ZBCL memoises
// its tail thunks, refining to a deeper precision REUSES the work already pulled --
// the incremental-precision edge over the eager elastic types.
//
// elreal is an ELASTIC type: it holds a ZBCL (shared_ptr) and is therefore NOT
// trivially copyable (the #925 hardware-shareable triviality rule applies to the
// static `block`, not to this elastic facade -- same as ereal/einteger/edecimal).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <vector>
#include <type_traits>

#include <universal/number/shared/specific_value_encoding.hpp>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>     // from_native, to_double_approx
#include <universal/number/elreal/threeAdd.hpp>         // add
#include <universal/number/elreal/negate.hpp>           // negate
#include <universal/number/elreal/online_multiply.hpp>  // mul_online
#include <universal/number/elreal/online_divide.hpp>    // div_online

namespace sw { namespace universal {

// elreal default precision (in blocks). One block ~ k bits ~ 16 decimal digits for
// a double host; the default is the pull depth used by boundary operations
// (conversion / comparison / I/O) unless overridden per-object via precision(). It
// is a thread-local default so an enclosing scope can change it (see
// elreal_precision_guard) without recompiling the type.
// The compile-time nominal default precision (in blocks). numeric_limits reports
// its precision-dependent fields against this; the runtime default below is seeded
// from it and may be changed per-scope (elreal_precision_guard).
inline constexpr std::size_t kElrealDefaultPrecision = 8;   // ~128 decimal digits on a double host

inline std::size_t& elreal_default_precision() {
    static thread_local std::size_t depth = kElrealDefaultPrecision;
    return depth;
}

// elreal is exact over the FINITE reals, but it also carries an IEEE-style
// non-finite classification so it behaves predictably in plug-in kernels that
// can produce NaN / +-Inf (e.g. x/0, log of a negative, overflow on conversion).
// A non-finite elreal holds an empty stream and a non-finite _cls tag; the tag
// propagates through the arithmetic, comparison, and conversion boundaries by the
// usual IEEE-754 rules (#1079 Phase 5).
enum class elreal_class : std::uint8_t { finite = 0, pinf = 1, ninf = 2, qnan = 3 };

namespace detail {
    inline constexpr bool is_inf_class(elreal_class c) noexcept {
        return c == elreal_class::pinf || c == elreal_class::ninf;
    }
    inline constexpr elreal_class negate_class(elreal_class c) noexcept {
        return c == elreal_class::pinf ? elreal_class::ninf
             : c == elreal_class::ninf ? elreal_class::pinf
             : c;  // finite / qnan unchanged
    }
    // the inf class with the given sign (+1 -> pinf, -1 -> ninf)
    inline constexpr elreal_class inf_with_sign(int s) noexcept {
        return s < 0 ? elreal_class::ninf : elreal_class::pinf;
    }
}

// RAII scoped override of the default precision. Non-copyable/non-movable: it
// restores shared thread-local state on destruction, so a copy/move would let a
// stale instance clobber the active scope.
struct elreal_precision_guard {
    std::size_t saved;
    explicit elreal_precision_guard(std::size_t d) : saved(elreal_default_precision()) {
        elreal_default_precision() = d;
    }
    elreal_precision_guard(const elreal_precision_guard&) = delete;
    elreal_precision_guard& operator=(const elreal_precision_guard&) = delete;
    elreal_precision_guard(elreal_precision_guard&&) = delete;
    elreal_precision_guard& operator=(elreal_precision_guard&&) = delete;
    ~elreal_precision_guard() { elreal_default_precision() = saved; }
};

template <typename FpType = double>
class elreal {
public:
    using value_type = FpType;
    using block_type = block<FpType>;
    using stream_type = ZBCL<FpType>;

    // --- construction --------------------------------------------------------
    elreal() : _value{}, _depth(elreal_default_precision()), _cls(elreal_class::finite) {}   // 0
    elreal(const elreal&) = default;
    elreal(elreal&&) noexcept = default;
    elreal& operator=(const elreal&) = default;
    elreal& operator=(elreal&&) noexcept = default;

    // wrap an existing lazy stream (advanced / internal) -- always finite
    explicit elreal(stream_type v, std::size_t depth = elreal_default_precision())
        : _value(std::move(v)), _depth(depth), _cls(elreal_class::finite) {}

    // native conversions (exact for integers < 2^53; from_native for floats)
    elreal(signed char iv)        { *this = static_cast<double>(iv); }
    elreal(short iv)              { *this = static_cast<double>(iv); }
    elreal(int iv)               { *this = static_cast<double>(iv); }
    elreal(long iv)              { *this = static_cast<double>(iv); }
    elreal(long long iv)         { *this = static_cast<double>(iv); }
    elreal(unsigned char iv)     { *this = static_cast<double>(iv); }
    elreal(unsigned short iv)    { *this = static_cast<double>(iv); }
    elreal(unsigned int iv)      { *this = static_cast<double>(iv); }
    elreal(unsigned long iv)     { *this = static_cast<double>(iv); }
    elreal(unsigned long long iv){ *this = static_cast<double>(iv); }
    elreal(float iv)             { *this = static_cast<double>(iv); }
    elreal(double iv)            { *this = iv; }

    elreal& operator=(double rhs) {
        _depth = elreal_default_precision();
        if (std::isnan(rhs)) { _value = stream_type{}; _cls = elreal_class::qnan; return *this; }
        if (std::isinf(rhs)) { _value = stream_type{}; _cls = detail::inf_with_sign(rhs < 0 ? -1 : 1); return *this; }
        _value = from_native<FpType>(rhs);
        _cls   = elreal_class::finite;
        return *this;
    }
    elreal& operator=(float rhs)       { return *this = static_cast<double>(rhs); }
    elreal& operator=(int rhs)         { return *this = static_cast<double>(rhs); }
    elreal& operator=(long long rhs)   { return *this = static_cast<double>(rhs); }

    // SpecificValue: materialise a named encoding. elreal now carries a non-finite
    // classification, so infpos/infneg/qnan/snan map to the corresponding states.
    elreal(const SpecificValue code) : _value{}, _depth(elreal_default_precision()), _cls(elreal_class::finite) {
        switch (code) {
            case SpecificValue::maxpos:  *this =  1.0e308;  break;
            case SpecificValue::maxneg:  *this = -1.0e308;  break;
            case SpecificValue::minpos:  *this =  1.0e-307; break;
            case SpecificValue::minneg:  *this = -1.0e-307; break;
            case SpecificValue::infpos:  _cls = elreal_class::pinf; break;
            case SpecificValue::infneg:  _cls = elreal_class::ninf; break;
            case SpecificValue::qnan:
            case SpecificValue::snan:    _cls = elreal_class::qnan; break;
            default:                     _value = stream_type{};  break;  // zero
        }
    }

    // --- conversions (boundary: forces evaluation to _depth) -----------------
    explicit operator double()      const noexcept { return host_value<double>(); }
    explicit operator float()       const noexcept { return static_cast<float>(host_value<double>()); }
    explicit operator long double() const noexcept { return host_value<long double>(); }

    // --- arithmetic (LAZY for finite operands; IEEE rules for non-finite) ----
    elreal operator-() const {
        if (_cls != elreal_class::finite) return elreal(detail::negate_class(_cls), _depth);
        return elreal(negate(_value), _depth);
    }

    elreal& operator+=(const elreal& rhs) {
        const std::size_t nd = deeper(rhs);
        if (isnan() || rhs.isnan()) return become(elreal_class::qnan, nd);
        const bool ai = isinf(), bi = rhs.isinf();
        if (ai && bi) return (_cls == rhs._cls) ? keep(nd) : become(elreal_class::qnan, nd); // inf-inf -> nan
        if (ai) return keep(nd);                                    // inf + finite -> inf
        if (bi) return become(rhs._cls, nd);                        // finite + inf -> inf
        _value = add(_value, rhs._value); _depth = nd; return *this;
    }
    elreal& operator-=(const elreal& rhs) { return *this += (-rhs); }
    elreal& operator*=(const elreal& rhs) {
        const std::size_t nd = deeper(rhs);
        if (isnan() || rhs.isnan()) return become(elreal_class::qnan, nd);
        if (isinf() || rhs.isinf()) {
            if (iszero() || rhs.iszero()) return become(elreal_class::qnan, nd);  // inf * 0
            return become(detail::inf_with_sign(sign() * rhs.sign()), nd);
        }
        _value = mul_online(_value, rhs._value); _depth = nd; return *this;
    }
    elreal& operator/=(const elreal& rhs) {
        const std::size_t nd = deeper(rhs);
        if (isnan() || rhs.isnan()) return become(elreal_class::qnan, nd);
        if (rhs.isinf()) return isinf() ? become(elreal_class::qnan, nd)         // inf / inf
                                        : become(elreal_class::finite, nd);      // finite / inf -> 0
        if (rhs.iszero()) return iszero() ? become(elreal_class::qnan, nd)       // 0 / 0
                                          : become(detail::inf_with_sign(sign() * rhs.sign()), nd); // x / 0
        if (isinf()) return become(detail::inf_with_sign(sign() * rhs.sign()), nd);  // inf / finite
        _value = div_online(_value, rhs._value); _depth = nd; return *this;
    }

    // --- lazy API / state-machine extension ----------------------------------
    // precision(): the per-object default pull depth for boundary ops.
    std::size_t precision() const noexcept { return _depth; }
    elreal& precision(std::size_t d) noexcept { _depth = d; return *this; }

    // limbs(n): pull the first n blocks (reuses memoised work on repeat/deeper calls).
    std::vector<block_type> limbs(std::size_t n) const { return _value.take(n); }

    // approx<T>(depth): materialise the value to `depth` blocks as a native floating
    // T, summed IN T -- so approx<long double> keeps the extra range/precision a wider
    // host offers (rather than rounding through double). T must be a floating type.
    template <typename T = double>
    T approx(std::size_t depth) const {
        static_assert(std::is_floating_point_v<T>, "elreal::approx<T> requires a native floating-point T");
        const auto blocks = _value.take(depth);
        return std::accumulate(blocks.begin(), blocks.end(), T{0},
            [](T acc, const auto& b) { return acc + b.template value_as<T>(); });
    }

    // refine(depth): raise the default pull depth; the next boundary op pulls deeper,
    // reusing everything already memoised. The incremental-precision primitive.
    elreal& refine(std::size_t depth) noexcept { if (depth > _depth) _depth = depth; return *this; }

    // stream(): the raw lazy state machine (advanced).
    const stream_type& stream() const noexcept { return _value; }

    // --- classification ------------------------------------------------------
    bool iszero()   const noexcept { return _cls == elreal_class::finite && _value.is_empty(); }
    bool isnan()    const noexcept { return _cls == elreal_class::qnan; }
    bool isinf()    const noexcept { return detail::is_inf_class(_cls); }
    bool isfinite() const noexcept { return _cls == elreal_class::finite; }
    bool isneg()    const noexcept { return sign() < 0; }

    // sign(): -1 if negative, +1 otherwise. For +-inf the inf sign; for nan +1
    // (sign of NaN is unspecified); for finite the most significant block's sign
    // (+1 for zero).
    int sign() const noexcept {
        if (_cls == elreal_class::pinf || _cls == elreal_class::qnan) return 1;
        if (_cls == elreal_class::ninf) return -1;
        auto bl = _value.take(1);
        return (!bl.empty() && bl.front().sign() < 0) ? -1 : 1;
    }
    // scale(): the value's binary exponent (leading block's combined exponent);
    // 0 for zero and for non-finite. int64_t (not int): elreal carries an unbounded
    // integer<256> exponent, so a narrow int cast could overflow; the host-FP
    // attribute surface this feeds (ldexp / significand) is bounded by FpType's
    // exponent range, for which int64_t has vast headroom; the full-width exponent
    // stays reachable via block::exponent().
    int64_t scale() const noexcept {
        if (_cls != elreal_class::finite) return 0;
        auto bl = _value.take(1);
        return bl.empty() ? 0 : static_cast<int64_t>(bl.front().exponent());
    }

private:
    stream_type  _value;   // lazy, memoised block co-list (empty when non-finite)
    std::size_t  _depth;   // default pull depth for boundary operations
    elreal_class _cls;     // IEEE-style finite / +-inf / nan classification

    // internal: construct a non-finite elreal (empty stream, given class)
    elreal(elreal_class cls, std::size_t depth) : _value{}, _depth(depth), _cls(cls) {}
    template <typename> friend class elreal;

    // host_value<T>(): the value as native T, honouring the non-finite class.
    template <typename T>
    T host_value() const noexcept {
        switch (_cls) {
            case elreal_class::qnan: return std::numeric_limits<T>::quiet_NaN();
            case elreal_class::pinf: return std::numeric_limits<T>::infinity();
            case elreal_class::ninf: return -std::numeric_limits<T>::infinity();
            default:                 return approx<T>(_depth);
        }
    }

    // arithmetic-result helpers
    std::size_t deeper(const elreal& rhs) const noexcept { return _depth > rhs._depth ? _depth : rhs._depth; }
    elreal& keep(std::size_t d) noexcept { _depth = d; return *this; }   // retain class/value, set depth
    elreal& become(elreal_class c, std::size_t d) {                       // finite c -> zero; else that state
        _value = stream_type{}; _cls = c; _depth = d; return *this;
    }
};

// =============================================================================
// free arithmetic operators (lazy)
// =============================================================================
template <typename FpType>
inline elreal<FpType> operator+(const elreal<FpType>& a, const elreal<FpType>& b) { elreal<FpType> r(a); r += b; return r; }
template <typename FpType>
inline elreal<FpType> operator-(const elreal<FpType>& a, const elreal<FpType>& b) { elreal<FpType> r(a); r -= b; return r; }
template <typename FpType>
inline elreal<FpType> operator*(const elreal<FpType>& a, const elreal<FpType>& b) { elreal<FpType> r(a); r *= b; return r; }
template <typename FpType>
inline elreal<FpType> operator/(const elreal<FpType>& a, const elreal<FpType>& b) { elreal<FpType> r(a); r /= b; return r; }

// mixed with double
template <typename FpType> inline elreal<FpType> operator+(const elreal<FpType>& a, double b) { return a + elreal<FpType>(b); }
template <typename FpType> inline elreal<FpType> operator-(const elreal<FpType>& a, double b) { return a - elreal<FpType>(b); }
template <typename FpType> inline elreal<FpType> operator*(const elreal<FpType>& a, double b) { return a * elreal<FpType>(b); }
template <typename FpType> inline elreal<FpType> operator/(const elreal<FpType>& a, double b) { return a / elreal<FpType>(b); }
template <typename FpType> inline elreal<FpType> operator+(double a, const elreal<FpType>& b) { return elreal<FpType>(a) + b; }
template <typename FpType> inline elreal<FpType> operator-(double a, const elreal<FpType>& b) { return elreal<FpType>(a) - b; }
template <typename FpType> inline elreal<FpType> operator*(double a, const elreal<FpType>& b) { return elreal<FpType>(a) * b; }
template <typename FpType> inline elreal<FpType> operator/(double a, const elreal<FpType>& b) { return elreal<FpType>(a) / b; }

// =============================================================================
// logic operators (DEPTH-BOUNDED: compare a-b to the deeper of the two depths;
// equal-to-precision when no nonzero limb appears within that window). Exact
// ordering when the difference's leading block is nonzero. Exact equality of two
// distinct irrationals is undecidable -- this is a precision-bounded comparison.
// =============================================================================
template <typename FpType>
inline int elreal_cmp(const elreal<FpType>& a, const elreal<FpType>& b) {
    const std::size_t d = a.precision() > b.precision() ? a.precision() : b.precision();
    ZBCL<FpType> diff = add(a.stream(), negate(b.stream()));
    const auto blocks = diff.take(d + 1);
    // first nonzero limb decides the ordering; none within d -> equal to precision
    const auto it = std::find_if(blocks.begin(), blocks.end(),
        [](const auto& blk) { return !blk.is_zero_block(); });
    return it != blocks.end() ? it->sign() : 0;
}
// total ordering with IEEE non-finite semantics layered on top of the finite
// (depth-bounded) primitive: NaN is unordered (every relation but != is false);
// -inf < finite < +inf; like-signed infinities compare equal.
enum class elreal_order : std::uint8_t { less, equal, greater, unordered };
template <typename FpType>
inline elreal_order elreal_order_of(const elreal<FpType>& a, const elreal<FpType>& b) {
    if (a.isnan() || b.isnan()) return elreal_order::unordered;
    if (a.isinf() || b.isinf()) {
        const int ra = a.isinf() ? a.sign() * 2 : 0;   // +inf->+2, -inf->-2, finite->0
        const int rb = b.isinf() ? b.sign() * 2 : 0;
        if (ra == rb) return elreal_order::equal;
        return ra < rb ? elreal_order::less : elreal_order::greater;
    }
    const int c = elreal_cmp(a, b);
    return c < 0 ? elreal_order::less : (c > 0 ? elreal_order::greater : elreal_order::equal);
}
template <typename FpType> inline bool operator==(const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_order_of(a, b) == elreal_order::equal; }
template <typename FpType> inline bool operator!=(const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_order_of(a, b) != elreal_order::equal; }
template <typename FpType> inline bool operator< (const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_order_of(a, b) == elreal_order::less; }
template <typename FpType> inline bool operator> (const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_order_of(a, b) == elreal_order::greater; }
template <typename FpType> inline bool operator<=(const elreal<FpType>& a, const elreal<FpType>& b) { const auto o = elreal_order_of(a, b); return o == elreal_order::less    || o == elreal_order::equal; }
template <typename FpType> inline bool operator>=(const elreal<FpType>& a, const elreal<FpType>& b) { const auto o = elreal_order_of(a, b); return o == elreal_order::greater || o == elreal_order::equal; }
template <typename FpType> inline bool operator==(const elreal<FpType>& a, double b) { return a == elreal<FpType>(b); }
template <typename FpType> inline bool operator!=(const elreal<FpType>& a, double b) { return !(a == elreal<FpType>(b)); }
template <typename FpType> inline bool operator< (const elreal<FpType>& a, double b) { return a <  elreal<FpType>(b); }
template <typename FpType> inline bool operator> (const elreal<FpType>& a, double b) { return a >  elreal<FpType>(b); }

// abs / fabs
template <typename FpType>
inline elreal<FpType> abs(const elreal<FpType>& a) {
    if (a.isnan()) return a;                       // |nan| = nan
    if (a.isinf()) return a.sign() < 0 ? -a : a;   // |-inf| = +inf
    // sign from the leading (most significant) block
    auto bl = a.stream().take(1);
    bool neg = !bl.empty() && bl.front().sign() < 0;
    return neg ? -a : a;
}
template <typename FpType> inline elreal<FpType> fabs(const elreal<FpType>& a) { return abs(a); }

}} // namespace sw::universal

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
#include <cstddef>
#include <cstdint>
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
    elreal() : _value{}, _depth(elreal_default_precision()) {}   // 0 (empty stream)
    elreal(const elreal&) = default;
    elreal(elreal&&) noexcept = default;
    elreal& operator=(const elreal&) = default;
    elreal& operator=(elreal&&) noexcept = default;

    // wrap an existing lazy stream (advanced / internal)
    explicit elreal(stream_type v, std::size_t depth = elreal_default_precision())
        : _value(std::move(v)), _depth(depth) {}

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
        _value = from_native<FpType>(rhs);
        _depth = elreal_default_precision();
        return *this;
    }
    elreal& operator=(float rhs)       { return *this = static_cast<double>(rhs); }
    elreal& operator=(int rhs)         { return *this = static_cast<double>(rhs); }
    elreal& operator=(long long rhs)   { return *this = static_cast<double>(rhs); }

    // SpecificValue: elreal is an exact FINITE real; non-finite codes map to 0 for
    // plug-in compatibility (a dedicated non-finite state is a later design item).
    elreal(const SpecificValue code) : _value{}, _depth(elreal_default_precision()) {
        switch (code) {
            case SpecificValue::maxpos:  *this =  1.0e308; break;
            case SpecificValue::maxneg:  *this = -1.0e308; break;
            case SpecificValue::minpos:  *this =  1.0e-307; break;
            case SpecificValue::minneg:  *this = -1.0e-307; break;
            default:                     _value = stream_type{}; break;  // zero / nan / inf
        }
    }

    // --- conversions (boundary: forces evaluation to _depth) -----------------
    explicit operator double()      const noexcept { return approx<double>(_depth); }
    explicit operator float()       const noexcept { return static_cast<float>(approx<double>(_depth)); }
    explicit operator long double() const noexcept { return approx<long double>(_depth); }

    // --- arithmetic (LAZY: store unforced streams) ---------------------------
    elreal operator-() const { return elreal(negate(_value), _depth); }

    elreal& operator+=(const elreal& rhs) {
        _value = add(_value, rhs._value);
        _depth = _depth > rhs._depth ? _depth : rhs._depth;
        return *this;
    }
    elreal& operator-=(const elreal& rhs) { return *this += (-rhs); }
    elreal& operator*=(const elreal& rhs) {
        _value = mul_online(_value, rhs._value);
        _depth = _depth > rhs._depth ? _depth : rhs._depth;
        return *this;
    }
    elreal& operator/=(const elreal& rhs) {
        _value = div_online(_value, rhs._value);
        _depth = _depth > rhs._depth ? _depth : rhs._depth;
        return *this;
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
        T acc = T{0};
        for (const auto& b : _value.take(depth)) acc += b.template value_as<T>();
        return acc;
    }

    // refine(depth): raise the default pull depth; the next boundary op pulls deeper,
    // reusing everything already memoised. The incremental-precision primitive.
    elreal& refine(std::size_t depth) noexcept { if (depth > _depth) _depth = depth; return *this; }

    // stream(): the raw lazy state machine (advanced).
    const stream_type& stream() const noexcept { return _value; }

    bool iszero() const noexcept { return _value.is_empty(); }
    bool isneg()  const noexcept { return sign() < 0; }

    // sign(): -1 if negative, +1 otherwise (the most significant block's sign;
    // +1 for zero). scale(): the value's binary exponent (the leading block's
    // combined exponent E = scale_of_v + exp; 0 for zero).
    int sign() const noexcept {
        auto bl = _value.take(1);
        return (!bl.empty() && bl.front().sign() < 0) ? -1 : 1;
    }
    // int64_t (not int): elreal carries an unbounded integer<256> exponent, so a
    // narrow int cast could overflow. The host-FP attribute surface this feeds
    // (ldexp / significand) is bounded by FpType's exponent range, for which int64_t
    // has vast headroom; the full-width exponent stays reachable via block::exponent().
    int64_t scale() const noexcept {
        auto bl = _value.take(1);
        return bl.empty() ? 0 : static_cast<int64_t>(bl.front().exponent());
    }

private:
    stream_type _value;    // lazy, memoised block co-list
    std::size_t _depth;    // default pull depth for boundary operations
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
    for (const auto& blk : diff.take(d + 1)) {
        if (!blk.is_zero_block()) return blk.sign();   // first nonzero limb decides
    }
    return 0;   // no nonzero limb within d -> equal to precision
}
template <typename FpType> inline bool operator==(const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_cmp(a, b) == 0; }
template <typename FpType> inline bool operator!=(const elreal<FpType>& a, const elreal<FpType>& b) { return !(a == b); }
template <typename FpType> inline bool operator< (const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_cmp(a, b) <  0; }
template <typename FpType> inline bool operator> (const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_cmp(a, b) >  0; }
template <typename FpType> inline bool operator<=(const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_cmp(a, b) <= 0; }
template <typename FpType> inline bool operator>=(const elreal<FpType>& a, const elreal<FpType>& b) { return elreal_cmp(a, b) >= 0; }
template <typename FpType> inline bool operator==(const elreal<FpType>& a, double b) { return a == elreal<FpType>(b); }
template <typename FpType> inline bool operator!=(const elreal<FpType>& a, double b) { return !(a == elreal<FpType>(b)); }
template <typename FpType> inline bool operator< (const elreal<FpType>& a, double b) { return a <  elreal<FpType>(b); }
template <typename FpType> inline bool operator> (const elreal<FpType>& a, double b) { return a >  elreal<FpType>(b); }

// abs / fabs
template <typename FpType>
inline elreal<FpType> abs(const elreal<FpType>& a) {
    // sign from the leading (most significant) block
    auto bl = a.stream().take(1);
    bool neg = !bl.empty() && bl.front().sign() < 0;
    return neg ? -a : a;
}
template <typename FpType> inline elreal<FpType> fabs(const elreal<FpType>& a) { return abs(a); }

}} // namespace sw::universal

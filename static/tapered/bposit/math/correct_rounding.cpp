// correct_rounding.cpp: the b-posit elementary functions are correctly rounded (#1653, phase 2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every result must EQUAL the true value rounded once into the b-posit, to nearest with
// ties to even, saturating at minpos and maxpos.  The oracle shares no code with the
// library's engine:
//
//   1. long double libm (x87's 64-bit significand on x86) evaluates the function at the
//      exact argument; if 64 ulps either side round to the same b-posit, that is the answer;
//   2. otherwise ereal<8>, about 420 bits, evaluates it, and the b-posit is chosen by
//      comparing that value with the midpoints of the neighboring b-posits, formed exactly
//      in ereal from the encodings.
//
// The library decides most inputs from libm in double and the rest in quad-double.  Each
// function is run both ways: the default engine, and the quad-double engine alone
// (engine::qd_only), so the quad-double kernels are verified on every input and not only
// on the few the fast path leaves them.  16-bit configurations are exhaustive, 32-bit ones
// sampled.  bposit<64,6,5> never takes the fast path, and is sampled.  Of the 26 functions,
// erf, erfc and the gamma functions use the oracle's own series (ereal's are double stubs).
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::SpecificValue;
using sw::universal::bposit_math::engine;
using Oracle = sw::universal::ereal<8>;

// what an infinity or a zero from the long double reference means
enum class Range { any, finite, nonzero };

// erf, erfc and the gamma functions of ereal are evaluated in double, so the oracle carries
// its own, at ereal<8>'s 400-odd bits: the positive-term series of erf, erfc = 1 - erf (which
// keeps 180 bits even where erfc reaches minpos), and Stirling's series for lgamma after a
// shift to 100, with exact Bernoulli numbers (B_2 .. B_60, from sympy).
const Oracle& oracle_pi() {
	static const Oracle p = sw::universal::ereal_pi<8, double>();
	return p;
}

Oracle oracle_erf_positive(const Oracle& a) {
	// beyond 30, erf is 1 - O(e^-900): 1 for any b-posit, and ereal's exp(-a^2) is lost there
	if (a > Oracle(30.0)) return Oracle(1.0);
	const Oracle t = a * a * Oracle(2.0);
	Oracle term(1.0), sum(1.0);
	for (int n = 1; n < 4000; ++n) {
		term = term * t / Oracle(2.0 * n + 1.0);
		sum = sum + term;
		if (term < sum * Oracle(std::ldexp(1.0, -440))) break;
	}
	return Oracle(2.0) * a * exp(-(a * a)) * sum / sqrt(oracle_pi());
}
Oracle oracle_erf(const Oracle& v)  { return v < Oracle(0.0) ? -oracle_erf_positive(-v) : oracle_erf_positive(v); }
Oracle oracle_erfc(const Oracle& v) { return v < Oracle(0.0) ? Oracle(1.0) + oracle_erf_positive(-v) : Oracle(1.0) - oracle_erf_positive(v); }

struct fraction { const char* num; const char* den; };
constexpr fraction bernoulli[30] = {     // B_2k, k = 1 .. 30
	{ "1", "6" },
	{ "-1", "30" },
	{ "1", "42" },
	{ "-1", "30" },
	{ "5", "66" },
	{ "-691", "2730" },
	{ "7", "6" },
	{ "-3617", "510" },
	{ "43867", "798" },
	{ "-174611", "330" },
	{ "854513", "138" },
	{ "-236364091", "2730" },
	{ "8553103", "6" },
	{ "-23749461029", "870" },
	{ "8615841276005", "14322" },
	{ "-7709321041217", "510" },
	{ "2577687858367", "6" },
	{ "-26315271553053477373", "1919190" },
	{ "2929993913841559", "6" },
	{ "-261082718496449122051", "13530" },
	{ "1520097643918070802691", "1806" },
	{ "-27833269579301024235023", "690" },
	{ "596451111593912163277961", "282" },
	{ "-5609403368997817686249127547", "46410" },
	{ "495057205241079648212477525", "66" },
	{ "-801165718135489957347924991853", "1590" },
	{ "29149963634884862421418123812691", "798" },
	{ "-2479392929313226753685415739663229", "870" },
	{ "84483613348880041862046775994036021", "354" },
	{ "-1215233140483755572040304994079820246041491", "56786730" },
};

Oracle oracle_lgamma_positive(const Oracle& x) {
	Oracle y = x, product(1.0);
	while (y < Oracle(100.0)) { product = product * y; y = y + Oracle(1.0); }
	Oracle r = (y - Oracle(0.5)) * log(y) - y + log(Oracle(2.0) * oracle_pi()) * Oracle(0.5);
	const Oracle y2 = y * y;
	Oracle p = y;                                             // y^(2k - 1)
	for (int k = 1; k <= 30; ++k) {
		Oracle b, d;
		b.parse(bernoulli[k - 1].num);
		d.parse(bernoulli[k - 1].den);
		r = r + b / (d * Oracle(2.0 * k * (2.0 * k - 1.0)) * p);
		p = p * y2;
	}
	return r - log(product);
}
// reflection for x < 0, with sin(pi x) from the exact fractional part, which spares the oracle
// a reduction of pi x by 2 pi
Oracle oracle_lgamma(const Oracle& x) {
	if (x > Oracle(0.0)) return oracle_lgamma_positive(x);
	Oracle f = x - floor(x);
	if (f > Oracle(0.5)) f = Oracle(1.0) - f;
	return log(oracle_pi() / sin(oracle_pi() * f)) - oracle_lgamma_positive(Oracle(1.0) - x);
}
Oracle oracle_tgamma(const Oracle& x) {
	const Oracle g = exp(oracle_lgamma(x));
	if (x > Oracle(0.0)) return g;
	const double fl = double(floor(x));
	return (std::fmod(fl, 2.0) != 0.0) ? -g : g;
}

// the exact value of a b-posit in ereal
template<typename B>
Oracle exact(const B& x) {
	if (x.iszero()) return Oracle(0.0);
	const auto d = x.decode_fields();
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	const int e = d.scale - static_cast<int>(d.nf);
	Oracle v = Oracle(std::ldexp(static_cast<double>(sig >> 32), e + 32)) + Oracle(std::ldexp(static_cast<double>(sig & 0xFFFF'FFFFu), e));
	return d.negative ? -v : v;
}

// round an ereal value to the b-posit by midpoint comparison
template<typename B>
B round_oracle(const Oracle& v, bool snap_ties = false) {
	if (v.iszero()) return B(0);
	const bool negative = v < Oracle(0.0);
	const Oracle a = negative ? -v : v;
	const B maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	// starting guess; the midpoint walk below corrects it.  ereal converts to long double only
	// where long double is its own format (not MSVC, where it is double)
#if LONG_DOUBLE_SUPPORT
	B c(static_cast<long double>(a));
#else
	B c(static_cast<double>(a));
#endif
	if (c.isnar() || c.iszero()) c = minpos;
	// For the functions with exact results (snap_ties), a value within 2^-300 of a midpoint is
	// taken to be on it: ereal<8>'s functions carry some 400 bits, and such a function only
	// comes that close to a midpoint by being on one, like exp10(4) = 10000 between two 16-bit
	// b-posits.  A transcendental value is never on one, but can come closer than 2^-300:
	// atan(q) for a tiny q that is itself a midpoint differs from it by q^3/3.
	auto on = [&](const Oracle& v, const Oracle& mid) { return snap_ties && abs(v - mid) <= mid * Oracle(std::ldexp(1.0, -300)); };
	for (int guard = 0; guard < 4096; ++guard) {
		if (c != maxpos) {
			B up(c); ++up;
			const Oracle mid = (exact(c) + exact(up)) * Oracle(0.5);
			const bool tie = on(a, mid);
			if ((!tie && a > mid) || (tie && (c.raw() & 1u))) { c = up; continue; }
		}
		if (c != minpos) {
			B down(c); --down;
			const Oracle mid = (exact(c) + exact(down)) * Oracle(0.5);
			const bool tie = on(a, mid);
			if ((!tie && a < mid) || (tie && (c.raw() & 1u))) { c = down; continue; }
		}
		break;
	}
	return negative ? -c : c;
}

template<typename B>
B saturate(long double r, Range range, bool negative) {
	if (r != r) return B(SpecificValue::nar);
	if (std::isinf(r)) {
		if (range == Range::any) return B(SpecificValue::nar);
		return B(r > 0 ? SpecificValue::maxpos : SpecificValue::maxneg);
	}
	if (r == 0.0l && range == Range::nonzero) return B(negative ? SpecificValue::minneg : SpecificValue::minpos);
	return B(r);
}

// The correctly rounded reference.  The long double stage needs the argument exactly in a
// long double, which holds unless long double is double and the b-posit is 64 bits.
template<typename B>
B reference(long double r, Range range, bool negative, const std::function<Oracle()>& high, bool snap_ties = false, double ld_floor = 0.0) {
	if (r != r || std::isinf(r) || r == 0.0l) return saturate<B>(r, range, negative);
	// far outside the range the rounding is saturation, whatever the last bits: this also
	// spares ereal arguments its double-based exponent cannot hold
	const long double maxpos = static_cast<long double>(B(SpecificValue::maxpos));
	const long double minpos = static_cast<long double>(B(SpecificValue::minpos));
	if (std::fabs(r) > 4 * maxpos || std::fabs(r) < minpos / 4) return B(r);
	const int ld_digits = std::numeric_limits<long double>::digits;
	if (ld_digits >= static_cast<int>(B::fbits) + 1 + 8) {
		const long double err = std::ldexp(64.0l, std::ilogb(std::fmax(std::fabs(r), static_cast<long double>(ld_floor))) - (ld_digits - 1));
		const B lo(r - err), hi(r + err);
		if (lo == hi) return lo;
	}
	return round_oracle<B>(high(), snap_ties);
}

template<typename B>
bool long_double_exact() { return std::numeric_limits<long double>::digits >= static_cast<int>(B::fbits) + 1; }

struct Tally { int fails = 0; long checked = 0; };

template<typename B>
void judge(const char* name, const B& x, const B& got, const B& want, bool report, Tally& t) {
	++t.checked;
	if (got == want) return;
	++t.fails;
	if (report && t.fails < 12) {
		std::cerr << "FAIL " << sw::universal::type_tag(x) << ' ' << name << '(' << sw::universal::to_binary(x) << " = "
		          << std::setprecision(20) << static_cast<long double>(x) << ") = " << sw::universal::to_binary(got)
		          << ", want " << sw::universal::to_binary(want) << " (" << static_cast<long double>(want) << ")\n";
	}
}

// one unary function: the library at each engine, the long double reference, the ereal one
template<typename B>
struct Unary {
	const char* name;
	B (*fast)(const B&);
	B (*qd)(const B&);
	long double (*ld)(long double);
	Oracle (*hi)(const Oracle&);
	Range range;
	bool snap_ties = false;     // the function has exact results at midpoints
	double ld_floor = 0.0;      // libm's error is absolute below this magnitude (lgamma)
	bool gamma_poles = false;   // NaR at 0 and the negative integers
};

template<typename B>
std::vector<Unary<B>> functions() {
	namespace bm = sw::universal::bposit_math;
	using sw::universal::sqrt; using sw::universal::cbrt; using sw::universal::exp; using sw::universal::exp2;
	using sw::universal::exp10; using sw::universal::expm1; using sw::universal::log; using sw::universal::log2;
	using sw::universal::log10; using sw::universal::log1p; using sw::universal::sin; using sw::universal::cos;
	using sw::universal::tan; using sw::universal::asin; using sw::universal::acos; using sw::universal::atan;
	using sw::universal::sinh; using sw::universal::cosh; using sw::universal::tanh; using sw::universal::asinh;
	using sw::universal::acosh; using sw::universal::atanh;
#define BPOSIT_CR_ENTRY(f, range)                                                                     \
	Unary<B>{ #f, [](const B& x) { return bm::f<engine::libm_then_qd>(x); },                        \
	          [](const B& x) { return bm::f<engine::qd_only>(x); },                                \
	          [](long double v) { return std::f(v); }, [](const Oracle& v) { return f(v); }, range, false }
#define BPOSIT_CR_EXACT(f, range)                                                                     \
	Unary<B>{ #f, [](const B& x) { return bm::f<engine::libm_then_qd>(x); },                        \
	          [](const B& x) { return bm::f<engine::qd_only>(x); },                                \
	          [](long double v) { return std::f(v); }, [](const Oracle& v) { return f(v); }, range, true }
	return {
		BPOSIT_CR_EXACT(sqrt, Range::any),   BPOSIT_CR_EXACT(cbrt, Range::any),
		BPOSIT_CR_ENTRY(exp, Range::nonzero), BPOSIT_CR_EXACT(exp2, Range::nonzero),
		Unary<B>{ "exp10", [](const B& x) { return bm::exp10<engine::libm_then_qd>(x); }, [](const B& x) { return bm::exp10<engine::qd_only>(x); },
		          [](long double v) { return std::pow(10.0l, v); }, [](const Oracle& v) { return exp10(v); }, Range::nonzero, true },
		BPOSIT_CR_ENTRY(expm1, Range::finite),
		BPOSIT_CR_ENTRY(log, Range::any),    BPOSIT_CR_ENTRY(log2, Range::any),
		BPOSIT_CR_ENTRY(log10, Range::any),  BPOSIT_CR_ENTRY(log1p, Range::any),
		BPOSIT_CR_ENTRY(sin, Range::any),    BPOSIT_CR_ENTRY(cos, Range::any),
		BPOSIT_CR_ENTRY(tan, Range::finite), BPOSIT_CR_ENTRY(asin, Range::any),
		BPOSIT_CR_ENTRY(acos, Range::any),   BPOSIT_CR_ENTRY(atan, Range::any),
		BPOSIT_CR_ENTRY(sinh, Range::finite), BPOSIT_CR_ENTRY(cosh, Range::finite),
		BPOSIT_CR_ENTRY(tanh, Range::any),   BPOSIT_CR_ENTRY(asinh, Range::any),
		Unary<B>{ "erf", [](const B& x) { return bm::erf<engine::libm_then_qd>(x); }, [](const B& x) { return bm::erf<engine::qd_only>(x); },
		          [](long double v) { return std::erf(v); }, oracle_erf, Range::any, false },
		Unary<B>{ "erfc", [](const B& x) { return bm::erfc<engine::libm_then_qd>(x); }, [](const B& x) { return bm::erfc<engine::qd_only>(x); },
		          [](long double v) { return std::erfc(v); }, oracle_erfc, Range::nonzero, false },
		Unary<B>{ "tgamma", [](const B& x) { return bm::tgamma<engine::libm_then_qd>(x); }, [](const B& x) { return bm::tgamma<engine::qd_only>(x); },
		          [](long double v) { return std::tgamma(v); }, oracle_tgamma, Range::nonzero, true, 0.0, true },
		Unary<B>{ "lgamma", [](const B& x) { return bm::lgamma<engine::libm_then_qd>(x); }, [](const B& x) { return bm::lgamma<engine::qd_only>(x); },
		          [](long double v) { return std::lgamma(v); }, oracle_lgamma, Range::finite, false, 1.0, true },
		BPOSIT_CR_ENTRY(acosh, Range::any),  BPOSIT_CR_ENTRY(atanh, Range::any),
	};
#undef BPOSIT_CR_ENTRY
#undef BPOSIT_CR_EXACT
}

template<typename B>
void check(const Unary<B>& u, const B& x, bool qd_only, bool report, Tally& t) {
	B want;
	if (x.isnar() || (u.gamma_poles && sw::universal::bposit_math::gamma_pole(x))) want = B(SpecificValue::nar);
	else {
		const long double v = long_double_exact<B>() ? static_cast<long double>(x) : std::numeric_limits<long double>::quiet_NaN();
		const long double r = long_double_exact<B>() ? u.ld(v) : std::numeric_limits<long double>::quiet_NaN();
		if (long_double_exact<B>()) want = reference<B>(r, u.range, std::signbit(r), [&]() { return u.hi(exact(x)); }, u.snap_ties, u.ld_floor);
		else {
			// no exact long double: domain from the double approximation, value from ereal
			const long double approx = u.ld(static_cast<long double>(x));
			want = (approx != approx || std::isinf(approx)) ? saturate<B>(approx, u.range, false) : round_oracle<B>(u.hi(exact(x)), u.snap_ties);
		}
	}
	judge(u.name, x, qd_only ? u.qd(x) : u.fast(x), want, report, t);
}

template<typename B>
int VerifyExhaustive(bool qd_only, bool report) {
	Tally t;
	const std::uint64_t N = std::uint64_t(1) << B::nbits;
	for (const auto& u : functions<B>()) {
		for (std::uint64_t i = 0; i < N; ++i) {
			B x; x.setbits(i);
			check(u, x, qd_only, report, t);
		}
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << (qd_only ? " qd only" : " libm then qd") << ": " << t.checked << " results\n";
	return t.fails;
}

std::uint64_t lcg(std::uint64_t& state) {
	state = state * 6364136223846793005ull + 1442695040888963407ull;
	return state;
}

// random encodings, and as many again of moderate magnitude, where the functions are not saturated
template<typename B>
B sample(std::uint64_t& state, unsigned k) {
	B x;
	if (k & 1u) { x.setbits(lcg(state) >> (64 - B::nbits)); return x; }
	const double m = static_cast<double>(lcg(state) >> 11) / 9007199254740992.0;   // [0, 1)
	const int e = static_cast<int>(lcg(state) >> 59) - 16;                        // [-16, 15]
	x = B(std::ldexp(1.0 + m, e));
	return (lcg(state) >> 63) ? -x : x;
}

template<typename B>
int VerifySampled(unsigned count, bool qd_only, bool report) {
	Tally t;
	std::uint64_t state = 0x9E3779B97F4A7C15ull;
	for (const auto& u : functions<B>()) {
		for (unsigned k = 0; k < count; ++k) check(u, sample<B>(state, k), qd_only, report, t);
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << (qd_only ? " qd only" : " libm then qd") << ": " << t.checked << " results\n";
	return t.fails;
}

// pow, atan2 and hypot on random pairs
template<typename B>
int VerifyBinary(unsigned count, bool qd_only, bool report) {
	namespace bm = sw::universal::bposit_math;
	using sw::universal::pow; using sw::universal::atan2; using sw::universal::hypot;
	Tally t;
	std::uint64_t state = 0xD1B54A32D192ED03ull;
	const bool ld = long_double_exact<B>();
	for (unsigned k = 0; k < count; ++k) {
		const B x = sample<B>(state, k), y = sample<B>(state, k + 1);
		if (x.isnar() || y.isnar()) continue;
		const long double a = static_cast<long double>(x), b = static_cast<long double>(y);
		// pow: the special cases are pinned in mathlib.cpp; here a nonzero base
		if (!x.iszero() && !(x.isneg() && bm::integer_parity(y) < 0)) {
			const bool negative = x.isneg() && bm::integer_parity(y) == 1;
			const B got = qd_only ? bm::pow_exact<engine::qd_only>(x, bm::to_qd(y), bm::integer_parity(y), double(y), false) : pow(x, y);
			const B want = ld ? reference<B>(std::pow(a, b), Range::nonzero, negative, [&]() { return pow(exact(x), exact(y)); }, true)
			                  : round_oracle<B>(pow(exact(x), exact(y)), true);
			judge("pow", x, got, want, report, t);
		}
		if (!x.iszero() && !y.iszero()) {
			const B got = qd_only ? bm::atan2<engine::qd_only>(y, x) : atan2(y, x);
			const B want = ld ? reference<B>(std::atan2(b, a), Range::any, false, [&]() { return atan2(exact(y), exact(x)); })
			                  : round_oracle<B>(atan2(exact(y), exact(x)));
			judge("atan2", x, got, want, report, t);
		}
		{
			const B got = qd_only ? bm::hypot<engine::qd_only>(x, y) : hypot(x, y);
			const B want = ld ? reference<B>(std::hypot(a, b), Range::finite, false, [&]() { return hypot(exact(x), exact(y)); }, true)
			                  : round_oracle<B>(hypot(exact(x), exact(y)), true);
			judge("hypot", x, got, want, report, t);
		}
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << (qd_only ? " qd only" : " libm then qd") << " pow/atan2/hypot: " << t.checked << " results\n";
	return t.fails;
}

// floor, ceil, trunc, round, fmod, remainder and frac are exact, at every width: checked
// against the same operations in ereal, which are exact on these dyadic values
template<typename B>
int VerifyExactFunctions(unsigned count, bool report) {
	using namespace sw::universal;
	Tally t;
	std::uint64_t state = 0x2545F4914F6CDD1Dull;
	for (unsigned k = 0; k < count; ++k) {
		const B x = sample<B>(state, k), y = sample<B>(state, k + 1);
		if (x.isnar() || y.isnar()) continue;
		const Oracle ex = exact(x), ey = exact(y);
		judge("floor", x, floor(x), round_oracle<B>(floor(ex)), report, t);
		judge("ceil",  x, ceil(x),  round_oracle<B>(ceil(ex)),  report, t);
		judge("trunc", x, trunc(x), round_oracle<B>(trunc(ex)), report, t);
		judge("round", x, round(x), round_oracle<B>(round(ex)), report, t);
		judge("frac",  x, frac(x),  round_oracle<B>(ex - trunc(ex)), report, t);
		// fmod and remainder are exact in long double when it holds the operands, and rounding
		// that once is the reference
		if (!y.iszero() && long_double_exact<B>()) {
			const long double a = static_cast<long double>(x), b = static_cast<long double>(y);
			judge("fmod", x, fmod(x, y), B(std::fmod(a, b)), report, t);
			judge("remainder", x, remainder(x, y), B(std::remainder(a, b)), report, t);
		}
	}
	if (report) std::cout << "  " << type_tag(B()) << " exact functions: " << t.checked << " results\n";
	return t.fails;
}

// pow at exact midpoints: x^2 with x of (p + 2) / 2 bits lands between two b-posits, and
// must go to the even one; x^2 and x^0.5 at exact results must be exact
template<typename B>
int VerifyPowTies(bool report) {
	namespace bm = sw::universal::bposit_math;
	using sw::universal::pow;
	Tally t;
	for (int m = 3; m < 4096; m += 2) {
		const B x(m);
		if (double(x) != m) continue;
		const B sq(static_cast<long double>(m) * m);
		const Oracle e = exact(x) * exact(x);
		const B want = round_oracle<B>(e, true);
		judge("pow(x, 2)", x, pow(x, 2), want, report, t);
		judge("pow(x, 2) qd", x, bm::pow_exact<engine::qd_only>(x, sw::universal::qd(2.0), 0, 2.0, true), want, report, t);
		if (!want.isnar() && exact(want) == e) {
			judge("pow(x^2, 0.5)", want, pow(want, 0.5), x, report, t);
		}
		(void)sq;
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << " pow ties and exact results: " << t.checked << " results\n";
	return t.fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "bposit correct rounding";
	std::string test_tag    = "correctly rounded";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using bposit16_42 = bposit<16, 4, 2, std::uint16_t>;
	using bposit16_52 [[maybe_unused]] = bposit<16, 5, 2, std::uint16_t>;
	using bposit32_42 [[maybe_unused]] = bposit<32, 4, 2, std::uint32_t>;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16_42>(true, true), "bposit<16,4,2>", "qd only");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	// light enough for the sanitizer and coverage builds, which run unoptimized: the 64-bit
	// samples need the ereal oracle every time, which is slow there, so they are at level 2
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit16_42>(3000, false, reportTestCases), "bposit<16,4,2>", "sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit16_42>(400, true, reportTestCases), "bposit<16,4,2>", "sampled, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<bposit16_42>(1000, true, reportTestCases), "bposit<16,4,2>", "pow atan2 hypot, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyExactFunctions<bposit16_42>(1000, reportTestCases), "bposit<16,4,2>", "exact functions");
	nrOfFailedTestCases += ReportTestResult(VerifyPowTies<bposit16_42>(reportTestCases), "bposit<16,4,2>", "pow ties");
	nrOfFailedTestCases += ReportTestResult(VerifyExactFunctions<bposit64>(300, reportTestCases), "bposit<64,6,5>", "exact functions");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16_42>(false, reportTestCases), "bposit<16,4,2>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<bposit16_42>(5000, true, reportTestCases), "bposit<16,4,2>", "pow atan2 hypot, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyExactFunctions<bposit16_42>(5000, reportTestCases), "bposit<16,4,2>", "exact functions");
	nrOfFailedTestCases += ReportTestResult(VerifyExactFunctions<bposit64>(2000, reportTestCases), "bposit<64,6,5>", "exact functions");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16_42>(true, reportTestCases), "bposit<16,4,2>", "exhaustive, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16>(false, reportTestCases), "bposit<16,6,5>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16>(true, reportTestCases), "bposit<16,6,5>", "exhaustive, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16_52>(false, reportTestCases), "bposit<16,5,2>", "exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<bposit16_52>(true, reportTestCases), "bposit<16,5,2>", "exhaustive, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<bposit16>(20000, false, reportTestCases), "bposit<16,6,5>", "pow atan2 hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyPowTies<bposit16>(reportTestCases), "bposit<16,6,5>", "pow ties");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit64>(200, true, reportTestCases), "bposit<64,6,5>", "sampled, qd only");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit32_42>(20000, false, reportTestCases), "bposit<32,4,2>", "sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit32_42>(5000, true, reportTestCases), "bposit<32,4,2>", "sampled, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<bposit32>(5000, true, reportTestCases), "bposit<32,6,5>", "sampled, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<bposit32>(5000, true, reportTestCases), "bposit<32,6,5>", "pow atan2 hypot, qd only");
	nrOfFailedTestCases += ReportTestResult(VerifyPowTies<bposit32_42>(reportTestCases), "bposit<32,4,2>", "pow ties");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<bposit64>(1000, true, reportTestCases), "bposit<64,6,5>", "pow atan2 hypot");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

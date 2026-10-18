#pragma once
// ubox_trace.hpp: record the uncertainty box of every step of a computation and draw it as an HTML page
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// #196.  An enclosure type -- tile_interval<areal>, tile_interval<poxel>, or anything with
// the same accessors -- carries a box that grows and shrinks as a computation proceeds.
// A ubox_trace records that box at every named step, for several types side by side, and
// write_html() turns one or more traces into a self-contained HTML page (inline SVG, no
// scripts, no external files):
//   - a chart of the relative width of the box per step, one line per type, on a log
//     scale: a rising line is the box expanding, a falling one contracting
//   - a row per step that draws each type's box on a common number line around the
//     reference value, with the interval and its width in tiles
//
//     sw::universal::ubox_trace t("r1 as written", "(-b + sqrt(b^2 - 4ac)) / (2a)");
//     t.step("b^2", 10000.0);                   // the steps, with their exact values
//     t.step("sqrt(d)", std::sqrt(9976.0));
//     auto& s = t.series("areal<32,8>");       // then one series per type, in step order
//     auto bb = s.record(b * b);                // record() returns its argument
//     auto sd = s.record(sqrt(bb - fac));
//     std::ofstream f("ubox.html");
//     sw::universal::write_html(f, "page title", "intro text", { t });
//
// The interval needs lower<Real>(), upper<Real>(), lower_open(), upper_open(), isnan() and
// isexact(); str(int) and integral lo_key()/hi_key() are used when present.  A type that
// cannot provide them can record a ubox_box directly.
#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace sw { namespace universal {

// one box: [lo, hi] with open or closed ends, as long double
struct ubox_box {
	long double lo{ 0 }, hi{ 0 };
	bool lo_open{ false }, hi_open{ false };
	bool nan{ false };
	bool exact{ false };          // a single exact value
	long long tiles{ -1 };        // width in tiles (lattice points and open gaps), -1 when unknown
	std::string text;             // the interval as the type prints it

	bool bounded() const noexcept { return !nan && std::isfinite(lo) && std::isfinite(hi); }
	// (hi - lo) / max(|lo|, |hi|): 0 for an exact box, inf when unbounded or nan
	long double relative_width() const noexcept {
		if (nan || !bounded()) return std::numeric_limits<long double>::infinity();
		if (exact) return 0.0L;
		const long double m = std::max(std::fabs(lo), std::fabs(hi));
		const long double w = hi - lo;
		if (m == 0.0L) return 0.0L;
		// an inexact box narrower than long double can resolve still has a width
		return w > 0.0L ? w / m : std::numeric_limits<long double>::epsilon();
	}
	bool contains(long double v) const noexcept {
		if (nan) return false;
		return (lo_open ? lo < v : lo <= v) && (hi_open ? v < hi : v <= hi);
	}
};

namespace detail {

template<typename I>
std::string ubox_text(const I& x, int precision) {
	if constexpr (requires { { x.str(precision) } -> std::convertible_to<std::string>; }) {
		return x.str(precision);
	}
	else {
		std::ostringstream s;
		s.precision(precision);
		if (x.isnan()) return "nan";
		s << (x.lower_open() ? '(' : '[') << x.template lower<long double>() << ", " << x.template upper<long double>() << (x.upper_open() ? ')' : ']');
		return s.str();
	}
}

inline std::string html_escape(const std::string& s) {
	std::string r;
	r.reserve(s.size());
	for (char c : s) {
		switch (c) {
		case '&': r += "&amp;"; break;
		case '<': r += "&lt;"; break;
		case '>': r += "&gt;"; break;
		case '"': r += "&quot;"; break;
		default:  r += c; break;
		}
	}
	return r;
}

inline std::string fmt(long double v, int precision = 4, bool sci = false) {
	std::ostringstream s;
	if (sci) s << std::scientific;
	s.precision(precision);
	s << v;
	return s.str();
}

}  // namespace detail

// the box of an enclosure type
template<typename I>
ubox_box make_ubox(const I& x, int precision = 10) {
	ubox_box b;
	b.nan = x.isnan();
	b.text = detail::ubox_text(x, precision);
	if (b.nan) return b;
	b.lo = x.template lower<long double>();
	b.hi = x.template upper<long double>();
	b.lo_open = x.lower_open();
	b.hi_open = x.upper_open();
	b.exact = x.isexact();
	if constexpr (requires { x.lo_key(); x.hi_key(); }) {
		using K = std::decay_t<decltype(x.lo_key())>;
		if constexpr (std::is_integral_v<K>) b.tiles = static_cast<long long>(x.hi_key() - x.lo_key()) + 1;
	}
	return b;
}
inline ubox_box make_ubox(const ubox_box& b, int = 10) { return b; }

// the boxes of one type, one per step
struct ubox_series {
	std::string name;
	std::vector<ubox_box> boxes;
	int precision{ 10 };

	template<typename I>
	const I& record(const I& x) { boxes.push_back(make_ubox(x, precision)); return x; }
};

// a computation: its named steps (with reference values) and one series per type
class ubox_trace {
public:
	struct step_info {
		std::string label;
		long double reference;   // the exact value of the step, NaN when not given
	};

	ubox_trace(std::string title, std::string formula = "", std::string note = "")
		: _title(std::move(title)), _formula(std::move(formula)), _note(std::move(note)) {}

	void step(const std::string& label, long double reference = std::numeric_limits<long double>::quiet_NaN()) {
		_steps.push_back({ label, reference });
	}
	// the series of this name, created on first use; pass precision for its interval text
	ubox_series& series(const std::string& name, int precision = 10) {
		for (auto& s : _series) if (s.name == name) return s;
		_series.push_back({ name, {}, precision });
		return _series.back();
	}

	const std::string& title() const noexcept { return _title; }
	const std::string& formula() const noexcept { return _formula; }
	const std::string& note() const noexcept { return _note; }
	const std::vector<step_info>& steps() const noexcept { return _steps; }
	const std::vector<ubox_series>& all_series() const noexcept { return _series; }

	// every series recorded exactly one box per step
	bool complete() const noexcept {
		for (const auto& s : _series) if (s.boxes.size() != _steps.size()) return false;
		return !_series.empty();
	}

private:
	std::string _title, _formula, _note;
	std::vector<step_info> _steps;
	std::vector<ubox_series> _series;
};

namespace detail {

constexpr int ubox_palette_size = 6;

// the chart: log10 of the relative width per step, one polyline per series.  Exact boxes sit
// on a lane below the plot and unbounded ones on a lane above it.
inline void write_width_chart(std::ostream& o, const ubox_trace& t) {
	const auto& steps = t.steps();
	const auto& series = t.all_series();
	const std::size_t n = steps.size();
	if (n == 0) return;

	// y range in decades over the finite, nonzero widths
	long double ymin = 0.0L, ymax = 0.0L;
	bool any = false;
	for (const auto& s : series) for (const auto& b : s.boxes) {
		const long double rw = b.relative_width();
		if (rw > 0.0L && std::isfinite(rw)) {
			const long double d = std::log10(rw);
			if (!any) { ymin = ymax = d; any = true; }
			ymin = std::min(ymin, d);
			ymax = std::max(ymax, d);
		}
	}
	int top = any ? static_cast<int>(std::ceil(ymax)) : 0;
	int bot = any ? static_cast<int>(std::floor(ymin)) : -1;
	if (top <= bot) bot = top - 1;

	const double W = 760, H = 340, left = 84, right = 150, ptop = 34, pbot = 96;
	const double lane = 14, gap = 12;                                  // the inf and exact lanes
	const double y0 = ptop + lane + gap, y1 = H - pbot - lane - gap;   // plot area for finite widths
	const double xstep = n > 1 ? (W - left - right) / double(n - 1) : 0.0;
	auto X = [&](std::size_t i) { return n > 1 ? left + xstep * double(i) : left + (W - left - right) / 2; };
	auto Y = [&](const ubox_box& b) -> double {
		const long double rw = b.relative_width();
		if (!std::isfinite(rw)) return ptop + lane / 2;                 // unbounded or nan
		if (rw == 0.0L) return H - pbot - lane / 2;                    // exact
		const long double d = std::log10(rw);
		return y1 - (y1 - y0) * double((d - bot) / (top - bot));
	};

	o << "<svg class=\"chart\" viewBox=\"0 0 " << W << ' ' << H << "\" role=\"img\" aria-label=\"relative width of the box per step\">\n";
	// lanes and grid
	o << "<rect class=\"lane\" x=\"" << left << "\" y=\"" << ptop << "\" width=\"" << W - left - right << "\" height=\"" << lane << "\"/>\n";
	o << "<rect class=\"lane\" x=\"" << left << "\" y=\"" << H - pbot - lane << "\" width=\"" << W - left - right << "\" height=\"" << lane << "\"/>\n";
	o << "<text class=\"axis\" x=\"" << left - 6 << "\" y=\"" << ptop + lane / 2 + 4 << "\" text-anchor=\"end\">unbounded</text>\n";
	o << "<text class=\"axis\" x=\"" << left - 6 << "\" y=\"" << H - pbot - lane / 2 + 4 << "\" text-anchor=\"end\">exact</text>\n";
	const int span = top - bot;
	const int every = span > 12 ? (span + 11) / 12 : 1;
	for (int d = bot; d <= top; ++d) {
		if ((d - bot) % every != 0 && d != top) continue;
		const double y = y1 - (y1 - y0) * double(d - bot) / double(top - bot);
		o << "<line class=\"grid\" x1=\"" << left << "\" x2=\"" << W - right << "\" y1=\"" << y << "\" y2=\"" << y << "\"/>\n";
		o << "<text class=\"axis\" x=\"" << left - 6 << "\" y=\"" << y + 4 << "\" text-anchor=\"end\">1e" << d << "</text>\n";
	}
	for (std::size_t i = 0; i < n; ++i) {
		const double x = X(i);
		o << "<line class=\"tick\" x1=\"" << x << "\" x2=\"" << x << "\" y1=\"" << H - pbot << "\" y2=\"" << H - pbot + 4 << "\"/>\n";
		o << "<text class=\"axis step\" transform=\"translate(" << x + 4 << ',' << H - pbot + 10 << ") rotate(35)\">" << html_escape(steps[i].label) << "</text>\n";
	}
	o << "<text class=\"axis\" x=\"" << left << "\" y=\"" << ptop - 12 << "\">relative width of the box, (hi - lo) / max(|lo|, |hi|)</text>\n";
	// one polyline per series
	for (std::size_t k = 0; k < series.size(); ++k) {
		const auto& s = series[k];
		const int c = int(k % ubox_palette_size);
		o << "<polyline class=\"line c" << c << "\" points=\"";
		for (std::size_t i = 0; i < s.boxes.size() && i < n; ++i) o << X(i) << ',' << Y(s.boxes[i]) << ' ';
		o << "\"/>\n";
		for (std::size_t i = 0; i < s.boxes.size() && i < n; ++i) {
			const auto& b = s.boxes[i];
			o << "<circle class=\"dot c" << c << "\" cx=\"" << X(i) << "\" cy=\"" << Y(b) << "\" r=\"3.5\"><title>"
			  << html_escape(s.name) << " | " << html_escape(steps[i].label) << " | " << html_escape(b.text);
			if (b.tiles >= 0) o << " | " << b.tiles << " tiles";
			o << "</title></circle>\n";
		}
	}
	o << "</svg>\n";
}

// the boxes of one step on a common number line, centered on the reference value (or the
// midpoint of the widest box) and scaled to the widest bounded box
inline void write_step_strip(std::ostream& o, const ubox_trace& t, std::size_t i) {
	const auto& series = t.all_series();
	const long double ref = t.steps()[i].reference;
	const double W = 420, pad = 10, row = 16, H = row * double(series.size()) + 24;

	long double c = ref;
	if (!std::isfinite(c)) {
		long double best = -1;
		for (const auto& s : series) {
			const auto& b = s.boxes[i];
			if (b.bounded() && b.hi - b.lo > best) { best = b.hi - b.lo; c = (b.lo + b.hi) / 2; }
		}
		if (!std::isfinite(c)) c = 0;
	}
	long double half = 0;
	for (const auto& s : series) {
		const auto& b = s.boxes[i];
		if (b.bounded()) half = std::max({ half, std::fabs(b.lo - c), std::fabs(b.hi - c) });
	}
	const double x0 = pad, x1 = W - pad, xc = (x0 + x1) / 2;
	auto X = [&](long double v) -> double {
		if (half == 0) return xc;
		const double x = xc + (x1 - x0) / 2 * double((v - c) / half);
		return std::clamp(x, x0, x1);
	};

	o << "<svg class=\"strip\" viewBox=\"0 0 " << W << ' ' << H << "\" role=\"img\" aria-label=\"boxes of step " << html_escape(t.steps()[i].label) << "\">\n";
	o << "<line class=\"axisline\" x1=\"" << x0 << "\" x2=\"" << x1 << "\" y1=\"" << H - 20 << "\" y2=\"" << H - 20 << "\"/>\n";
	if (std::isfinite(ref)) o << "<line class=\"ref\" x1=\"" << X(ref) << "\" x2=\"" << X(ref) << "\" y1=\"2\" y2=\"" << H - 20 << "\"><title>exact value " << fmt(ref, 17) << "</title></line>\n";
	if (half > 0) {
		o << "<text class=\"axis\" x=\"" << x0 << "\" y=\"" << H - 2 << "\">-" << fmt(half, 2, true) << "</text>\n";
		o << "<text class=\"axis\" x=\"" << x1 << "\" y=\"" << H - 2 << "\" text-anchor=\"end\">+" << fmt(half, 2, true) << "</text>\n";
	}
	o << "<text class=\"axis\" x=\"" << xc << "\" y=\"" << H - 2 << "\" text-anchor=\"middle\">" << fmt(c, 10) << "</text>\n";
	for (std::size_t k = 0; k < series.size(); ++k) {
		const auto& b = series[k].boxes[i];
		const int cl = int(k % ubox_palette_size);
		const double y = 3 + row * double(k), h = row - 6;
		o << "<g class=\"c" << cl << "\"><title>" << html_escape(series[k].name) << ' ' << html_escape(b.text);
		if (b.tiles >= 0) o << " | " << b.tiles << " tiles";
		o << "</title>\n";
		if (b.nan) {
			o << "<text class=\"axis\" x=\"" << xc << "\" y=\"" << y + h << "\" text-anchor=\"middle\">nan</text></g>\n";
			continue;
		}
		const double xl = std::isfinite(b.lo) ? X(b.lo) : x0;
		const double xr = std::isfinite(b.hi) ? X(b.hi) : x1;
		double bw = xr - xl;
		double bx = xl;
		if (bw < 2) { bx = (xl + xr) / 2 - 1; bw = 2; }   // narrower than the strip resolves
		o << "<rect class=\"box\" x=\"" << bx << "\" y=\"" << y << "\" width=\"" << bw << "\" height=\"" << h << "\"/>\n";
		// closed ends get a solid cap, open ends a hollow one, infinite ends an arrow
		auto cap = [&](double x, bool open, bool inf, bool left) {
			if (inf) {
				const double d = left ? -6 : 6;
				o << "<path class=\"arrow\" d=\"M" << x + d << ',' << y + h / 2 << " L" << x << ',' << y << " L" << x << ',' << y + h << " Z\"/>\n";
			}
			else if (bw > 6) {
				o << "<line class=\"" << (open ? "cap open" : "cap") << "\" x1=\"" << x << "\" x2=\"" << x << "\" y1=\"" << y - 2 << "\" y2=\"" << y + h + 2 << "\"/>\n";
			}
		};
		cap(bx, b.lo_open, !std::isfinite(b.lo), true);
		cap(bx + bw, b.hi_open, !std::isfinite(b.hi), false);
		o << "</g>\n";
	}
	o << "</svg>\n";
}

inline const char* ubox_style() {
	return R"css(
:root { --bg:#ffffff; --fg:#1d1f23; --muted:#5f6670; --grid:#e3e6ea; --lane:#f3f4f6; --ref:#1d1f23; --card:#fafbfc;
  --c0:#2563eb; --c1:#d97706; --c2:#059669; --c3:#c026d3; --c4:#dc2626; --c5:#0891b2; }
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) { --bg:#15171a; --fg:#e7e9ec; --muted:#9aa1ab; --grid:#2b2f35; --lane:#1f2227; --ref:#e7e9ec; --card:#1b1e22;
  --c0:#60a5fa; --c1:#fbbf24; --c2:#34d399; --c3:#e879f9; --c4:#f87171; --c5:#22d3ee; } }
:root[data-theme="dark"] { --bg:#15171a; --fg:#e7e9ec; --muted:#9aa1ab; --grid:#2b2f35; --lane:#1f2227; --ref:#e7e9ec; --card:#1b1e22;
  --c0:#60a5fa; --c1:#fbbf24; --c2:#34d399; --c3:#e879f9; --c4:#f87171; --c5:#22d3ee; }
* { box-sizing: border-box; }
body { margin:0; background:var(--bg); color:var(--fg); font:15px/1.5 system-ui, -apple-system, "Segoe UI", sans-serif; }
main { max-width: 1100px; margin: 0 auto; padding: 24px 16px 64px; }
h1 { font-size: 1.6rem; margin: 0 0 8px; }
h2 { font-size: 1.2rem; margin: 40px 0 4px; }
p.intro, p.note { color: var(--muted); max-width: 80ch; }
code, .mono { font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; font-size: 0.85rem; }
.formula { font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; margin: 4px 0 8px; }
.legend { display:flex; flex-wrap:wrap; gap: 6px 18px; margin: 10px 0; }
.legend span { display:inline-flex; align-items:center; gap:6px; }
.legend i { width: 14px; height: 10px; border-radius: 2px; display:inline-block; }
.card { background: var(--card); border: 1px solid var(--grid); border-radius: 8px; padding: 12px; overflow-x: auto; }
svg { display:block; max-width: 100%; height: auto; }
svg text { fill: var(--muted); font-size: 11px; font-family: system-ui, sans-serif; }
svg.strip text { font-size: 15px; }
.grid { stroke: var(--grid); }
.lane { fill: var(--lane); }
.tick, .axisline { stroke: var(--muted); }
.line { fill: none; stroke-width: 2; }
.ref { stroke: var(--ref); stroke-dasharray: 3 2; }
.box { fill-opacity: 0.35; stroke-width: 1; }
.cap { stroke-width: 2.5; }
.cap.open { stroke-width: 1; stroke-dasharray: 2 2; }
table { border-collapse: collapse; width: 100%; margin-top: 12px; }
th, td { text-align: left; vertical-align: top; padding: 6px 8px; border-top: 1px solid var(--grid); }
th { color: var(--muted); font-weight: 600; font-size: 0.85rem; }
td.step { white-space: nowrap; }
td.boxes { min-width: 260px; width: 420px; }
td.texts div { white-space: nowrap; }
.tiles { color: var(--muted); }
@media (max-width: 720px) { td.texts { display:none; } }
)css"
	       R"css(
.c0 { color: var(--c0); stroke: var(--c0); fill: var(--c0); } .line.c0 { fill: none; }
.c1 { color: var(--c1); stroke: var(--c1); fill: var(--c1); } .line.c1 { fill: none; }
.c2 { color: var(--c2); stroke: var(--c2); fill: var(--c2); } .line.c2 { fill: none; }
.c3 { color: var(--c3); stroke: var(--c3); fill: var(--c3); } .line.c3 { fill: none; }
.c4 { color: var(--c4); stroke: var(--c4); fill: var(--c4); } .line.c4 { fill: none; }
.c5 { color: var(--c5); stroke: var(--c5); fill: var(--c5); } .line.c5 { fill: none; }
g.c0 .box, g.c1 .box, g.c2 .box, g.c3 .box, g.c4 .box, g.c5 .box { stroke: currentColor; fill: currentColor; }
g .cap, g .arrow { stroke: currentColor; fill: currentColor; }
td.texts div.c0, td.texts div.c1, td.texts div.c2, td.texts div.c3, td.texts div.c4, td.texts div.c5 { stroke: none; fill: none; }
)css";
}

inline void write_trace(std::ostream& o, const ubox_trace& t) {
	const auto& series = t.all_series();
	o << "<section>\n<h2>" << html_escape(t.title()) << "</h2>\n";
	if (!t.formula().empty()) o << "<div class=\"formula\">" << html_escape(t.formula()) << "</div>\n";
	if (!t.note().empty()) o << "<p class=\"note\">" << html_escape(t.note()) << "</p>\n";
	o << "<div class=\"legend\">";
	for (std::size_t k = 0; k < series.size(); ++k)
		o << "<span class=\"c" << k % ubox_palette_size << "\"><i style=\"background:currentColor\"></i>" << html_escape(series[k].name) << "</span>";
	o << "</div>\n<div class=\"card\">\n";
	write_width_chart(o, t);
	o << "<table>\n<tr><th>step</th><th>exact value</th><th>boxes on a common scale</th><th>intervals</th></tr>\n";
	for (std::size_t i = 0; i < t.steps().size(); ++i) {
		const auto& st = t.steps()[i];
		o << "<tr><td class=\"step mono\">" << html_escape(st.label) << "</td><td class=\"mono\">"
		  << (std::isfinite(st.reference) ? fmt(st.reference, 12) : std::string("-")) << "</td><td class=\"boxes\">\n";
		write_step_strip(o, t, i);
		o << "</td><td class=\"texts mono\">";
		for (std::size_t k = 0; k < series.size(); ++k) {
			const auto& b = series[k].boxes[i];
			o << "<div class=\"c" << k % ubox_palette_size << "\">" << html_escape(b.text);
			if (b.tiles >= 0) o << " <span class=\"tiles\">" << b.tiles << (b.tiles == 1 ? " tile" : " tiles") << "</span>";
			o << "</div>";
		}
		o << "</td></tr>\n";
	}
	o << "</table>\n</div>\n</section>\n";
}

}  // namespace detail

// write one self-contained HTML page with a section per trace; throws std::invalid_argument
// when a trace has a series that did not record one box per step
inline void write_html(std::ostream& o, const std::string& title, const std::string& intro, const std::vector<ubox_trace>& traces) {
	for (const auto& t : traces)
		if (!t.complete()) throw std::invalid_argument("ubox_trace '" + t.title() + "': every series must record one box per step");
	o << "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
	  << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	  << "<title>" << detail::html_escape(title) << "</title>\n<style>" << detail::ubox_style() << "</style>\n</head>\n<body>\n<main>\n"
	  << "<h1>" << detail::html_escape(title) << "</h1>\n";
	if (!intro.empty()) o << "<p class=\"intro\">" << detail::html_escape(intro) << "</p>\n";
	for (const auto& t : traces) detail::write_trace(o, t);
	o << "</main>\n</body>\n</html>\n";
}

}}  // namespace sw::universal

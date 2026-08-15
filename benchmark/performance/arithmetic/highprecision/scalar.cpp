//  scalar.cpp : scalar operator performance of the multi-component types: dd, dd_cascade, td_cascade, qd, qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// issue #1315: the cascade types are the modernized multi-component implementations built on
// floatcascade<N>, and the intent is that they replace the hand-specialized classic dd and qd.
// This program measures the per-operation cost of that generalization.
//
// Every workload is a dependent chain: the result of one operation is the input to the next.
// That measures operation latency, which is what a renormalization-heavy implementation pays,
// rather than the throughput a superscalar core can extract from independent work. The kernel
// benchmarks (kernels.cpp) cover the throughput side.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "hp_types.hpp"

namespace {

	constexpr std::size_t TABLE_SIZE = 64;   // power of two: the index wrap is a mask, not a division

	// operand tables are built once, outside the timed region, from runtime data so that the
	// compiler cannot constant-fold the workload away
	template<typename Scalar>
	const std::vector<Scalar>& operands() {
		static const std::vector<Scalar> table = [] {
			std::vector<double> data = hpbench::sampleData(TABLE_SIZE);
			std::vector<Scalar> t(TABLE_SIZE);
			for (std::size_t i = 0; i < TABLE_SIZE; ++i) t[i] = Scalar(data[i]);
			return t;
			}();
		return table;
	}

	// the multiplicative table alternates x and 1/x so that a long dependent chain of multiplies
	// or divides stays in the neighborhood of 1.0 instead of overflowing to infinity
	template<typename Scalar>
	const std::vector<Scalar>& reciprocalPairs() {
		static const std::vector<Scalar> table = [] {
			std::vector<double> data = hpbench::sampleData(TABLE_SIZE / 2, 0xBEEFull);
			std::vector<Scalar> t(TABLE_SIZE);
			for (std::size_t i = 0; i < TABLE_SIZE / 2; ++i) {
				t[2 * i] = Scalar(data[i]);
				t[2 * i + 1] = Scalar(1.0) / Scalar(data[i]);
			}
			return t;
			}();
		return table;
	}

	// the additive table alternates +x and -x for the same reason
	template<typename Scalar>
	const std::vector<Scalar>& oppositePairs() {
		static const std::vector<Scalar> table = [] {
			std::vector<double> data = hpbench::sampleData(TABLE_SIZE / 2, 0xFEEDull);
			std::vector<Scalar> t(TABLE_SIZE);
			for (std::size_t i = 0; i < TABLE_SIZE / 2; ++i) {
				t[2 * i] = Scalar(data[i]);
				t[2 * i + 1] = -Scalar(data[i]);
			}
			return t;
			}();
		return table;
	}

	const std::vector<double>& doubleOperands() {
		static const std::vector<double> data = hpbench::sampleData(TABLE_SIZE);
		return data;
	}

	// decimal strings wide enough to exercise the full significand of a qd
	const std::vector<std::string>& decimalStrings() {
		static const std::vector<std::string> strings = [] {
			hpbench::Lcg rng(0x5EEDull);
			std::vector<std::string> s(TABLE_SIZE);
			for (std::size_t i = 0; i < TABLE_SIZE; ++i) {
				std::ostringstream oss;
				oss << "1." ;
				for (int digit = 0; digit < 62; ++digit) oss << char('0' + (rng.next() % 10));
				oss << "e+0" << (rng.next() % 9);
				s[i] = oss.str();
			}
			return s;
			}();
		return strings;
	}

	// string -> value: the number systems expose assign(const std::string&); double does not
	template<typename Scalar>
	inline void fromString(Scalar& v, const std::string& s) { v.assign(s); }
	inline void fromString(double& v, const std::string& s) { v = std::stod(s); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// workloads

	template<typename Scalar>
	void ConstructionWorkload(std::size_t nrOps) {
		const std::vector<double>& data = doubleOperands();
		for (std::size_t i = 0; i < nrOps; ++i) {
			Scalar a(data[i & (TABLE_SIZE - 1)]);
			hpbench::consume(a);
		}
	}

	template<typename Scalar>
	void CopyConstructionWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		for (std::size_t i = 0; i < nrOps; ++i) {
			Scalar a(table[i & (TABLE_SIZE - 1)]);
			hpbench::consume(a);
		}
	}

	template<typename Scalar>
	void AssignmentWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		Scalar a(0.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			a = table[i & (TABLE_SIZE - 1)];
			hpbench::consume(a);
		}
	}

	template<typename Scalar>
	void AdditionWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = oppositePairs<Scalar>();
		Scalar acc(1.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			acc = acc + table[i & (TABLE_SIZE - 1)];
		}
		hpbench::consume(acc);
	}

	template<typename Scalar>
	void SubtractionWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = oppositePairs<Scalar>();
		Scalar acc(1.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			acc = acc - table[i & (TABLE_SIZE - 1)];
		}
		hpbench::consume(acc);
	}

	template<typename Scalar>
	void MultiplicationWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = reciprocalPairs<Scalar>();
		Scalar acc(1.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			acc = acc * table[i & (TABLE_SIZE - 1)];
		}
		hpbench::consume(acc);
	}

	template<typename Scalar>
	void DivisionWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = reciprocalPairs<Scalar>();
		Scalar acc(1.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			acc = acc / table[i & (TABLE_SIZE - 1)];
		}
		hpbench::consume(acc);
	}

	template<typename Scalar>
	void ComparisonWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		Scalar pivot(1.25);
		std::size_t count{ 0 };
		for (std::size_t i = 0; i < nrOps; ++i) {
			if (table[i & (TABLE_SIZE - 1)] < pivot) ++count;
		}
		hpbench::consume(count);
	}

	template<typename Scalar>
	void EqualityWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		Scalar pivot(1.25);
		std::size_t count{ 0 };
		for (std::size_t i = 0; i < nrOps; ++i) {
			if (table[i & (TABLE_SIZE - 1)] == pivot) ++count;
		}
		hpbench::consume(count);
	}

	template<typename Scalar>
	void ToDoubleWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		double sum{ 0.0 };
		for (std::size_t i = 0; i < nrOps; ++i) {
			sum += double(table[i & (TABLE_SIZE - 1)]);
		}
		hpbench::consume(sum);
	}

	template<typename Scalar>
	void FromDoubleWorkload(std::size_t nrOps) {
		const std::vector<double>& data = doubleOperands();
		Scalar a(0.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			a = data[i & (TABLE_SIZE - 1)];
			hpbench::consume(a);
		}
	}

	template<typename Scalar>
	void ToStringWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		// the stream is hoisted out of the loop: constructing an ostringstream costs more than
		// formatting a double, and that would swamp the number being measured
		std::ostringstream oss;
		oss << std::setprecision(60);
		std::size_t characters{ 0 };
		for (std::size_t i = 0; i < nrOps; ++i) {
			oss.str(std::string());
			oss << table[i & (TABLE_SIZE - 1)];
			characters += oss.str().size();
		}
		hpbench::consume(characters);
	}

	template<typename Scalar>
	void FromStringWorkload(std::size_t nrOps) {
		const std::vector<std::string>& strings = decimalStrings();
		Scalar a(0.0);
		for (std::size_t i = 0; i < nrOps; ++i) {
			fromString(a, strings[i & (TABLE_SIZE - 1)]);
			hpbench::consume(a);
		}
	}

	// the reference floor for every other measurement in this program: an empty loop that still
	// stores through the volatile sink. Anything measuring at this cost was optimized away.
	template<typename Scalar>
	void SinkOnlyWorkload(std::size_t nrOps) {
		const std::vector<Scalar>& table = operands<Scalar>();
		for (std::size_t i = 0; i < nrOps; ++i) {
			hpbench::consume(table[i & (TABLE_SIZE - 1)]);
		}
	}

	template<typename Scalar>
	void measureType(hpbench::Suite& suite, const std::string& type) {
		suite.measure("sink only (floor)", type, SinkOnlyWorkload<Scalar>);
		suite.measure("construct", type, ConstructionWorkload<Scalar>);
		suite.measure("copy construct", type, CopyConstructionWorkload<Scalar>);
		suite.measure("assign", type, AssignmentWorkload<Scalar>);
		suite.measure("add", type, AdditionWorkload<Scalar>);
		suite.measure("subtract", type, SubtractionWorkload<Scalar>);
		suite.measure("multiply", type, MultiplicationWorkload<Scalar>);
		suite.measure("divide", type, DivisionWorkload<Scalar>);
		suite.measure("compare (<)", type, ComparisonWorkload<Scalar>);
		suite.measure("compare (==)", type, EqualityWorkload<Scalar>);
		suite.measure("convert to double", type, ToDoubleWorkload<Scalar>);
		suite.measure("convert from double", type, FromDoubleWorkload<Scalar>);
		suite.measure("convert to string", type, ToStringWorkload<Scalar>);
		suite.measure("convert from string", type, FromStringWorkload<Scalar>);
	}

}  // anonymous namespace

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	hpbench::Suite suite("multi-component scalar operator performance (universal#1315)", hpbench::targetWindow(argc, argv));
	suite.reportEnvironment();

	measureType<double>(suite, "double");
	measureType<dd>(suite, "dd");
	measureType<dd_cascade>(suite, "dd_cascade");
	measureType<td_cascade>(suite, "td_cascade");
	measureType<qd>(suite, "qd");
	measureType<qd_cascade>(suite, "qd_cascade");

	suite.reportLatency();
	suite.reportThroughput();
	suite.reportRatios(hpbench::cascadeVsClassic());

	std::cout << "\ndone.\n";
	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}

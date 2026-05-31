#pragma once
// expression_graph.hpp: DAG-based expression graph for POP precision analysis
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The ExprGraph builds a directed acyclic graph (DAG) of arithmetic operations
// and performs iterative fixpoint analysis to determine minimum bit requirements
// at each node. The analysis proceeds in three phases:
//
//   1. Forward pass: propagate precision from inputs to outputs
//   2. Backward pass: propagate requirements from outputs to inputs
//   3. Finalize: nsb_final = max(nsb_forward, nsb_backward)
//
// The iterative fixpoint repeats forward+backward until convergence, handling
// the case where some nodes have requirements from multiple output consumers.
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021, Chapters 4-5.

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cassert>

#include <universal/mixedprecision/transfer.hpp>
#include <universal/mixedprecision/ufp.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/type_advisor.hpp>

namespace sw { namespace universal {

enum class OpKind : uint8_t {
	Constant,
	Variable,
	Add,
	Sub,
	Mul,
	Div,
	Neg,
	Abs,
	Sqrt
};

inline const char* to_string(OpKind op) {
	switch (op) {
		case OpKind::Constant: return "const";
		case OpKind::Variable: return "var";
		case OpKind::Add:      return "+";
		case OpKind::Sub:      return "-";
		case OpKind::Mul:      return "*";
		case OpKind::Div:      return "/";
		case OpKind::Neg:      return "neg";
		case OpKind::Abs:      return "abs";
		case OpKind::Sqrt:     return "sqrt";
		default:               return "?";
	}
}

struct ExprNode {
	OpKind op;
	int id;
	std::string name;
	int lhs{-1};
	int rhs{-1};

	// Range information (from dynamic analysis or user)
	double lo{0.0};
	double hi{0.0};
	int ufp{0};

	// Precision analysis results
	int nsb_forward{0};
	int nsb_backward{0};
	int nsb_final{0};

	// Carry bit (1 = conservative, 0 = refined via carry analysis)
	int carry{1};

	// User-specified requirement (-1 = none)
	int nsb_required{-1};

	// Consumers: nodes that use this node as input
	std::vector<int> consumers;
};

class ExprGraph {
public:
	// ================================================================
	// Graph construction
	// ================================================================

	int constant(double value, const std::string& name = "") {
		ExprNode node;
		node.op = OpKind::Constant;
		node.id = static_cast<int>(nodes_.size());
		node.name = name.empty() ? ("c" + std::to_string(node.id)) : name;
		node.lo = value;
		node.hi = value;
		node.ufp = (value != 0.0) ? compute_ufp(value) : 0;
		nodes_.push_back(node);
		return node.id;
	}

	int variable(const std::string& name, double lo, double hi) {
		ExprNode node;
		node.op = OpKind::Variable;
		node.id = static_cast<int>(nodes_.size());
		node.name = name;
		node.lo = lo;
		node.hi = hi;
		node.ufp = compute_ufp(lo, hi);
		nodes_.push_back(node);
		return node.id;
	}

	// Integration with range_analyzer: extract range and ufp
	template<typename NumberSystem>
	int variable(const std::string& name, const range_analyzer<NumberSystem>& ra) {
		double lo = static_cast<double>(ra.minValue());
		double hi = static_cast<double>(ra.maxValue());
		return variable(name, lo, hi);
	}

	int add(int lhs_id, int rhs_id) { return binary_op(OpKind::Add, lhs_id, rhs_id); }
	int sub(int lhs_id, int rhs_id) { return binary_op(OpKind::Sub, lhs_id, rhs_id); }
	int mul(int lhs_id, int rhs_id) { return binary_op(OpKind::Mul, lhs_id, rhs_id); }
	int div(int lhs_id, int rhs_id) { return binary_op(OpKind::Div, lhs_id, rhs_id); }

	int neg(int operand_id) { return unary_op(OpKind::Neg, operand_id); }
	int abs(int operand_id) { return unary_op(OpKind::Abs, operand_id); }
	int sqrt(int operand_id) { return unary_op(OpKind::Sqrt, operand_id); }

	// ================================================================
	// Requirements specification
	// ================================================================

	void require_nsb(int node_id, int nsb) {
		assert(node_id >= 0 && node_id < static_cast<int>(nodes_.size()));
		nodes_[static_cast<size_t>(node_id)].nsb_required = nsb;
	}

	// ================================================================
	// Analysis
	// ================================================================

	// Run complete analysis: forward + backward + finalize (iterative fixpoint)
	void analyze(int max_iterations = 20) {
		// Initialize forward nsb for leaves
		for (auto& node : nodes_) {
			if (node.op == OpKind::Constant || node.op == OpKind::Variable) {
				// Leaves start with "infinite" precision (we use a large number)
				node.nsb_forward = 53; // double precision as default source precision
			} else {
				node.nsb_forward = 0;
			}
			node.nsb_backward = 0;
		}

		// Iterative fixpoint
		for (int iter = 0; iter < max_iterations; ++iter) {
			bool changed = false;

			// Forward pass (topological order = node id order for DAG)
			for (auto& node : nodes_) {
				int old_nsb = node.nsb_forward;
				compute_forward(node);
				if (node.nsb_forward != old_nsb) changed = true;
			}

			// Backward pass (reverse topological order)
			for (int i = static_cast<int>(nodes_.size()) - 1; i >= 0; --i) {
				int old_nsb = nodes_[static_cast<size_t>(i)].nsb_backward;
				compute_backward(nodes_[static_cast<size_t>(i)]);
				if (nodes_[static_cast<size_t>(i)].nsb_backward != old_nsb) changed = true;
			}

			if (!changed) break;
		}

		// Finalize: nsb_final = max(nsb_forward_demand, nsb_backward)
		for (auto& node : nodes_) {
			node.nsb_final = node.nsb_backward;
			if (node.nsb_required > 0) {
				node.nsb_final = std::max(node.nsb_final, node.nsb_required);
			}
			// Clamp: can't exceed the forward-determined upper bound
			if (node.nsb_forward > 0 && node.nsb_final > node.nsb_forward) {
				node.nsb_final = node.nsb_forward;
			}
			// Minimum 1 bit for any non-constant
			if (node.nsb_final < 1 && node.op != OpKind::Constant) {
				node.nsb_final = 1;
			}
		}
	}

	// ================================================================
	// Results
	// ================================================================

	int get_nsb(int node_id) const {
		assert(node_id >= 0 && node_id < static_cast<int>(nodes_.size()));
		return nodes_[static_cast<size_t>(node_id)].nsb_final;
	}

	const ExprNode& get_node(int node_id) const {
		assert(node_id >= 0 && node_id < static_cast<int>(nodes_.size()));
		return nodes_[static_cast<size_t>(node_id)];
	}

	int size() const { return static_cast<int>(nodes_.size()); }

	// Recommend a Universal type for a node based on its nsb and range
	std::string recommended_type(int node_id, const TypeAdvisor& advisor = TypeAdvisor()) const {
		const auto& node = get_node(node_id);
		auto rec = advisor.recommendForNsb(node.nsb_final, node.lo, node.hi);
		return rec.type.name;
	}

	// Print analysis report
	void report(std::ostream& ostr) const {
		ostr << "POP Expression Graph Analysis\n";
		ostr << std::string(70, '=') << "\n\n";
		ostr << std::left
		     << std::setw(4)  << "ID"
		     << std::setw(12) << "Name"
		     << std::setw(6)  << "Op"
		     << std::setw(6)  << "UFP"
		     << std::setw(6)  << "Fwd"
		     << std::setw(6)  << "Bwd"
		     << std::setw(6)  << "Final"
		     << std::setw(6)  << "Req"
		     << "\n";
		ostr << std::string(52, '-') << "\n";

		for (const auto& node : nodes_) {
			ostr << std::left
			     << std::setw(4)  << node.id
			     << std::setw(12) << node.name
			     << std::setw(6)  << to_string(node.op)
			     << std::setw(6)  << node.ufp
			     << std::setw(6)  << node.nsb_forward
			     << std::setw(6)  << node.nsb_backward
			     << std::setw(6)  << node.nsb_final
			     << std::setw(6)  << (node.nsb_required >= 0 ? std::to_string(node.nsb_required) : "-")
			     << "\n";
		}
		ostr << "\n";
	}

	// Print report with type recommendations
	void report(std::ostream& ostr, const TypeAdvisor& advisor) const {
		ostr << "POP Expression Graph Analysis with Type Recommendations\n";
		ostr << std::string(80, '=') << "\n\n";
		ostr << std::left
		     << std::setw(4)  << "ID"
		     << std::setw(12) << "Name"
		     << std::setw(6)  << "Op"
		     << std::setw(6)  << "UFP"
		     << std::setw(6)  << "NSB"
		     << std::setw(30) << "Recommended Type"
		     << "\n";
		ostr << std::string(64, '-') << "\n";

		for (const auto& node : nodes_) {
			auto rec = advisor.recommendForNsb(node.nsb_final, node.lo, node.hi);
			ostr << std::left
			     << std::setw(4)  << node.id
			     << std::setw(12) << node.name
			     << std::setw(6)  << to_string(node.op)
			     << std::setw(6)  << node.ufp
			     << std::setw(6)  << node.nsb_final
			     << std::setw(30) << rec.type.name
			     << "\n";
		}
		ostr << "\n";
	}

	// Access to nodes for LP solver and carry analysis
	std::vector<ExprNode>& nodes() { return nodes_; }
	const std::vector<ExprNode>& nodes() const { return nodes_; }

private:
	std::vector<ExprNode> nodes_;

	int binary_op(OpKind op, int lhs_id, int rhs_id) {
		assert(lhs_id >= 0 && lhs_id < static_cast<int>(nodes_.size()));
		assert(rhs_id >= 0 && rhs_id < static_cast<int>(nodes_.size()));

		ExprNode node;
		node.op = op;
		node.id = static_cast<int>(nodes_.size());
		node.name = std::string(to_string(op)) + std::to_string(node.id);
		node.lhs = lhs_id;
		node.rhs = rhs_id;

		// Estimate range for binary ops
		auto& l = nodes_[static_cast<size_t>(lhs_id)];
		auto& r = nodes_[static_cast<size_t>(rhs_id)];
		estimate_range(node, l, r);

		// Register as consumer
		l.consumers.push_back(node.id);
		r.consumers.push_back(node.id);

		nodes_.push_back(node);
		return node.id;
	}

	int unary_op(OpKind op, int operand_id) {
		assert(operand_id >= 0 && operand_id < static_cast<int>(nodes_.size()));

		ExprNode node;
		node.op = op;
		node.id = static_cast<int>(nodes_.size());
		node.name = std::string(to_string(op)) + std::to_string(node.id);
		node.lhs = operand_id;
		node.rhs = -1;

		auto& input = nodes_[static_cast<size_t>(operand_id)];
		estimate_unary_range(node, input);

		input.consumers.push_back(node.id);

		nodes_.push_back(node);
		return node.id;
	}

	void estimate_range(ExprNode& z, const ExprNode& x, const ExprNode& y) {
		// Conservative interval arithmetic for range estimation
		double xlo = x.lo, xhi = x.hi;
		double ylo = y.lo, yhi = y.hi;

		switch (z.op) {
		case OpKind::Add:
			z.lo = xlo + ylo;
			z.hi = xhi + yhi;
			break;
		case OpKind::Sub:
			z.lo = xlo - yhi;
			z.hi = xhi - ylo;
			break;
		case OpKind::Mul: {
			double a = xlo * ylo, b = xlo * yhi, c = xhi * ylo, d = xhi * yhi;
			z.lo = std::min({a, b, c, d});
			z.hi = std::max({a, b, c, d});
			break;
		}
		case OpKind::Div: {
			// Skip division by zero range
			if (ylo <= 0.0 && yhi >= 0.0) {
				z.lo = -1e100;
				z.hi = 1e100;
			} else {
				double a = xlo / ylo, b = xlo / yhi, c = xhi / ylo, d = xhi / yhi;
				z.lo = std::min({a, b, c, d});
				z.hi = std::max({a, b, c, d});
			}
			break;
		}
		default:
			z.lo = xlo;
			z.hi = xhi;
			break;
		}
		z.ufp = compute_ufp(z.lo, z.hi);
	}

	void estimate_unary_range(ExprNode& z, const ExprNode& x) {
		switch (z.op) {
		case OpKind::Neg:
			z.lo = -x.hi;
			z.hi = -x.lo;
			break;
		case OpKind::Abs:
			if (x.lo >= 0) {
				z.lo = x.lo;
				z.hi = x.hi;
			} else if (x.hi <= 0) {
				z.lo = -x.hi;
				z.hi = -x.lo;
			} else {
				z.lo = 0.0;
				z.hi = std::max(-x.lo, x.hi);
			}
			break;
		case OpKind::Sqrt:
			z.lo = (x.lo >= 0) ? std::sqrt(x.lo) : 0.0;
			z.hi = (x.hi >= 0) ? std::sqrt(x.hi) : 0.0;
			break;
		default:
			z.lo = x.lo;
			z.hi = x.hi;
			break;
		}
		z.ufp = compute_ufp(z.lo, z.hi);
	}

	void compute_forward(ExprNode& node) {
		if (node.op == OpKind::Constant || node.op == OpKind::Variable) {
			return; // Leaves keep their initial nsb_forward
		}

		if (node.lhs < 0) return;

		auto& l = nodes_[static_cast<size_t>(node.lhs)];

		if (node.rhs < 0) {
			// Unary operation
			switch (node.op) {
			case OpKind::Neg:
			case OpKind::Abs:
				node.nsb_forward = l.nsb_forward;
				break;
			case OpKind::Sqrt: {
				auto pi = forward_sqrt({l.ufp, l.nsb_forward}, node.ufp, node.carry);
				node.nsb_forward = pi.nsb;
				break;
			}
			default:
				node.nsb_forward = l.nsb_forward;
				break;
			}
			return;
		}

		auto& r = nodes_[static_cast<size_t>(node.rhs)];

		precision_info lp{l.ufp, l.nsb_forward};
		precision_info rp{r.ufp, r.nsb_forward};

		switch (node.op) {
		case OpKind::Add: {
			auto pi = forward_add(lp, rp, node.ufp, node.carry);
			node.nsb_forward = pi.nsb;
			break;
		}
		case OpKind::Sub: {
			auto pi = forward_sub(lp, rp, node.ufp, node.carry);
			node.nsb_forward = pi.nsb;
			break;
		}
		case OpKind::Mul: {
			auto pi = forward_mul(lp, rp, node.ufp, node.carry);
			node.nsb_forward = pi.nsb;
			break;
		}
		case OpKind::Div: {
			auto pi = forward_div(lp, rp, node.ufp, node.carry);
			node.nsb_forward = pi.nsb;
			break;
		}
		default:
			break;
		}
	}

	void compute_backward(ExprNode& node) {
		// Seed backward nsb from requirements
		if (node.nsb_required > 0) {
			node.nsb_backward = std::max(node.nsb_backward, node.nsb_required);
		}

		// Propagate from all consumers
		for (int consumer_id : node.consumers) {
			auto& consumer = nodes_[static_cast<size_t>(consumer_id)];
			int consumer_nsb = consumer.nsb_backward;
			if (consumer_nsb <= 0) continue;

			int demanded = compute_backward_demand(consumer, node.id);
			node.nsb_backward = std::max(node.nsb_backward, demanded);
		}
	}

	int compute_backward_demand(const ExprNode& consumer, int input_id) const {
		int nsb_z = consumer.nsb_backward;
		if (nsb_z <= 0) return 0;

		bool is_lhs = (consumer.lhs == input_id);

		switch (consumer.op) {
		case OpKind::Add:
			if (is_lhs) {
				return backward_add_lhs(nsb_z, consumer.ufp, nodes_[static_cast<size_t>(input_id)].ufp, consumer.carry);
			} else {
				return backward_add_rhs(nsb_z, consumer.ufp, nodes_[static_cast<size_t>(input_id)].ufp, consumer.carry);
			}
		case OpKind::Sub:
			if (is_lhs) {
				return backward_sub_lhs(nsb_z, consumer.ufp, nodes_[static_cast<size_t>(input_id)].ufp, consumer.carry);
			} else {
				return backward_sub_rhs(nsb_z, consumer.ufp, nodes_[static_cast<size_t>(input_id)].ufp, consumer.carry);
			}
		case OpKind::Mul:
			if (is_lhs) {
				return backward_mul_lhs(nsb_z, consumer.carry);
			} else {
				return backward_mul_rhs(nsb_z, consumer.carry);
			}
		case OpKind::Div:
			if (is_lhs) {
				return backward_div_lhs(nsb_z, consumer.carry);
			} else {
				return backward_div_rhs(nsb_z, consumer.carry);
			}
		case OpKind::Neg:
			return backward_neg(nsb_z);
		case OpKind::Abs:
			return backward_abs(nsb_z);
		case OpKind::Sqrt:
			return backward_sqrt(nsb_z, consumer.carry);
		default:
			return nsb_z;
		}
	}
};

}} // namespace sw::universal

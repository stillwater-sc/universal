#pragma once
// carry_analysis.hpp: carry-bit refinement via policy iteration for POP
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The default carry bit (carry=1) is conservative. For many operations,
// carry=0 is safe when the operand error cannot affect the result:
//
//   carry(z = x op y) = 0  when  lsb(x_err) > ufp(z)
//
// Policy iteration alternates between:
//   1. Solve LP with current carry values
//   2. Recompute carries from the LP solution
//   3. Repeat until stable
//
// Typically reduces total bits by 10-30%.
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021, Section 5.4.

#include <universal/mixedprecision/expression_graph.hpp>
#include <universal/mixedprecision/pop_solver.hpp>
#include <vector>
#include <cmath>
#include <iostream>

namespace sw { namespace universal {

class CarryAnalyzer {
public:
	// Run carry-bit refinement via policy iteration
	// Returns the number of iterations to convergence
	int refine(ExprGraph& graph, int max_iterations = 10) {
		auto& nodes = graph.nodes();
		int n = static_cast<int>(nodes.size());

		// Initialize all carries to 1 (conservative)
		for (auto& node : nodes) {
			node.carry = 1;
		}

		int iter = 0;
		for (; iter < max_iterations; ++iter) {
			// Step 1: Solve LP with current carries
			PopSolver solver;
			if (!solver.solve(graph)) {
				// LP failed; keep current carries
				break;
			}

			// Step 2: Recompute carries from the solution
			bool changed = false;
			for (int i = 0; i < n; ++i) {
				auto& node = nodes[static_cast<size_t>(i)];
				int old_carry = node.carry;

				int new_carry = compute_carry(node, nodes);
				if (new_carry != old_carry) {
					node.carry = new_carry;
					changed = true;
				}
			}

			if (!changed) break; // Converged
		}

		// Final solve with refined carries
		PopSolver final_solver;
		final_solver.solve(graph);
		iterations_ = iter;

		return iter;
	}

	int iterations() const { return iterations_; }

	// Report carry analysis results
	void report(std::ostream& ostr, const ExprGraph& graph) const {
		ostr << "Carry Analysis Results (converged in " << iterations_ << " iterations)\n";
		ostr << std::string(50, '=') << "\n\n";

		int carry0_count = 0;
		int carry1_count = 0;

		for (const auto& node : graph.nodes()) {
			if (node.op == OpKind::Constant || node.op == OpKind::Variable) continue;

			if (node.carry == 0) ++carry0_count;
			else ++carry1_count;

			ostr << "  " << node.name << " (" << to_string(node.op) << "): carry = "
			     << node.carry << "\n";
		}

		ostr << "\nRefined carries: " << carry0_count << " of "
		     << (carry0_count + carry1_count) << " operations have carry=0\n";
	}

private:
	int iterations_{0};

	// Compute carry for a node based on current nsb assignments
	// carry = 0 when lsb(operand_error) > ufp(result)
	int compute_carry(const ExprNode& node, const std::vector<ExprNode>& nodes) const {
		switch (node.op) {
		case OpKind::Add:
		case OpKind::Sub: {
			if (node.lhs < 0 || node.rhs < 0) return 1;
			auto& l = nodes[static_cast<size_t>(node.lhs)];
			auto& r = nodes[static_cast<size_t>(node.rhs)];

			// lsb of operand error = ufp - nsb (the position of the last significant bit)
			int lsb_l_err = l.ufp - l.nsb_final;
			int lsb_r_err = r.ufp - r.nsb_final;

			// If both operand error lsbs are above ufp(z), carry = 0
			if (lsb_l_err > node.ufp && lsb_r_err > node.ufp) {
				return 0;
			}
			return 1;
		}
		case OpKind::Mul: {
			if (node.lhs < 0 || node.rhs < 0) return 1;
			auto& l = nodes[static_cast<size_t>(node.lhs)];
			auto& r = nodes[static_cast<size_t>(node.rhs)];

			// For multiplication, the product error depends on both operands
			// carry = 0 when nsb_l + nsb_r is small enough
			int lsb_l = l.ufp - l.nsb_final + 1;
			int lsb_r = r.ufp - r.nsb_final + 1;
			int lsb_product = lsb_l + lsb_r;

			// If the product's lsb is above the result's ufp - nsb position, carry = 0
			if (lsb_product > node.ufp - node.nsb_final + 1) {
				return 0;
			}
			return 1;
		}
		case OpKind::Div: {
			// Division carry analysis is more complex; keep conservative
			return 1;
		}
		case OpKind::Sqrt: {
			if (node.lhs < 0) return 1;
			auto& l = nodes[static_cast<size_t>(node.lhs)];
			int lsb_l_err = l.ufp - l.nsb_final;
			if (lsb_l_err > node.ufp) return 0;
			return 1;
		}
		default:
			return 1;
		}
	}
};

}} // namespace sw::universal

#pragma once
// pop_solver.hpp: LP-based optimal bit assignment for POP precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The PopSolver translates an ExprGraph into an LP problem:
//
//   minimize    sum(nsb_i)              // minimize total bits
//   subject to  transfer function constraints at each node
//               nsb_i >= nsb_required_i  for output nodes
//               nsb_i >= 1               for all nodes
//
// This finds the globally optimal bit assignment that minimizes
// total cost while meeting all accuracy requirements.
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021, Chapter 5.

#include <universal/mixedprecision/expression_graph.hpp>

// Use embedded simplex by default; GLPK if available
#ifdef UNIVERSAL_HAS_GLPK
#include <universal/mixedprecision/glpk_solver.hpp>
#else
#include <universal/mixedprecision/simplex.hpp>
#endif

#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace sw { namespace universal {

class PopSolver {
public:
	// Solve the LP and write optimal nsb values back to the graph
	bool solve(ExprGraph& graph) {
		auto& nodes = graph.nodes();
		int n = static_cast<int>(nodes.size());
		if (n == 0) return false;

		SimplexSolver lp;
		lp.set_num_vars(n);

		// Objective: minimize sum of nsb values (uniform weight)
		std::vector<double> obj(static_cast<size_t>(n), 1.0);
		lp.set_objective(obj);

		// Constraints from transfer functions
		for (int i = 0; i < n; ++i) {
			auto& node = nodes[static_cast<size_t>(i)];

			// All variables >= 1 (minimum 1 significant bit)
			{
				std::vector<double> c(static_cast<size_t>(n), 0.0);
				c[static_cast<size_t>(i)] = 1.0;
				lp.add_ge_constraint(c, 1.0);
			}

			// User-specified requirements
			if (node.nsb_required > 0) {
				std::vector<double> c(static_cast<size_t>(n), 0.0);
				c[static_cast<size_t>(i)] = 1.0;
				lp.add_ge_constraint(c, static_cast<double>(node.nsb_required));
			}

			// Transfer function constraints (backward direction)
			switch (node.op) {
			case OpKind::Add:
			case OpKind::Sub: {
				// nsb(lhs) >= nsb(z) + ufp(z) - ufp(lhs) + carry
				// =>  nsb(lhs) - nsb(z) >= ufp(z) - ufp(lhs) + carry
				int lhs = node.lhs;
				int rhs_id = node.rhs;
				if (lhs >= 0) {
					int ufp_shift = node.ufp - nodes[static_cast<size_t>(lhs)].ufp + node.carry;
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(lhs)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, static_cast<double>(ufp_shift));
				}
				if (rhs_id >= 0) {
					int ufp_shift = node.ufp - nodes[static_cast<size_t>(rhs_id)].ufp + node.carry;
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(rhs_id)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, static_cast<double>(ufp_shift));
				}
				break;
			}
			case OpKind::Mul:
			case OpKind::Div: {
				// nsb(lhs) >= nsb(z) + carry
				// => nsb(lhs) - nsb(z) >= carry
				int lhs = node.lhs;
				int rhs_id = node.rhs;
				if (lhs >= 0) {
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(lhs)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, static_cast<double>(node.carry));
				}
				if (rhs_id >= 0) {
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(rhs_id)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, static_cast<double>(node.carry));
				}
				break;
			}
			case OpKind::Neg:
			case OpKind::Abs: {
				// nsb(input) >= nsb(z)
				int lhs = node.lhs;
				if (lhs >= 0) {
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(lhs)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, 0.0);
				}
				break;
			}
			case OpKind::Sqrt: {
				// nsb(input) >= nsb(z) + carry
				int lhs = node.lhs;
				if (lhs >= 0) {
					std::vector<double> c(static_cast<size_t>(n), 0.0);
					c[static_cast<size_t>(lhs)] = 1.0;
					c[static_cast<size_t>(i)] = -1.0;
					lp.add_ge_constraint(c, static_cast<double>(node.carry));
				}
				break;
			}
			case OpKind::Constant:
			case OpKind::Variable:
				// No transfer constraints for leaves
				break;
			}
		}

		// Solve
		LPStatus status = lp.solve();

		if (status != LPStatus::Optimal) {
			status_ = status;
			return false;
		}

		status_ = status;

		// Write back solution (ceil to integer bits)
		total_nsb_ = 0.0;
		for (int i = 0; i < n; ++i) {
			double val = lp.get_value(i);
			int nsb = static_cast<int>(std::ceil(val - 1e-9)); // round up, tolerant of floating-point
			if (nsb < 1) nsb = 1;
			nodes[static_cast<size_t>(i)].nsb_final = nsb;
			total_nsb_ += nsb;
		}

		return true;
	}

	// Total nsb across all nodes (cost)
	double total_nsb() const { return total_nsb_; }

	// LP solver status
	LPStatus status() const { return status_; }

	// Print solution report
	void report(std::ostream& ostr, const ExprGraph& graph) const {
		ostr << "POP LP Solver Results\n";
		ostr << std::string(50, '=') << "\n";
		ostr << "Status: " << to_string(status_) << "\n";
		ostr << "Total NSB: " << total_nsb_ << "\n\n";

		ostr << std::left
		     << std::setw(4)  << "ID"
		     << std::setw(12) << "Name"
		     << std::setw(6)  << "Op"
		     << std::setw(6)  << "NSB"
		     << std::setw(6)  << "Req"
		     << "\n";
		ostr << std::string(34, '-') << "\n";

		for (const auto& node : graph.nodes()) {
			ostr << std::left
			     << std::setw(4)  << node.id
			     << std::setw(12) << node.name
			     << std::setw(6)  << to_string(node.op)
			     << std::setw(6)  << node.nsb_final
			     << std::setw(6)  << (node.nsb_required >= 0 ? std::to_string(node.nsb_required) : "-")
			     << "\n";
		}
	}

private:
	LPStatus status_{LPStatus::Infeasible};
	double total_nsb_{0.0};
};

}} // namespace sw::universal

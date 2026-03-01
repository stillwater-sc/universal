#pragma once
// simplex.hpp: embedded header-only LP solver for POP precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// A minimal simplex solver for small linear programs arising from
// POP precision tuning. Solves:
//
//   minimize    c^T x
//   subject to  A x >= b
//               x >= 0
//
// Implementation uses the revised simplex method with a dense tableau.
// For small problems (< 100 variables) this is more than adequate.
// For larger problems, link against GLPK or HiGHS.
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021, Chapter 5.

#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <cassert>
#include <iostream>

namespace sw { namespace universal {

enum class LPStatus {
	Optimal,
	Infeasible,
	Unbounded,
	MaxIterations
};

inline const char* to_string(LPStatus s) {
	switch (s) {
		case LPStatus::Optimal:       return "Optimal";
		case LPStatus::Infeasible:    return "Infeasible";
		case LPStatus::Unbounded:     return "Unbounded";
		case LPStatus::MaxIterations: return "MaxIterations";
		default:                      return "Unknown";
	}
}

// Minimal LP solver using the two-phase simplex method
// Solves: minimize c^T x  subject to  A x >= b, x >= 0
class SimplexSolver {
public:
	SimplexSolver() : nvars_(0), status_(LPStatus::Infeasible), obj_value_(0.0) {}

	// Set number of decision variables
	void set_num_vars(int n) {
		nvars_ = n;
		objective_.assign(static_cast<size_t>(n), 0.0);
	}

	// Set the objective function coefficients (minimize)
	void set_objective(const std::vector<double>& coeffs) {
		assert(static_cast<int>(coeffs.size()) == nvars_);
		objective_ = coeffs;
	}

	// Add a >= constraint: sum(coeffs[i] * x[i]) >= rhs
	void add_ge_constraint(const std::vector<double>& coeffs, double rhs) {
		assert(static_cast<int>(coeffs.size()) == nvars_);
		Constraint c;
		c.coeffs = coeffs;
		c.rhs = rhs;
		constraints_.push_back(c);
	}

	// Add a <= constraint: sum(coeffs[i] * x[i]) <= rhs
	void add_le_constraint(const std::vector<double>& coeffs, double rhs) {
		// Convert to >=: -coeffs >= -rhs
		std::vector<double> neg(coeffs.size());
		for (size_t i = 0; i < coeffs.size(); ++i) neg[i] = -coeffs[i];
		add_ge_constraint(neg, -rhs);
	}

	// Add an equality constraint: sum(coeffs[i] * x[i]) == rhs
	void add_eq_constraint(const std::vector<double>& coeffs, double rhs) {
		add_ge_constraint(coeffs, rhs);
		add_le_constraint(coeffs, rhs);
	}

	// Solve the LP using the Big-M simplex method
	LPStatus solve(int max_iterations = 10000) {
		int m = static_cast<int>(constraints_.size()); // number of constraints
		int n = nvars_;                                 // decision variables

		if (m == 0 || n == 0) {
			status_ = LPStatus::Infeasible;
			return status_;
		}

		// Build the tableau
		// Variables: x[0..n-1] (decision), s[0..m-1] (surplus), a[0..m-1] (artificial)
		// Total columns: n + m (surplus) + m (artificial) + 1 (rhs)
		int total_vars = n + 2 * m;
		int cols = total_vars + 1; // +1 for RHS
		int rows = m + 1;         // +1 for objective

		// Allocate tableau
		std::vector<std::vector<double>> T(static_cast<size_t>(rows),
			std::vector<double>(static_cast<size_t>(cols), 0.0));

		// Big-M penalty
		double M = 1e6;

		// Fill constraint rows
		for (int i = 0; i < m; ++i) {
			// Decision variables
			for (int j = 0; j < n; ++j) {
				T[static_cast<size_t>(i)][static_cast<size_t>(j)] = constraints_[static_cast<size_t>(i)].coeffs[static_cast<size_t>(j)];
			}
			// Surplus variable (>= becomes == with surplus subtracted)
			T[static_cast<size_t>(i)][static_cast<size_t>(n + i)] = -1.0;
			// Artificial variable
			T[static_cast<size_t>(i)][static_cast<size_t>(n + m + i)] = 1.0;
			// RHS
			T[static_cast<size_t>(i)][static_cast<size_t>(total_vars)] = constraints_[static_cast<size_t>(i)].rhs;
		}

		// Objective row: minimize c^T x + M * sum(artificial)
		for (int j = 0; j < n; ++j) {
			T[static_cast<size_t>(m)][static_cast<size_t>(j)] = objective_[static_cast<size_t>(j)];
		}
		for (int j = 0; j < m; ++j) {
			T[static_cast<size_t>(m)][static_cast<size_t>(n + m + j)] = M;
		}

		// Subtract M * (constraint rows) from objective to make artificial basis feasible
		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < cols; ++j) {
				T[static_cast<size_t>(m)][static_cast<size_t>(j)] -= M * T[static_cast<size_t>(i)][static_cast<size_t>(j)];
			}
		}

		// Basis tracking: basis[i] = column index of basic variable in row i
		std::vector<int> basis(static_cast<size_t>(m));
		for (int i = 0; i < m; ++i) {
			basis[static_cast<size_t>(i)] = n + m + i; // artificial variables
		}

		// Simplex iterations
		double eps = 1e-10;
		for (int iter = 0; iter < max_iterations; ++iter) {
			// Find pivot column: most negative coefficient in objective row
			int pivot_col = -1;
			double min_val = -eps;
			for (int j = 0; j < total_vars; ++j) {
				if (T[static_cast<size_t>(m)][static_cast<size_t>(j)] < min_val) {
					min_val = T[static_cast<size_t>(m)][static_cast<size_t>(j)];
					pivot_col = j;
				}
			}

			if (pivot_col == -1) {
				// Optimal - check if artificial variables are zero
				bool feasible = true;
				for (int i = 0; i < m; ++i) {
					if (basis[static_cast<size_t>(i)] >= n + m) {
						if (std::abs(T[static_cast<size_t>(i)][static_cast<size_t>(total_vars)]) > eps) {
							feasible = false;
							break;
						}
					}
				}
				if (feasible) {
					status_ = LPStatus::Optimal;
				} else {
					status_ = LPStatus::Infeasible;
				}
				break;
			}

			// Find pivot row: minimum ratio test
			int pivot_row = -1;
			double min_ratio = std::numeric_limits<double>::max();
			for (int i = 0; i < m; ++i) {
				double aij = T[static_cast<size_t>(i)][static_cast<size_t>(pivot_col)];
				if (aij > eps) {
					double ratio = T[static_cast<size_t>(i)][static_cast<size_t>(total_vars)] / aij;
					if (ratio < min_ratio) {
						min_ratio = ratio;
						pivot_row = i;
					}
				}
			}

			if (pivot_row == -1) {
				status_ = LPStatus::Unbounded;
				break;
			}

			// Pivot
			double pivot_elem = T[static_cast<size_t>(pivot_row)][static_cast<size_t>(pivot_col)];
			for (int j = 0; j < cols; ++j) {
				T[static_cast<size_t>(pivot_row)][static_cast<size_t>(j)] /= pivot_elem;
			}
			for (int i = 0; i <= m; ++i) {
				if (i == pivot_row) continue;
				double factor = T[static_cast<size_t>(i)][static_cast<size_t>(pivot_col)];
				if (std::abs(factor) > eps) {
					for (int j = 0; j < cols; ++j) {
						T[static_cast<size_t>(i)][static_cast<size_t>(j)] -= factor * T[static_cast<size_t>(pivot_row)][static_cast<size_t>(j)];
					}
				}
			}

			basis[static_cast<size_t>(pivot_row)] = pivot_col;

			if (iter == max_iterations - 1) {
				status_ = LPStatus::MaxIterations;
			}
		}

		// Extract solution
		solution_.assign(static_cast<size_t>(n), 0.0);
		if (status_ == LPStatus::Optimal) {
			for (int i = 0; i < m; ++i) {
				int b = basis[static_cast<size_t>(i)];
				if (b < n) {
					solution_[static_cast<size_t>(b)] = T[static_cast<size_t>(i)][static_cast<size_t>(total_vars)];
				}
			}
			// Compute objective value
			obj_value_ = 0.0;
			for (int j = 0; j < n; ++j) {
				obj_value_ += objective_[static_cast<size_t>(j)] * solution_[static_cast<size_t>(j)];
			}
		}

		return status_;
	}

	// Get solution value for variable i
	double get_value(int var) const {
		assert(var >= 0 && var < static_cast<int>(solution_.size()));
		return solution_[static_cast<size_t>(var)];
	}

	// Get optimal objective value
	double objective_value() const { return obj_value_; }

	// Get solver status
	LPStatus status() const { return status_; }

private:
	struct Constraint {
		std::vector<double> coeffs;
		double rhs;
	};

	int nvars_;
	std::vector<double> objective_;
	std::vector<Constraint> constraints_;
	std::vector<double> solution_;
	LPStatus status_;
	double obj_value_;
};

}} // namespace sw::universal

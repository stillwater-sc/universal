#pragma once
// glpk_solver.hpp: optional GLPK binding for POP precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Provides a GLPK-backed LP/ILP solver as an alternative to the
// embedded simplex solver. Enable with -DUNIVERSAL_HAS_GLPK.
//
// This header is only included when UNIVERSAL_HAS_GLPK is defined.

#ifdef UNIVERSAL_HAS_GLPK

#include <glpk.h>
#include <vector>
#include <cmath>
#include <cassert>

namespace sw { namespace universal {

// GLPK-backed solver with ILP capability
// Provides the same interface as SimplexSolver but uses GLPK
// for better performance on larger problems and true integer solutions.
class GlpkSolver {
public:
	GlpkSolver() : nvars_(0), prob_(nullptr) {}

	// Rule of 5: raw pointer ownership
	GlpkSolver(const GlpkSolver&) = delete;
	GlpkSolver& operator=(const GlpkSolver&) = delete;
	GlpkSolver(GlpkSolver&& other) noexcept
		: nvars_(other.nvars_), objective_(std::move(other.objective_)),
		  constraints_(std::move(other.constraints_)),
		  solution_(std::move(other.solution_)), prob_(other.prob_) {
		other.prob_ = nullptr;
	}
	GlpkSolver& operator=(GlpkSolver&& other) noexcept {
		if (this != &other) {
			if (prob_) glp_delete_prob(prob_);
			nvars_ = other.nvars_;
			objective_ = std::move(other.objective_);
			constraints_ = std::move(other.constraints_);
			solution_ = std::move(other.solution_);
			prob_ = other.prob_;
			other.prob_ = nullptr;
		}
		return *this;
	}

	~GlpkSolver() {
		if (prob_) glp_delete_prob(prob_);
	}

	void set_num_vars(int n) {
		nvars_ = n;
		objective_.assign(static_cast<size_t>(n), 0.0);
	}

	void set_objective(const std::vector<double>& coeffs) {
		assert(static_cast<int>(coeffs.size()) == nvars_);
		objective_ = coeffs;
	}

	void add_ge_constraint(const std::vector<double>& coeffs, double rhs) {
		assert(static_cast<int>(coeffs.size()) == nvars_);
		Constraint c;
		c.coeffs = coeffs;
		c.rhs = rhs;
		c.type = GLP_LO;
		constraints_.push_back(c);
	}

	LPStatus solve(int /*max_iterations*/ = 10000) {
		if (prob_) glp_delete_prob(prob_);
		prob_ = glp_create_prob();
		glp_set_obj_dir(prob_, GLP_MIN);

		int m = static_cast<int>(constraints_.size());

		// Add columns (decision variables)
		glp_add_cols(prob_, nvars_);
		for (int j = 1; j <= nvars_; ++j) {
			glp_set_col_bnds(prob_, j, GLP_LO, 0.0, 0.0);
			glp_set_obj_coef(prob_, j, objective_[static_cast<size_t>(j - 1)]);
			glp_set_col_kind(prob_, j, GLP_IV); // Integer variable
		}

		// Add rows (constraints)
		glp_add_rows(prob_, m);
		for (int i = 0; i < m; ++i) {
			auto& con = constraints_[static_cast<size_t>(i)];
			glp_set_row_bnds(prob_, i + 1, con.type, con.rhs, 0.0);
		}

		// Load constraint matrix in sparse format
		int nz = m * nvars_;
		std::vector<int> ia(static_cast<size_t>(nz + 1));
		std::vector<int> ja(static_cast<size_t>(nz + 1));
		std::vector<double> ar(static_cast<size_t>(nz + 1));

		int k = 1;
		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < nvars_; ++j) {
				ia[static_cast<size_t>(k)] = i + 1;
				ja[static_cast<size_t>(k)] = j + 1;
				ar[static_cast<size_t>(k)] = constraints_[static_cast<size_t>(i)].coeffs[static_cast<size_t>(j)];
				++k;
			}
		}
		glp_load_matrix(prob_, nz, ia.data(), ja.data(), ar.data());

		// Solve LP relaxation first, then ILP
		glp_smcp parm;
		glp_init_smcp(&parm);
		parm.msg_lev = GLP_MSG_OFF;
		glp_simplex(prob_, &parm);

		glp_iocp iocp;
		glp_init_iocp(&iocp);
		iocp.msg_lev = GLP_MSG_OFF;
		int ret = glp_intopt(prob_, &iocp);

		if (ret != 0 || glp_mip_status(prob_) != GLP_OPT) {
			return LPStatus::Infeasible;
		}

		// Extract solution
		solution_.assign(static_cast<size_t>(nvars_), 0.0);
		for (int j = 1; j <= nvars_; ++j) {
			solution_[static_cast<size_t>(j - 1)] = glp_mip_col_val(prob_, j);
		}

		return LPStatus::Optimal;
	}

	double get_value(int var) const {
		assert(var >= 0 && var < static_cast<int>(solution_.size()));
		return solution_[static_cast<size_t>(var)];
	}

private:
	struct Constraint {
		std::vector<double> coeffs;
		double rhs;
		int type;
	};

	int nvars_;
	std::vector<double> objective_;
	std::vector<Constraint> constraints_;
	std::vector<double> solution_;
	glp_prob* prob_;
};

}} // namespace sw::universal

#endif // UNIVERSAL_HAS_GLPK

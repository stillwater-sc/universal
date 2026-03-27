#pragma once
// expression.hpp: recursive-descent expression parser and evaluator for ucalc
//
// Supports infix arithmetic with standard precedence, variables,
// built-in functions, and constants.
//
// Grammar:
//   statement   = assignment | expr
//   assignment  = IDENT '=' expr
//   expr        = term (('+' | '-') term)*
//   term        = unary (('*' | '/') unary)*
//   unary       = '-' unary | power
//   power       = postfix ('^' unary)?
//   postfix     = primary | IDENT '(' args ')'
//   primary     = NUMBER | IDENT | '(' expr ')'
//   args        = expr (',' expr)*
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cctype>

#include "type_dispatch.hpp"

namespace sw { namespace ucalc {

// Token types
enum class TokenType {
	Number, Ident, Plus, Minus, Star, Slash, Caret,
	LParen, RParen, Comma, Equals, End
};

struct Token {
	TokenType type;
	std::string text;
	double number_value;

	Token() : type(TokenType::End), number_value(0) {}
	Token(TokenType t, const std::string& s) : type(t), text(s), number_value(0) {}
	Token(TokenType t, const std::string& s, double v) : type(t), text(s), number_value(v) {}
};

// Tokenizer
class Tokenizer {
public:
	explicit Tokenizer(const std::string& input) : input_(input), pos_(0) {}

	std::vector<Token> tokenize() {
		std::vector<Token> tokens;
		while (pos_ < input_.size()) {
			skip_whitespace();
			if (pos_ >= input_.size()) break;

			char c = input_[pos_];

			if (std::isdigit(c) || (c == '.' && pos_ + 1 < input_.size() && std::isdigit(input_[pos_ + 1]))) {
				tokens.push_back(read_number());
			} else if (std::isalpha(c) || c == '_') {
				tokens.push_back(read_ident());
			} else {
				switch (c) {
				case '+': tokens.emplace_back(TokenType::Plus, "+"); break;
				case '-': tokens.emplace_back(TokenType::Minus, "-"); break;
				case '*': tokens.emplace_back(TokenType::Star, "*"); break;
				case '/': tokens.emplace_back(TokenType::Slash, "/"); break;
				case '^': tokens.emplace_back(TokenType::Caret, "^"); break;
				case '(': tokens.emplace_back(TokenType::LParen, "("); break;
				case ')': tokens.emplace_back(TokenType::RParen, ")"); break;
				case ',': tokens.emplace_back(TokenType::Comma, ","); break;
				case '=': tokens.emplace_back(TokenType::Equals, "="); break;
				default:
					throw std::runtime_error(std::string("unexpected character: '") + c + "'");
				}
				++pos_;
			}
		}
		tokens.emplace_back(TokenType::End, "");
		return tokens;
	}

private:
	void skip_whitespace() {
		while (pos_ < input_.size() && std::isspace(input_[pos_])) ++pos_;
	}

	Token read_number() {
		size_t start = pos_;
		// Handle hex: 0x...
		if (pos_ + 1 < input_.size() && input_[pos_] == '0' &&
		    (input_[pos_ + 1] == 'x' || input_[pos_ + 1] == 'X')) {
			pos_ += 2;
			while (pos_ < input_.size() && std::isxdigit(input_[pos_])) ++pos_;
			std::string text = input_.substr(start, pos_ - start);
			double val = static_cast<double>(std::stoull(text, nullptr, 16));
			return Token(TokenType::Number, text, val);
		}
		// Decimal with optional fraction and exponent
		while (pos_ < input_.size() && std::isdigit(input_[pos_])) ++pos_;
		if (pos_ < input_.size() && input_[pos_] == '.') {
			++pos_;
			while (pos_ < input_.size() && std::isdigit(input_[pos_])) ++pos_;
		}
		if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
			++pos_;
			if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) ++pos_;
			while (pos_ < input_.size() && std::isdigit(input_[pos_])) ++pos_;
		}
		std::string text = input_.substr(start, pos_ - start);
		double val = std::stod(text);
		return Token(TokenType::Number, text, val);
	}

	Token read_ident() {
		size_t start = pos_;
		while (pos_ < input_.size() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) ++pos_;
		std::string text = input_.substr(start, pos_ - start);
		return Token(TokenType::Ident, text);
	}

	std::string input_;
	size_t pos_;
};

// Trace step: records one arithmetic operation during evaluation
struct TraceStep {
	int step_number;
	std::string operation;       // e.g. "add", "sub", "mul", "sin"
	std::string description;     // human-readable, e.g. "1.0 + 1e-4"
	double operand_a;            // first operand (as double)
	double operand_b;            // second operand (as double, 0 for unary)
	double result;               // result in the active type (as double)
	std::string result_rep;      // native_rep of the result
	std::string result_binary;   // binary_rep of the result
};

// Parser and evaluator
class ExpressionEvaluator {
public:
	explicit ExpressionEvaluator(const TypeOps& ops) : ops_(&ops) {}

	void set_type(const TypeOps& ops) { ops_ = &ops; }

	// Enable/disable trace mode
	void enable_trace(bool on = true) { tracing_ = on; trace_steps_.clear(); }
	const std::vector<TraceStep>& trace_steps() const { return trace_steps_; }

	// Evaluate an expression string, return the result as a Value
	Value evaluate(const std::string& input) {
		Tokenizer tokenizer(input);
		tokens_ = tokenizer.tokenize();
		pos_ = 0;
		Value result = parse_statement();
		if (current().type != TokenType::End) {
			throw std::runtime_error("unexpected token: '" + current().text + "'");
		}
		return result;
	}

	// Variable access
	void set_variable(const std::string& name, const Value& val) {
		variables_[name] = val;
	}

	const std::map<std::string, Value>& variables() const { return variables_; }

	bool has_variable(const std::string& name) const {
		return variables_.find(name) != variables_.end();
	}

	// Check if an expression is an assignment (peek only)
	static bool is_assignment(const std::string& input) {
		// Simple heuristic: contains '=' that is not '==' and starts with ident
		size_t i = 0;
		while (i < input.size() && std::isspace(input[i])) ++i;
		if (i >= input.size() || !(std::isalpha(input[i]) || input[i] == '_')) return false;
		while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) ++i;
		while (i < input.size() && std::isspace(input[i])) ++i;
		return i < input.size() && input[i] == '=';
	}

private:
	// Record a trace step for a binary operation
	void record_binary(const std::string& op, const std::string& sym,
	                   const Value& a, const Value& b, const Value& result) {
		if (!tracing_) return;
		TraceStep step;
		step.step_number = static_cast<int>(trace_steps_.size()) + 1;
		step.operation = op;
		std::ostringstream desc;
		desc << a.native_rep << " " << sym << " " << b.native_rep;
		step.description = desc.str();
		step.operand_a = a.num;
		step.operand_b = b.num;
		step.result = result.num;
		step.result_rep = result.native_rep;
		step.result_binary = result.binary_rep;
		trace_steps_.push_back(std::move(step));
	}

	// Record a trace step for a unary operation
	void record_unary(const std::string& op, const Value& a, const Value& result) {
		if (!tracing_) return;
		TraceStep step;
		step.step_number = static_cast<int>(trace_steps_.size()) + 1;
		step.operation = op;
		std::ostringstream desc;
		if (op == "negate")
			desc << "-(" << a.native_rep << ")";
		else
			desc << op << "(" << a.native_rep << ")";
		step.description = desc.str();
		step.operand_a = a.num;
		step.operand_b = 0.0;
		step.result = result.num;
		step.result_rep = result.native_rep;
		step.result_binary = result.binary_rep;
		trace_steps_.push_back(std::move(step));
	}

	const Token& current() const { return tokens_[pos_]; }
	const Token& advance() { return tokens_[pos_++]; }
	const Token& peek(size_t offset = 0) const { return tokens_[pos_ + offset]; }

	void expect(TokenType type, const std::string& what) {
		if (current().type != type) {
			throw std::runtime_error("expected " + what + ", got '" + current().text + "'");
		}
		advance();
	}

	Value parse_statement() {
		// Check for assignment: IDENT = expr
		if (current().type == TokenType::Ident && pos_ + 1 < tokens_.size() &&
		    tokens_[pos_ + 1].type == TokenType::Equals) {
			std::string name = current().text;
			advance(); // skip ident
			advance(); // skip '='
			Value val = parse_expr();
			variables_[name] = val;
			return val;
		}
		return parse_expr();
	}

	Value parse_expr() {
		Value left = parse_term();
		while (current().type == TokenType::Plus || current().type == TokenType::Minus) {
			TokenType op = current().type;
			advance();
			Value right = parse_term();
			if (op == TokenType::Plus) {
				Value r = ops_->add(left, right);
				record_binary("add", "+", left, right, r);
				left = r;
			} else {
				Value r = ops_->sub(left, right);
				record_binary("sub", "-", left, right, r);
				left = r;
			}
		}
		return left;
	}

	Value parse_term() {
		Value left = parse_unary();
		while (current().type == TokenType::Star || current().type == TokenType::Slash) {
			TokenType op = current().type;
			advance();
			Value right = parse_unary();
			if (op == TokenType::Star) {
				Value r = ops_->mul(left, right);
				record_binary("mul", "*", left, right, r);
				left = r;
			} else {
				Value r = ops_->div(left, right);
				record_binary("div", "/", left, right, r);
				left = r;
			}
		}
		return left;
	}

	Value parse_unary() {
		if (current().type == TokenType::Minus) {
			advance();
			Value val = parse_unary();
			Value r = ops_->negate(val);
			record_unary("negate", val, r);
			return r;
		}
		if (current().type == TokenType::Plus) {
			advance();
			return parse_unary();
		}
		return parse_power();
	}

	Value parse_power() {
		Value base = parse_postfix();
		if (current().type == TokenType::Caret) {
			advance();
			Value exp = parse_unary(); // right-associative
			Value r = ops_->fn_pow(base, exp);
			record_binary("pow", "^", base, exp, r);
			return r;
		}
		return base;
	}

	Value parse_postfix() {
		if (current().type == TokenType::Ident && pos_ + 1 < tokens_.size() &&
		    tokens_[pos_ + 1].type == TokenType::LParen) {
			std::string fname = current().text;
			advance(); // skip ident
			advance(); // skip '('
			std::vector<Value> args;
			if (current().type != TokenType::RParen) {
				args.push_back(parse_expr());
				while (current().type == TokenType::Comma) {
					advance();
					args.push_back(parse_expr());
				}
			}
			expect(TokenType::RParen, "')'");
			return call_function(fname, args);
		}
		return parse_primary();
	}

	Value parse_primary() {
		if (current().type == TokenType::Number) {
			double val = current().number_value;
			advance();
			return ops_->from_double(val);
		}

		if (current().type == TokenType::Ident) {
			std::string name = current().text;
			advance();
			// Check constants -- use high-precision path via TypeOps::constant()
			if (name == "pi" || name == "e" || name == "phi" ||
			    name == "ln2" || name == "ln10" || name == "sqrt2") {
				if (ops_->constant) return ops_->constant(name);
				// fallback if constant callback not set
				if (name == "pi")  return ops_->from_double(3.14159265358979323846);
				if (name == "e")   return ops_->from_double(2.71828182845904523536);
				if (name == "phi") return ops_->from_double(1.61803398874989484820);
			}
			// Check variables: return stored Value preserving full precision
			// Arithmetic ops use Value.num (double interchange) regardless,
			// but display fields (binary_rep, native_rep) retain the precision
			// from the original evaluation context
			auto it = variables_.find(name);
			if (it != variables_.end()) {
				return it->second;
			}
			throw std::runtime_error("undefined variable: '" + name + "'");
		}

		if (current().type == TokenType::LParen) {
			advance();
			Value val = parse_expr();
			expect(TokenType::RParen, "')'");
			return val;
		}

		throw std::runtime_error("expected number, variable, or '(', got '" + current().text + "'");
	}

	Value call_function(const std::string& name, const std::vector<Value>& args) {
		Value r;
		if (args.size() == 1) {
			if (name == "sqrt") { r = ops_->fn_sqrt(args[0]); record_unary("sqrt", args[0], r); return r; }
			if (name == "abs")  { r = ops_->fn_abs(args[0]);  record_unary("abs",  args[0], r); return r; }
			if (name == "log")  { r = ops_->fn_log(args[0]);  record_unary("log",  args[0], r); return r; }
			if (name == "exp")  { r = ops_->fn_exp(args[0]);  record_unary("exp",  args[0], r); return r; }
			if (name == "sin")  { r = ops_->fn_sin(args[0]);  record_unary("sin",  args[0], r); return r; }
			if (name == "cos")  { r = ops_->fn_cos(args[0]);  record_unary("cos",  args[0], r); return r; }
		}
		if (args.size() == 2) {
			if (name == "pow")  { r = ops_->fn_pow(args[0], args[1]); record_binary("pow", ",", args[0], args[1], r); return r; }
		}
		throw std::runtime_error("unknown function or wrong arity: " + name +
		                         "(" + std::to_string(args.size()) + " args)");
	}

	const TypeOps* ops_;
	std::vector<Token> tokens_;
	size_t pos_;
	std::map<std::string, Value> variables_;
	bool tracing_ = false;
	std::vector<TraceStep> trace_steps_;
};

}} // namespace sw::ucalc

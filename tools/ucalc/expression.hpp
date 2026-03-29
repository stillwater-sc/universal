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
#include <memory>
#include <optional>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cctype>

#include "type_dispatch.hpp"

namespace sw { namespace ucalc {

///////////////////////////////////////////////////////////////////////////
// AST (Abstract Syntax Tree) for expression representation
///////////////////////////////////////////////////////////////////////////

enum class ASTKind { Literal, Variable, Constant, BinaryOp, UnaryOp, FunctionCall };

// Provenance: tracks whether a value is an exact input or a computed intermediate
enum class Provenance {
	exact,      // literal, variable, or constant -- known with full precision
	computed    // result of an arithmetic operation -- subject to rounding
};

struct ASTNode {
	ASTKind kind;
	Provenance provenance = Provenance::exact;
	// Literal
	double literal_value = 0.0;
	// Variable / Constant / FunctionCall name / BinaryOp/UnaryOp operator
	std::string name;
	// Children (left/right for binary, single for unary, args for function)
	std::shared_ptr<ASTNode> left;
	std::shared_ptr<ASTNode> right;
	std::vector<std::shared_ptr<ASTNode>> args;

	static std::shared_ptr<ASTNode> make_literal(double v) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::Literal;
		n->literal_value = v;
		return n;
	}
	static std::shared_ptr<ASTNode> make_variable(const std::string& name) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::Variable;
		n->name = name;
		return n;
	}
	static std::shared_ptr<ASTNode> make_constant(const std::string& name) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::Constant;
		n->name = name;
		return n;
	}
	static std::shared_ptr<ASTNode> make_binary(const std::string& op,
	    std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::BinaryOp;
		n->provenance = Provenance::computed;
		n->name = op;
		n->left = std::move(l);
		n->right = std::move(r);
		return n;
	}
	static std::shared_ptr<ASTNode> make_unary(const std::string& op,
	    std::shared_ptr<ASTNode> operand) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::UnaryOp;
		n->provenance = Provenance::computed;
		n->name = op;
		n->left = std::move(operand);
		return n;
	}
	static std::shared_ptr<ASTNode> make_function(const std::string& fname,
	    std::vector<std::shared_ptr<ASTNode>> arguments) {
		auto n = std::make_shared<ASTNode>();
		n->kind = ASTKind::FunctionCall;
		n->provenance = Provenance::computed;
		n->name = fname;
		n->args = std::move(arguments);
		return n;
	}
};

// Print AST as indented tree
inline void print_ast(const std::shared_ptr<ASTNode>& node, std::ostream& out,
                      const std::string& prefix = "", bool is_last = true,
                      bool show_provenance = false) {
	if (!node) return;
	out << prefix << (is_last ? "`-- " : "|-- ");
	std::string ptag = show_provenance
	    ? (node->provenance == Provenance::exact ? " [exact]" : " [computed]") : "";
	switch (node->kind) {
	case ASTKind::Literal:
		out << node->literal_value << ptag << "\n";
		break;
	case ASTKind::Variable:
		out << "var:" << node->name << ptag << "\n";
		break;
	case ASTKind::Constant:
		out << "const:" << node->name << ptag << "\n";
		break;
	case ASTKind::BinaryOp:
		out << "op:" << node->name << ptag << "\n";
		{
			std::string child_prefix = prefix + (is_last ? "    " : "|   ");
			print_ast(node->left, out, child_prefix, false, show_provenance);
			print_ast(node->right, out, child_prefix, true, show_provenance);
		}
		break;
	case ASTKind::UnaryOp:
		out << "unary:" << node->name << ptag << "\n";
		{
			std::string child_prefix = prefix + (is_last ? "    " : "|   ");
			print_ast(node->left, out, child_prefix, true, show_provenance);
		}
		break;
	case ASTKind::FunctionCall:
		out << "fn:" << node->name << ptag << "\n";
		{
			std::string child_prefix = prefix + (is_last ? "    " : "|   ");
			for (size_t i = 0; i < node->args.size(); ++i) {
				print_ast(node->args[i], out, child_prefix, i + 1 == node->args.size(), show_provenance);
			}
		}
		break;
	}
}

// Serialize AST to compact expression string
inline std::string ast_to_string(const std::shared_ptr<ASTNode>& node) {
	if (!node) return "";
	switch (node->kind) {
	case ASTKind::Literal: {
		std::ostringstream ss;
		ss << node->literal_value;
		return ss.str();
	}
	case ASTKind::Variable:
		return node->name;
	case ASTKind::Constant:
		return node->name;
	case ASTKind::BinaryOp:
		return "(" + ast_to_string(node->left) + " " + node->name + " " + ast_to_string(node->right) + ")";
	case ASTKind::UnaryOp:
		if (node->name == "negate") return "(-" + ast_to_string(node->left) + ")";
		return node->name + "(" + ast_to_string(node->left) + ")";
	case ASTKind::FunctionCall: {
		std::string s = node->name + "(";
		for (size_t i = 0; i < node->args.size(); ++i) {
			if (i > 0) s += ", ";
			s += ast_to_string(node->args[i]);
		}
		return s + ")";
	}
	}
	return "";
}

///////////////////////////////////////////////////////////////////////////
// AST Pattern Matching
///////////////////////////////////////////////////////////////////////////

// Bindings: map pattern variable names to matched subtrees
using ASTBindings = std::map<std::string, std::shared_ptr<ASTNode>>;

// Match an expression AST against a pattern AST.
// Pattern variables (Variable nodes) act as wildcards that bind to
// arbitrary subtrees. Returns bindings if match succeeds, empty optional
// if not. A pattern variable that appears multiple times must bind to
// structurally identical subtrees.
inline bool ast_equal(const std::shared_ptr<ASTNode>& a, const std::shared_ptr<ASTNode>& b) {
	if (!a && !b) return true;
	if (!a || !b) return false;
	if (a->kind != b->kind) return false;
	if (a->name != b->name) return false;
	if (a->kind == ASTKind::Literal && a->literal_value != b->literal_value) return false;
	if (!ast_equal(a->left, b->left)) return false;
	if (!ast_equal(a->right, b->right)) return false;
	if (a->args.size() != b->args.size()) return false;
	for (size_t i = 0; i < a->args.size(); ++i) {
		if (!ast_equal(a->args[i], b->args[i])) return false;
	}
	return true;
}

inline bool match_ast_impl(const std::shared_ptr<ASTNode>& expr,
                           const std::shared_ptr<ASTNode>& pattern,
                           ASTBindings& bindings) {
	if (!pattern) return !expr;
	if (!expr) return false;

	// Pattern variable: binds to any subtree
	if (pattern->kind == ASTKind::Variable) {
		auto it = bindings.find(pattern->name);
		if (it != bindings.end()) {
			// Already bound -- must match the same subtree
			return ast_equal(expr, it->second);
		}
		bindings[pattern->name] = expr;
		return true;
	}

	// Literal: must match value exactly
	if (pattern->kind == ASTKind::Literal) {
		return expr->kind == ASTKind::Literal &&
		       expr->literal_value == pattern->literal_value;
	}

	// Constant: must match name
	if (pattern->kind == ASTKind::Constant) {
		return expr->kind == ASTKind::Constant && expr->name == pattern->name;
	}

	// BinaryOp: match operator and both children
	if (pattern->kind == ASTKind::BinaryOp) {
		if (expr->kind != ASTKind::BinaryOp) return false;
		if (expr->name != pattern->name) return false;
		ASTBindings saved = bindings;
		if (match_ast_impl(expr->left, pattern->left, bindings) &&
		    match_ast_impl(expr->right, pattern->right, bindings)) {
			return true;
		}
		// Try commutative match for + and *
		if (pattern->name == "+" || pattern->name == "*") {
			bindings = saved;
			return match_ast_impl(expr->left, pattern->right, bindings) &&
			       match_ast_impl(expr->right, pattern->left, bindings);
		}
		return false;
	}

	// UnaryOp: match operator and operand
	if (pattern->kind == ASTKind::UnaryOp) {
		if (expr->kind != ASTKind::UnaryOp) return false;
		if (expr->name != pattern->name) return false;
		return match_ast_impl(expr->left, pattern->left, bindings);
	}

	// FunctionCall: match name and all args
	if (pattern->kind == ASTKind::FunctionCall) {
		if (expr->kind != ASTKind::FunctionCall) return false;
		if (expr->name != pattern->name) return false;
		if (expr->args.size() != pattern->args.size()) return false;
		for (size_t i = 0; i < expr->args.size(); ++i) {
			if (!match_ast_impl(expr->args[i], pattern->args[i], bindings)) return false;
		}
		return true;
	}

	return false;
}

// Top-level match: returns bindings if match succeeds
inline std::optional<ASTBindings> match_ast(
    const std::shared_ptr<ASTNode>& expr,
    const std::shared_ptr<ASTNode>& pattern) {
	ASTBindings bindings;
	if (match_ast_impl(expr, pattern, bindings)) {
		return bindings;
	}
	return std::nullopt;
}

// Substitute pattern variables in an AST template using bindings
inline std::shared_ptr<ASTNode> substitute_ast(
    const std::shared_ptr<ASTNode>& tmpl,
    const ASTBindings& bindings) {
	if (!tmpl) return nullptr;
	if (tmpl->kind == ASTKind::Variable) {
		auto it = bindings.find(tmpl->name);
		if (it != bindings.end()) return it->second;
		throw std::runtime_error("unbound pattern variable '" + tmpl->name +
		    "' in rewrite template");
	}
	auto result = std::make_shared<ASTNode>(*tmpl);
	result->left = substitute_ast(tmpl->left, bindings);
	result->right = substitute_ast(tmpl->right, bindings);
	for (size_t i = 0; i < result->args.size(); ++i) {
		result->args[i] = substitute_ast(tmpl->args[i], bindings);
	}
	return result;
}

// Search an expression AST for any subtree matching a pattern.
// Returns the first match found (depth-first), or nullopt.
struct ASTMatch {
	std::shared_ptr<ASTNode> matched_subtree;
	ASTBindings bindings;
};

// Find all subtrees matching a pattern (depth-first)
inline void find_all_patterns(
    const std::shared_ptr<ASTNode>& expr,
    const std::shared_ptr<ASTNode>& pattern,
    std::vector<ASTMatch>& out) {
	if (!expr) return;
	if (auto result = match_ast(expr, pattern)) {
		out.push_back(ASTMatch{ expr, *result });
	}
	find_all_patterns(expr->left, pattern, out);
	find_all_patterns(expr->right, pattern, out);
	for (const auto& arg : expr->args) {
		find_all_patterns(arg, pattern, out);
	}
}

// Convenience: find first match only
inline std::optional<ASTMatch> find_pattern(
    const std::shared_ptr<ASTNode>& expr,
    const std::shared_ptr<ASTNode>& pattern) {
	std::vector<ASTMatch> matches;
	find_all_patterns(expr, pattern, matches);
	if (matches.empty()) return std::nullopt;
	return matches.front();
}

///////////////////////////////////////////////////////////////////////////
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
	double operand_a;            // first operand (as double, lossy for >64-bit types)
	double operand_b;            // second operand (as double, 0 for unary)
	std::string operand_a_rep;   // lossless native_rep of first operand
	std::string operand_b_rep;   // lossless native_rep of second operand (empty for unary)
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

	// Build an AST from an expression string (does not evaluate)
	std::shared_ptr<ASTNode> build_ast(const std::string& input) {
		Tokenizer tokenizer(input);
		tokens_ = tokenizer.tokenize();
		pos_ = 0;
		auto tree = ast_statement();
		if (current().type != TokenType::End) {
			throw std::runtime_error("unexpected token: '" + current().text + "'");
		}
		return tree;
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
		step.operand_a_rep = a.native_rep;
		step.operand_b_rep = b.native_rep;
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
		step.operand_a_rep = a.native_rep;
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
			    name == "ln2" || name == "ln10" || name == "sqrt2" ||
			    name == "sqrt3" || name == "sqrt5") {
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
			if (name == "tan")  { r = ops_->fn_tan(args[0]);  record_unary("tan",  args[0], r); return r; }
			if (name == "asin") { r = ops_->fn_asin(args[0]); record_unary("asin", args[0], r); return r; }
			if (name == "acos") { r = ops_->fn_acos(args[0]); record_unary("acos", args[0], r); return r; }
			if (name == "atan") { r = ops_->fn_atan(args[0]); record_unary("atan", args[0], r); return r; }
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

	// AST-building parse methods (mirror the evaluation methods)
	std::shared_ptr<ASTNode> ast_statement() {
		if (current().type == TokenType::Ident && pos_ + 1 < tokens_.size() &&
		    tokens_[pos_ + 1].type == TokenType::Equals) {
			std::string name = current().text;
			advance(); advance();
			return ASTNode::make_binary("=", ASTNode::make_variable(name), ast_expr());
		}
		return ast_expr();
	}

	std::shared_ptr<ASTNode> ast_expr() {
		auto left = ast_term();
		while (current().type == TokenType::Plus || current().type == TokenType::Minus) {
			std::string op = (current().type == TokenType::Plus) ? "+" : "-";
			advance();
			auto right = ast_term();
			left = ASTNode::make_binary(op, left, right);
		}
		return left;
	}

	std::shared_ptr<ASTNode> ast_term() {
		auto left = ast_unary();
		while (current().type == TokenType::Star || current().type == TokenType::Slash) {
			std::string op = (current().type == TokenType::Star) ? "*" : "/";
			advance();
			auto right = ast_unary();
			left = ASTNode::make_binary(op, left, right);
		}
		return left;
	}

	std::shared_ptr<ASTNode> ast_unary() {
		if (current().type == TokenType::Minus) {
			advance();
			return ASTNode::make_unary("negate", ast_unary());
		}
		if (current().type == TokenType::Plus) {
			advance();
			return ast_unary();
		}
		return ast_power();
	}

	std::shared_ptr<ASTNode> ast_power() {
		auto base = ast_postfix();
		if (current().type == TokenType::Caret) {
			advance();
			auto exp = ast_unary();
			return ASTNode::make_binary("^", base, exp);
		}
		return base;
	}

	std::shared_ptr<ASTNode> ast_postfix() {
		if (current().type == TokenType::Ident && pos_ + 1 < tokens_.size() &&
		    tokens_[pos_ + 1].type == TokenType::LParen) {
			std::string fname = current().text;
			advance(); advance();
			std::vector<std::shared_ptr<ASTNode>> args;
			if (current().type != TokenType::RParen) {
				args.push_back(ast_expr());
				while (current().type == TokenType::Comma) {
					advance();
					args.push_back(ast_expr());
				}
			}
			expect(TokenType::RParen, "')'");
			return ASTNode::make_function(fname, std::move(args));
		}
		return ast_primary();
	}

	std::shared_ptr<ASTNode> ast_primary() {
		if (current().type == TokenType::Number) {
			double val = current().number_value;
			advance();
			return ASTNode::make_literal(val);
		}
		if (current().type == TokenType::Ident) {
			std::string name = current().text;
			advance();
			// Constants
			if (name == "pi" || name == "e" || name == "phi" ||
			    name == "ln2" || name == "ln10" || name == "sqrt2" ||
			    name == "sqrt3" || name == "sqrt5") {
				return ASTNode::make_constant(name);
			}
			return ASTNode::make_variable(name);
		}
		if (current().type == TokenType::LParen) {
			advance();
			auto node = ast_expr();
			expect(TokenType::RParen, "')'");
			return node;
		}
		throw std::runtime_error("expected number, variable, or '(', got '" + current().text + "'");
	}
};

}} // namespace sw::ucalc

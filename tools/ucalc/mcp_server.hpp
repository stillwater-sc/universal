#pragma once
// mcp_server.hpp: Model Context Protocol server for ucalc
//
// Implements the MCP protocol (JSON-RPC 2.0 over stdio) to expose
// ucalc commands as tools that AI assistants can call directly.
//
// Protocol: https://modelcontextprotocol.io/
// Transport: stdio (Content-Length framing)
//
// No external dependencies -- minimal JSON parsing for the fixed
// MCP message structure.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <map>
#include <cctype>

namespace sw { namespace ucalc {

// Minimal JSON value extraction for MCP messages.
// Not a general JSON parser -- handles the fixed MCP request structure.

// Extract a string value for a key: "key": "value"
inline std::string json_get_string(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	auto pos = json.find(search);
	if (pos == std::string::npos) return "";
	pos = json.find(':', pos + search.size());
	if (pos == std::string::npos) return "";
	pos = json.find('"', pos + 1);
	if (pos == std::string::npos) return "";
	++pos;
	auto end = json.find('"', pos);
	if (end == std::string::npos) return "";
	return json.substr(pos, end - pos);
}

// Extract an integer value for a key: "key": 123
inline int json_get_int(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	auto pos = json.find(search);
	if (pos == std::string::npos) return -1;
	pos = json.find(':', pos + search.size());
	if (pos == std::string::npos) return -1;
	++pos;
	while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
	return std::stoi(json.substr(pos));
}

// Extract the "params" object as a raw string (between { })
inline std::string json_get_object(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	auto pos = json.find(search);
	if (pos == std::string::npos) return "";
	pos = json.find('{', pos + search.size());
	if (pos == std::string::npos) return "";
	int depth = 1;
	size_t start = pos;
	++pos;
	while (pos < json.size() && depth > 0) {
		if (json[pos] == '{') ++depth;
		else if (json[pos] == '}') --depth;
		++pos;
	}
	return json.substr(start, pos - start);
}

// Read a JSON-RPC message from stdin (Content-Length framing)
inline bool read_message(std::string& out) {
	// Read headers until empty line
	std::string line;
	int content_length = -1;
	while (std::getline(std::cin, line)) {
		// Strip \r
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) break;
		if (line.substr(0, 16) == "Content-Length: ") {
			content_length = std::stoi(line.substr(16));
		}
	}
	if (content_length < 0) return false;
	out.resize(content_length);
	std::cin.read(&out[0], content_length);
	return std::cin.good();
}

// Write a JSON-RPC message to stdout (Content-Length framing)
inline void write_message(const std::string& json) {
	std::cout << "Content-Length: " << json.size() << "\r\n\r\n" << json;
	std::cout.flush();
}

// Escape a string for safe embedding in a JSON string literal
inline std::string mcp_json_escape(const std::string& s) {
	std::ostringstream ss;
	for (char c : s) {
		switch (c) {
		case '"':  ss << "\\\""; break;
		case '\\': ss << "\\\\"; break;
		case '\n': ss << "\\n"; break;
		case '\r': ss << "\\r"; break;
		case '\t': ss << "\\t"; break;
		default:
			if (static_cast<unsigned char>(c) < 0x20) {
				char buf[8];
				std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
				ss << buf;
			} else {
				ss << c;
			}
		}
	}
	return ss.str();
}

// Build a JSON-RPC response
inline std::string jsonrpc_result(const std::string& id_str, const std::string& result) {
	return "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":" + result + "}";
}

inline std::string jsonrpc_error(const std::string& id_str, int code, const std::string& message) {
	return "{\"jsonrpc\":\"2.0\",\"id\":" + id_str
	     + ",\"error\":{\"code\":" + std::to_string(code)
	     + ",\"message\":\"" + mcp_json_escape(message) + "\"}}";
}

// Extract the id field (could be int or string)
inline std::string extract_id(const std::string& json) {
	auto pos = json.find("\"id\"");
	if (pos == std::string::npos) return "null";
	pos = json.find(':', pos + 4);
	if (pos == std::string::npos) return "null";
	++pos;
	while (pos < json.size() && json[pos] == ' ') ++pos;
	if (json[pos] == '"') {
		// String id
		auto end = json.find('"', pos + 1);
		return json.substr(pos, end - pos + 1);
	}
	// Numeric id
	auto end = json.find_first_of(",} \t\r\n", pos);
	return json.substr(pos, end - pos);
}

// ucalc tool definitions for MCP
struct McpTool {
	std::string name;
	std::string description;
	std::string input_schema; // JSON schema for arguments
};

inline std::vector<McpTool> ucalc_tools() {
	return {
		{"ucalc.eval",      "Evaluate an expression in the active type",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.show",      "Show value with binary decomposition and components",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.compare",   "Evaluate across all types in a table",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.types",     "List all available number types",
		 "{\"type\":\"object\",\"properties\":{}}"},
		{"ucalc.precision", "Show type precision, epsilon, range",
		 "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"}},\"required\":[\"type\"]}"},
		{"ucalc.range",     "Show type dynamic range",
		 "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"}},\"required\":[\"type\"]}"},
		{"ucalc.ulp",       "Show ULP at a value",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.trace",     "Trace error propagation through an expression",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.cancel",    "Detect catastrophic cancellation",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.audit",     "Rounding audit with cumulative drift",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.quantize",  "Quantize data and report RMSE/QSNR",
		 "{\"type\":\"object\",\"properties\":{\"format\":{\"type\":\"string\"},\"data\":{\"type\":\"string\"}},\"required\":[\"format\",\"data\"]}"},
		{"ucalc.suggest",   "Find unstable patterns and suggest rewrites",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.oracle",    "Canonical result with rounding verification",
		 "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"},\"expression\":{\"type\":\"string\"}},\"required\":[\"type\",\"expression\"]}"},
		{"ucalc.faithful",  "Check faithful rounding",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.steps",     "Step-by-step arithmetic visualization",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"type\":{\"type\":\"string\"}},\"required\":[\"expression\"]}"},
		{"ucalc.heatmap",   "Precision vs magnitude heatmap",
		 "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"}},\"required\":[\"type\"]}"},
		{"ucalc.rewrites",  "List numerical rewrite patterns",
		 "{\"type\":\"object\",\"properties\":{}}"},
		// the uncertainty box and decidability (#1654): definitions are 'name = expr' assignments
		// separated by ';', evaluated in a tile type, so 'a = 3~; b = 100~' gives ULP-wide inputs
		{"ucalc.ubox",      "Uncertainty box of an expression in tile interval types (areal/poxel): tiles, tightest box, overestimation, decimals, sign",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"types\":{\"type\":\"string\",\"description\":\"space-separated, e.g. poxel16i poxel32i\"},\"definitions\":{\"type\":\"string\",\"description\":\"assignments separated by ';', e.g. a = 3~; b = 100~\"},\"trace\":{\"type\":\"string\",\"description\":\"'true' for per-operation tile counts\"}},\"required\":[\"expression\"]}"},
		{"ucalc.decide",    "Is a question about a result decidable in each tile type? 'sign <expr>' or '<expr> op <expr>' (op: < <= > >= == !=); yes / no / undecidable, and the narrowest deciding type",
		 "{\"type\":\"object\",\"properties\":{\"predicate\":{\"type\":\"string\"},\"types\":{\"type\":\"string\"},\"definitions\":{\"type\":\"string\"}},\"required\":[\"predicate\"]}"},
		{"ucalc.roots",     "Roots of a x^2 + b x + c in tile interval types: discriminant verdict, each root as written and stable, tightest box, containment, and the narrowest width deciding each question",
		 "{\"type\":\"object\",\"properties\":{\"a\":{\"type\":\"string\"},\"b\":{\"type\":\"string\"},\"c\":{\"type\":\"string\"},\"types\":{\"type\":\"string\"},\"digits\":{\"type\":\"string\"},\"definitions\":{\"type\":\"string\",\"description\":\"assignments separated by ';', for names the coefficients use\"}},\"required\":[\"a\",\"b\",\"c\"]}"},
		{"ucalc.rootbox",   "Roots of f(x) in [lo, hi] by bisection over the tiles on the sign of f: root / exact root / undecided / unexplored boxes",
		 "{\"type\":\"object\",\"properties\":{\"expression\":{\"type\":\"string\"},\"variable\":{\"type\":\"string\",\"description\":\"default x\"},\"lo\":{\"type\":\"string\"},\"hi\":{\"type\":\"string\"},\"types\":{\"type\":\"string\"},\"definitions\":{\"type\":\"string\"}},\"required\":[\"expression\",\"lo\",\"hi\"]}"},
	};
}

// A types argument names registered types only: letters, digits, underscores and spaces.
inline bool plain_words(const std::string& s) {
	for (char c : s) {
		if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == ' ')) return false;
	}
	return true;
}

// The definitions argument: 'name = expr' assignments separated by ';'.  Each piece must be an
// assignment, so the separator cannot smuggle in another command.  Returns the commands, or
// false when a piece is not an assignment.
inline bool definition_commands(const std::string& defs, std::string& out) {
	out.clear();
	std::string piece;
	std::istringstream in(defs);
	while (std::getline(in, piece, ';')) {
		const auto b = piece.find_first_not_of(" \t");
		if (b == std::string::npos) continue;
		piece = piece.substr(b);
		if (piece.find('\n') != std::string::npos || piece.find('\r') != std::string::npos) return false;
		if (!ExpressionEvaluator::is_assignment(piece)) return false;
		out += piece + "; ";
	}
	return true;
}

// Build the tools/list response
inline std::string tools_list_json() {
	std::ostringstream ss;
	ss << "{\"tools\":[";
	const auto& tools = ucalc_tools();
	for (size_t i = 0; i < tools.size(); ++i) {
		if (i > 0) ss << ",";
		ss << "{\"name\":\"" << tools[i].name << "\""
		   << ",\"description\":\"" << tools[i].description << "\""
		   << ",\"inputSchema\":" << tools[i].input_schema << "}";
	}
	ss << "]}";
	return ss.str();
}

// Reject command-separator characters that could inject extra REPL commands
inline bool contains_injection(const std::string& s) {
	return s.find(';') != std::string::npos
	    || s.find('\n') != std::string::npos
	    || s.find('\r') != std::string::npos;
}

// Map an MCP tool call to a ucalc command string
// Returns empty string on unknown tool or if arguments contain injection characters.
inline std::string tool_to_command(const std::string& tool_name, const std::string& args_json) {
	std::string type_arg = json_get_string(args_json, "type");
	std::string expr = json_get_string(args_json, "expression");
	std::string fmt = json_get_string(args_json, "format");
	std::string data = json_get_string(args_json, "data");

	const std::string predicate = json_get_string(args_json, "predicate");
	const std::string types = json_get_string(args_json, "types");
	const std::string defs = json_get_string(args_json, "definitions");
	const std::string a = json_get_string(args_json, "a"), b = json_get_string(args_json, "b"), c = json_get_string(args_json, "c");
	const std::string digits = json_get_string(args_json, "digits");
	const std::string variable = json_get_string(args_json, "variable");
	const std::string lo = json_get_string(args_json, "lo"), hi = json_get_string(args_json, "hi");
	const std::string trace = json_get_string(args_json, "trace");

	// Block command injection via separator characters
	if (contains_injection(type_arg) || contains_injection(expr)
	    || contains_injection(fmt) || contains_injection(data)
	    || contains_injection(predicate) || contains_injection(a) || contains_injection(b) || contains_injection(c)
	    || contains_injection(digits) || contains_injection(variable) || contains_injection(lo) || contains_injection(hi)
	    || !plain_words(types) || !plain_words(digits) || !plain_words(variable)) {
		return "";
	}

	// the uncertainty tools: their definitions are evaluated in a tile type (x~ needs one)
	if (tool_name == "ucalc.ubox" || tool_name == "ucalc.decide" || tool_name == "ucalc.roots" || tool_name == "ucalc.rootbox") {
		// the required arguments, as the schemas declare them
		if ((tool_name == "ucalc.ubox" && expr.empty()) || (tool_name == "ucalc.decide" && predicate.empty())
		    || (tool_name == "ucalc.roots" && (a.empty() || b.empty() || c.empty()))
		    || (tool_name == "ucalc.rootbox" && (expr.empty() || lo.empty() || hi.empty()))) {
			return "";
		}
		std::string prefix;
		if (!defs.empty()) {
			if (!definition_commands(defs, prefix)) return "";
			prefix = "type " + (type_arg.empty() ? std::string("poxel32i") : type_arg) + "; " + prefix;
		}
		const std::string in_types = types.empty() ? std::string() : " in " + types;
		if (tool_name == "ucalc.ubox") return prefix + "ubox " + (trace == "true" ? "trace " : "") + expr + in_types;
		if (tool_name == "ucalc.decide") return prefix + "decide " + predicate + in_types;
		if (tool_name == "ucalc.roots") return prefix + "roots " + a + ", " + b + ", " + c + in_types + (digits.empty() ? "" : " digits " + digits);
		return prefix + "rootbox " + expr + " for " + (variable.empty() ? std::string("x") : variable) + " in [" + lo + ", " + hi + "]" + (types.empty() ? "" : " " + types);
	}

	std::string cmd;

	// Set type if provided
	if (!type_arg.empty()) {
		cmd += "type " + type_arg + "; ";
	}

	if (tool_name == "ucalc.eval")      cmd += expr;
	else if (tool_name == "ucalc.show")      cmd += "show " + expr;
	else if (tool_name == "ucalc.compare")   cmd += "compare " + expr;
	else if (tool_name == "ucalc.types")     cmd += "types";
	else if (tool_name == "ucalc.precision") cmd += "precision";
	else if (tool_name == "ucalc.range")     cmd += "range";
	else if (tool_name == "ucalc.ulp")       cmd += "ulp " + expr;
	else if (tool_name == "ucalc.trace")     cmd += "trace " + expr;
	else if (tool_name == "ucalc.cancel")    cmd += "cancel " + expr;
	else if (tool_name == "ucalc.audit")     cmd += "audit " + expr;
	else if (tool_name == "ucalc.quantize")  cmd += "quantize " + fmt + " " + data;
	else if (tool_name == "ucalc.suggest")   cmd += "suggest " + expr;
	else if (tool_name == "ucalc.oracle")    cmd += "oracle " + type_arg + " " + expr;
	else if (tool_name == "ucalc.faithful")  cmd += "faithful " + expr;
	else if (tool_name == "ucalc.steps")     cmd += "steps " + expr;
	else if (tool_name == "ucalc.heatmap")   cmd += "heatmap";
	else if (tool_name == "ucalc.rewrites")  cmd += "rewrites";
	else return "";

	return cmd;
}

// Format a tool call result for MCP response
inline std::string tool_result_json(const std::string& output, bool is_error = false) {
	std::string result = "{\"content\":[{\"type\":\"text\",\"text\":\""
	                   + mcp_json_escape(output) + "\"}]";
	if (is_error) result += ",\"isError\":true";
	result += "}";
	return result;
}

}} // namespace sw::ucalc

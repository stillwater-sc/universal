---
title: "MCP Server: ucalc as an AI Agent Tool"
description: "AI agent integration via the Model Context Protocol"
---


The Model Context Protocol (MCP) allows AI assistants to call external
tools directly, without the user copy-pasting output back and forth.
ucalc includes a built-in MCP server that exposes its full command set
as structured tools, making it a compute oracle that AI agents can
query programmatically.

## Why an MCP Server for Mixed-Precision Arithmetic?

AI assistants are increasingly used to analyze numerical algorithms,
debug precision issues, and recommend number types for specific
workloads. But AI models have no native ability to perform exact
arithmetic in posit, lns, dbns, or any of the 42+ types that ucalc
supports. Without a tool like ucalc, an AI assistant must guess at
numerical behavior -- it cannot verify whether an expression suffers
from catastrophic cancellation in bfloat16, or whether posit32
produces a faithfully rounded result for a given function.

The MCP server closes this gap. An AI assistant connected to ucalc can:

- **Evaluate expressions** in any number type and inspect the exact result
- **Compare representations** across all 42+ types to find the best fit
- **Trace error propagation** through compound expressions
- **Detect cancellation** and suggest stable rewrites
- **Generate test vectors** for regression suites
- **Verify rounding** against quad-double references
- **Visualize precision** with heatmaps and numberlines

All of this happens through structured JSON-RPC calls -- no parsing of
terminal output, no prompt engineering to extract numbers from text.

## Architecture

The MCP server is implemented in a single header (`mcp_server.hpp`,
~270 lines) with zero external dependencies, consistent with the
Universal library's design principle of no external dependencies.
It implements JSON-RPC 2.0 over stdio with Content-Length framing:

```
AI Assistant                     ucalc --mcp
    |                                |
    |-- Content-Length: N\r\n\r\n -->|
    |   {"method":"initialize",...}  |
    |                                |
    |<-- Content-Length: M\r\n\r\n --|
    |   {"result":{"capabilities"}}  |
    |                                |
    |-- tools/list ----------------->|
    |<-- 17 tools with JSON schemas -|
    |                                |
    |-- tools/call: ucalc.oracle --->|
    |   {"type":"posit32",           |
    |    "expression":"sin(0.1)"}    |
    |<-- result with value, binary,  |
    |    rounding verification ------|
```

## Starting the MCP Server

```bash
ucalc --mcp
```

The server reads JSON-RPC messages from stdin and writes responses to
stdout. It is designed to be launched as a subprocess by an AI assistant
framework (Claude Desktop, VS Code extensions, or any MCP-compatible client).

## Available Tools

The MCP server exposes 17 tools:

| Tool | Description |
|------|-------------|
| `ucalc.eval` | Evaluate an expression in a specified type |
| `ucalc.show` | Value with binary decomposition and components |
| `ucalc.compare` | Evaluate across all types in a table |
| `ucalc.types` | List all available number types |
| `ucalc.precision` | Type precision, epsilon, range |
| `ucalc.range` | Dynamic range of a type |
| `ucalc.ulp` | Unit in the last place at a value |
| `ucalc.trace` | Error propagation through an expression |
| `ucalc.cancel` | Catastrophic cancellation detection |
| `ucalc.audit` | Rounding audit with cumulative drift |
| `ucalc.quantize` | Quantize data and report RMSE/QSNR |
| `ucalc.suggest` | Find unstable patterns and suggest rewrites |
| `ucalc.oracle` | Canonical result with rounding verification |
| `ucalc.faithful` | Faithful rounding check |
| `ucalc.steps` | Step-by-step arithmetic visualization |
| `ucalc.heatmap` | Precision vs magnitude heatmap |
| `ucalc.rewrites` | List numerical rewrite patterns |

Each tool has a JSON schema describing its parameters, so AI assistants
can discover and call tools without prior knowledge of the ucalc command
syntax.

## Example Interactions

An AI assistant investigating precision loss in a neural network might
make these calls:

```json
{"method": "tools/call", "params": {"name": "ucalc.compare",
  "arguments": {"expression": "1/3 + 1/3 + 1/3"}}}
```

The response contains the comparison table across all types, showing
which types evaluate 1/3 + 1/3 + 1/3 to exactly 1.0 and which don't.

```json
{"method": "tools/call", "params": {"name": "ucalc.oracle",
  "arguments": {"type": "bfloat16", "expression": "exp(-0.5)"}}}
```

Returns the exact bfloat16 result with rounding verification, letting
the assistant report whether bfloat16 is faithfully rounded for this
activation function input.

```json
{"method": "tools/call", "params": {"name": "ucalc.steps",
  "arguments": {"type": "posit16", "expression": "1.5 * 0.375"}}}
```

Returns a step-by-step decomposition of posit multiplication showing
regime decoding, exponent addition, significand multiplication, and
regime re-encoding -- the same output a human sees in the REPL, but
returned as structured text the AI can reason about.

## Integrating with AI Assistants

### Claude Desktop / Claude Code

Add ucalc as an MCP server in your configuration:

```json
{
  "mcpServers": {
    "ucalc": {
      "command": "/path/to/ucalc",
      "args": ["--mcp"]
    }
  }
}
```

Once configured, the AI assistant can call any ucalc tool directly.
For example, asking "What is the ULP of pi in posit32?" will trigger
a `ucalc.ulp` call with `{"expression": "pi", "type": "posit32"}`.

### Any MCP-Compatible Client

ucalc's MCP server follows the standard protocol:

1. Launch `ucalc --mcp` as a subprocess
2. Send `initialize` to negotiate capabilities
3. Send `tools/list` to discover available tools
4. Send `tools/call` with tool name and arguments
5. Parse the JSON-RPC response

No authentication or network configuration needed -- everything runs
over stdio pipes.

## Security

The MCP server sanitizes all tool arguments to prevent command injection.
Arguments containing semicolons, newlines, or carriage returns are
rejected, ensuring that MCP tool calls cannot execute arbitrary REPL
commands beyond their declared schema.

The server only responds to the methods it recognizes (`initialize`,
`tools/list`, `tools/call`). Unknown methods receive a JSON-RPC error
response. The server has no file system access beyond what ucalc's
`-f` flag provides, and MCP mode does not enable `-f`.

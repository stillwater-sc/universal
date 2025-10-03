# Closure Plot Generation Tools

This directory contains tools for generating closure plots for Universal number systems.

## Overview

Closure plots are 2D visualizations that show the behavior of arithmetic operations across all possible operand combinations for a given number system. Each pixel in the plot represents the result of an operation between two operands, color-coded by the type of result:

- **Black**: Exact result
- **Purple (gradient)**: Approximation (intensity based on error magnitude)
- **Red**: Overflow
- **Blue**: Underflow
- **Yellow**: NaN/NAR (Not a Real)
- **Green**: Saturate

## Tools

### closure_plot_generator
Command-line tool for generating PNG closure plots for any Universal number system.

```bash
# Generate closure plots for 8-bit posit
closure_plot_generator --type posit --nbits 8 --es 0 --output plots/

# Generate closure plots for 8-bit cfloat
closure_plot_generator --type cfloat --nbits 8 --exp 4 --output plots/
```

## Output

For each number system, the tool generates:
- `{system}_add.png` - Addition closure plot
- `{system}_sub.png` - Subtraction closure plot
- `{system}_mul.png` - Multiplication closure plot
- `{system}_div.png` - Division closure plot

## Implementation

The PNG generation is implemented from scratch without external dependencies, following Universal's design principles.
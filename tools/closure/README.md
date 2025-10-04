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

 Orthogonal Design

  Two Independent Transformations:

  1. Coordinate Centering (setCenterAroundZero):
    - false: Raw pixel coordinates (0,0) top-left to (max,max) bottom-right
    - true: Mathematical centering with quadrant rearrangement
  2. Encoding Selection (setValueBasedMapping):
    - false: Raw bit pattern encoding order (0, 1, 2, ..., max)
    - true: Value-based ordering (maxneg → zero → maxpos)

  Four Possible Combinations:

  1. Raw + Raw (default): setMappingMode(false, false)
    - Raw bit patterns in raw coordinates
    - Good for seeing encoding patterns
  2. Raw + Centered: setMappingMode(true, false)
    - Raw bit patterns with coordinate centering
    - Useful for bit pattern analysis in mathematical layout
  3. Value + Raw: setMappingMode(false, true)
    - Value ordering in raw coordinates
    - Mathematical values but not centered
  4. Value + Centered: setMappingMode(true, true)
    - Value ordering with mathematical centering
    - True mathematical layout - this is what you want for cfloat<8,2>!

  Implementation Details:

  The transformation pipeline now works as:
  Pixel Coordinate → Coordinate Centering → Encoding Selection → Final Encoding

  - applyCenteringTransform(): Handles coordinate quadrant rearrangement
  - selectEncoding(): Chooses raw vs value-based encoding
  - Both transformations compose cleanly

  Updated API:

  // Individual controls (orthogonal)
  generator.setCenterAroundZero(true);     // Coordinate centering
  generator.setValueBasedMapping(true);    // Encoding selection

  // Convenience method
  generator.setMappingMode(true, true);    // Both at once

  // For cfloat<8,2> with correct overflow placement:
  generator.setMappingMode(true, true);    // Value-based + Centered

  This design gives users full control over both dimensions independently, and for your cfloat<8,2> case, using
  setMappingMode(true, true) will ensure overflow appears correctly in the top-right corner when adding large positive values!
# faithful: Compensated Evaluation

## Why

Every floating-point operation rounds, and the rounding error is not random noise -- for
`+`, `-` and `*` it is *exactly representable* in the same format as the result. An
error-free transformation computes both halves: the rounded result you would have got
anyway, and the error term that rounding threw away.

`faithful` is that pair, made into a type. Instead of returning `fl(a + b)` and discarding
the residual, it carries `(value, error)` forward, so a computation can see how much
precision it has been losing.

```cpp
#include <universal/number/faithful/faithful.hpp>
using namespace sw::universal;

faithful<double> f(1.0);
std::cout << f << '\n';   // ( 1, 0)   -- value and error
```

The name refers to *faithful rounding*: a result that is one of the two representable
values bracketing the exact answer. Tracking the residual is what lets an algorithm
establish that property, rather than assuming it.

## What

```cpp
template<typename FloatingPointType = double>
class faithful;
```

A `faithful<T>` holds two `T` values: the rounded result and the error term recovered by
the error-free transformation. Arithmetic uses the Dekker and Knuth algorithms --
`two_sum` for addition, `two_product` for multiplication -- which are exact when the
format has subnormals and no overflow occurs.

| Property | Value |
|---|---|
| Representation | `(value, error)` pair in the host type |
| Host | any IEEE binary format; `double` by default |
| Exactness | `+`, `-`, `*` recover the residual exactly |

### The same machinery, used three ways

The error-free transformations under `faithful` are the same ones that build the
multi-component types. What differs is what each does with the residual:

| type | what it does with the rounding error |
|---|---|
| `faithful` | carries it alongside, so you can inspect it |
| [`dd`](/universal/number-systems/dd/) / [`qd`](/universal/number-systems/qd/) | folds it into a fixed 2- or 4-limb expansion |
| [`ereal`](/universal/number-systems/ereal/) | folds it into an expansion of chosen length |
| [`elreal`](/universal/number-systems/elreal/) | keeps producing more of it on demand |

Reach for `faithful` when the error term is the thing you want to *see* -- error analysis,
teaching, or establishing a bound -- rather than the thing you want folded back into a
more precise value.

### A caveat

The error-free transformations are only error-free on formats that behave: a format
without subnormals flushes the `two_sum` residual to zero, and an operation that overflows
loses the term entirely. That is a property of the host format, not of this type, and it
is why the interval tightness work restricts EFT-based bounds to native floating-point.

## Related

- [`interval`](/universal/number-systems/interval/) -- rigorous enclosures rather than a tracked residual
- [`areal`](/universal/number-systems/areal/) -- a faithful float carrying an explicit uncertainty bit
- [Exact Arithmetic](/universal/exact-arithmetic/) -- the quire, which avoids the rounding instead of tracking it

# Arbitrary precision type design

  When a numerical computation is required to deliver a binary [yes/no|in/out] result
via a floating-point computation, the required computational precision is unknown a-priori.
In such cases, especially during exploration, an adaptive, arbitrary precision arithmetic
can be used to study the results and the computational dynamics that lead to that result.

There are two adaptive arithmetic floating-point types that have been developed:
<ol>
<li>a value modeled as a triple, < sign, exponent, adaptive multi-limb significant > </li>
<li>a value modeled as a composite sum of floating-point values </li>
</ol>

The first construction is called a **multi-digit** format, while the second type is referred to as **multi-component**.
The benefit of a multi-component format is the sparse representation of complex values, but a disadvantage is computational results need to be normalized before storage or continued calculations. In contrast, a multi-digit format can be used without normalization but tends to be verbose.

Another benefit of the multi-component format is that it can be executed on existing floating-point hardware resulting in a relatively high-performance (tens of MOPS on GOPS hardware). However, the fixed exponent of the hardware will limit the attainable precision.

The <i>Universal</i> library supports both representations, in static and elastic (=adaptive) incarnations.

* multi-digit
  * static
    > areal: faithful format with an uncertainty bit

    > cfloat: a classic binary floating-point

    > dfloat: decimal floating-point

    > hfloat: hexadecimal floating-point
    
  * elastic
    > efloat: elastic binary floating-point

* multi-component
  * static
    > dd (double-double)
    
    > qd (quad-double)
    
  * elastic
    > ereal: elastic real, adaptive precision approximation to exact
    
    > elreal: exact lazy real


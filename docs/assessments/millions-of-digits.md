# Millions of digits of accuracy

Generating millions of digits of Pi requires a completely different approach from multi-component arithmetic.

Douglas Priest's multi-component arithmetic (often generalized as floating-point expansions, similar to double-double or quad-double) represents a high-precision number as an unevaluated sum of standard hardware floating-point numbers ($x = x_1 + x_2 + ... + x_n$). This is brilliant for 100 to 300 digits because it heavily leverages hardware FPUs. However, the multiplication of two $n$-component numbers requires $O(n^2)$ operations, and the continuous need to normalize overlapping mantissas introduces massive overhead. Once you get past a few hundred digits, this $O(n^2)$ scaling hits a brick wall.

To calculate millions or trillions of digits in seconds, software like **y-cruncher** (and libraries like GMP or MPFR) relies on a completely different paradigm built around **arbitrary-precision integer arrays (bignums)**, **asymptotically fast multiplication**, and **divide-and-conquer series evaluation**.

Here is how it is done:

### 1. The Representation: Big Integers, Not Multiple Floats

Instead of chaining together standard floats, these programs treat numbers as giant arrays of 32-bit or 64-bit integers ("limbs"). A one-million-digit number is simply represented as a massive integer in base-$2^{64}$, stored in contiguous memory. Floating-point math is handled separately by keeping track of a single exponent for the entire array, much like scientific notation.

### 2. The Engine: Asymptotically Fast Multiplication

The real secret to scaling to millions of digits is breaking the $O(n^2)$ multiplication barrier. Addition and subtraction are linear $O(n)$, but if multiplication is $O(n^2)$, calculating a million digits would require a trillion operations. To solve this, large-scale arithmetic uses a hierarchy of multiplication algorithms depending on the size of the numbers:

* **Small numbers (1-30 limbs):** Standard $O(n^2)$ "schoolbook" multiplication is used because the CPU handles it optimally.
* **Medium numbers (thousands of digits):** Algorithms like **Karatsuba** ($O(n^{1.58})$) or **Toom-Cook 3-way** ($O(n^{1.46})$) are used. These algorithms split the large integer arrays into smaller chunks and use clever algebraic tricks to trade costly multiplications for cheaper additions.
* **Massive numbers (millions of digits):** Programs switch to **Fast Fourier Transform (FFT)** or **Number-Theoretic Transform (NTT)** multiplication, typically via the **Schönhage–Strassen algorithm**.  By treating the arrays of numbers as coefficients of polynomials, they can be multiplied in $O(n \log n \log \log n)$ time. This is the single most important algorithmic breakthrough that makes y-cruncher possible.

### 3. The Formula: The Chudnovsky Algorithm

You cannot compute millions of digits of Pi efficiently using simple geometric series (like the Gregory-Leibniz $\arctan$ formulas). Today's record-breaking software almost exclusively uses the **Chudnovsky algorithm**.

It is a generalized hypergeometric series based on Ramanujan’s work. Its primary advantage is its extreme rate of convergence: **every single term computed adds about 14 correct decimal digits of Pi**. To get 14 million digits, you "only" need to compute 1 million terms.

### 4. The Evaluation Strategy: Binary Splitting

Even with the Chudnovsky algorithm, if you calculate the series naively by adding term 1, then term 2, then term 3, you would have to perform a massive FFT-based division and addition for every single step.

Instead, they use a technique called **Binary Splitting**.
Rather than calculating floats sequentially, Binary Splitting computes the series using exact integer fractions in a divide-and-conquer tree structure:

1. It pairs up adjacent terms in the series and combines them into exact rational fractions (e.g., combining term 1 and term 2 into $A/B$, and term 3 and term 4 into $C/D$).
2. It then combines those results ($A/B$ and $C/D$ become $(AD+BC)/BD$).
3. It builds this massive binary tree all the way to the top.

Because of this tree structure, the vast majority of the math is done on relatively small integer arrays at the bottom of the tree. Only at the very root of the tree do you end up with one unimaginably large integer numerator and one unimaginably large integer denominator. **You only perform one massive, million-digit floating-point division at the very end.**

### 5. Systems-Level Engineering (y-cruncher's Specialty)

While GMP and MPFR provide the math, y-cruncher is famously fast because Alexander Yee (the creator) hyper-optimized the system-level execution:

* **Cache Locality:** FFT multiplication jumps all over computer memory. y-cruncher optimizes memory access patterns so that data stays in the CPU's L2/L3 cache as much as possible.
* **Swap Space Management:** When calculating trillions of digits, you run out of RAM. y-cruncher is explicitly designed to use RAID-0 arrays of NVMe SSDs as an extension of RAM, hiding the disk I/O latency behind multi-threaded CPU computations.

**Summary for a possible Universal implementation:**
The Douglas Priest implementation in `universal` is a good fit for 300 digits—it minimizes state overhead and is very fast for "small large" numbers. 
But to break the million-digit barrier, multi-component floats are not the correct abstraction. Instead, contiguous integer arrays, and 
an FFT-based multiplication engine combined with Binary Splitting is required.

## Academic Papers

If you are looking to dive deep into the mechanics of these asymptotically fast multiplication algorithms, there are several excellent papers and surveys ranging from beginner-friendly overviews to rigorous mathematical treatises.

Since you are already working on advanced arithmetic emulation, you will likely appreciate both the theoretical foundations and the practical implementation details. Here are a few standout papers to guide your learning:

### 1. The Undergraduate-Friendly Introduction

**"Large Integer Multiplication: A Friendly Survey of Algorithms"** by Xiang Huang

* **Why read it:** If you want a gentle ramp-up before diving into heavy algebraic rings, this is the perfect starting point. The author explicitly wrote this survey to help undergraduate students look past the raw complexity of these algorithms. It walks through standard grade-school multiplication, introduces Karatsuba as the first major historical breakthrough, and then generalizes it into Toom-Cook. It provides concrete examples and focuses on the "beauty and wonder" of the algorithms rather than just dense proofs.

### 2. The Comprehensive Mathematical Bible

**"Multidigit Multiplication for Mathematicians"** by Daniel J. Bernstein (2001)

* **Why read it:** This is widely considered the gold-standard survey on the topic. Bernstein is a titan in computational number theory and cryptography. The paper systematically covers Karatsuba, Toom, their duals, the "FFT trick," and the Schönhage–Strassen method.
* **What you will learn:** Bernstein frames almost every multiplication technique—whether Karatsuba or FFT—as a combination of "mapping" and "lifting" between varieties of commutative rings. It is dense, but once you grasp his framework, you will understand the underlying mathematical architecture that connects all these seemingly disparate algorithms.

### 3. The Modern Practical Application (NTT)

**"Efficient Multiplication of Somewhat Small Integers Using Number-Theoretic Transforms"** (Becker et al., 2022)

* **Why read it:** Conventional wisdom often dictates that NTT/FFT-based multiplication (like Schönhage–Strassen) only becomes faster than Karatsuba or Toom-Cook for numbers with tens of thousands of digits. However, this paper explores the practical "crossover point" in modern hardware (Becker et al., 2022).
* **What you will learn:** It is a fantastic read for understanding the gap between asymptotic complexity ($O(n \log n)$) and concrete, real-world complexity. The authors demonstrate how NTT can be optimized to outperform big-number arithmetic for integers as small as 2048 bits on microcontrollers. This is highly relevant if you are thinking about how to optimize software implementations.

### How to approach them

I recommend starting with Huang's paper to get a solid intuitive grasp of how divide-and-conquer splits the operands (Karatsuba and Toom-Cook). Once you are comfortable with how polynomials are used to represent these splits, move on to Bernstein's paper to understand how FFT evaluates those polynomials at roots of unity to achieve $O(n \log n)$ scaling.

---

**References**

Becker, H., Hwang, V., Kannwischer, M. J., Panny, L., & Yang, B.-Y. (2022). Efficient Multiplication of Somewhat Small Integers Using Number-Theoretic Transforms. *Lecture Notes in Computer Science*, 3-23. [https://doi.org/10.1007/978-3-031-15255-9_1](https://doi.org/10.1007/978-3-031-15255-9_1)
Cited by: 10

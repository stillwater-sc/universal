# Notes on Chebyshev nodes



### Chebpts
```chebpts(n,kind)``` returns n Chebyshev nodes of the 1st or 2nd Kind.  
Chebyshev nodes of the 1st kind are roots of Chebyshev polynomial.
The 2nd kind are critical points.  

Monic Chebyshev polynomials have property that their maximum value
on the interval [-1,1] = 2^{1-n}, which is less than all other 
monic polynomials of degree n. (Burden & Faires, 2011). 


```diff(x,y)``` returns the difference elementwise of the vectors x and y.


```linscale(x,c,d)``` linearly scales and shifts data points x to [c,d]


```meandistance(x)``` returns Geometric mean of the differences of $ x_i$ with x_j 
\forall j \ne i.  


```prod(x)``` returns the product of the elements in vector x.






## References:

Burden, R. L., & Faires, J. D. (2011). Numerical analysis.



## BibTeX:

@misc{burden2011numerical,
  title={Numerical analysis},
  author={Burden, Richard L and Faires, J Douglas},
  year={2011},
  publisher={Cengage Learning}
}


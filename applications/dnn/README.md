# examples/dnn

Deep Learning examples

## How to build

The examples are automatically build by cmake.

## Mixed-Precision DNN 

MNIST hand-written digits characterization using a mixed-precision DNN model.

## MatMul schedules

inner-product method
ijk order; only a single dot product can be executed in parallel
```code
for (i = 0; i < N; ++i) {
	for (j = 0; j < N; ++j) {
		for (k = 0; k < N; ++k) {
			U[i,j] = U[i,j] + W[i,k] * Y[k,j];
		}
	}
}
```

inner-product ijk order; dot product is unrolled to be executed in parallel
```text
U[i, j] += W[i, k] * Y[k, j]
U[0, 0] += W[0, 0] * Y[0, 0]
U[0, 0] += W[0, 1] * Y[1, 0]
U[0, 0] += W[0, 2] * Y[2, 0]
U[0, 1] += W[0, 0] * Y[0, 1]
U[0, 1] += W[0, 1] * Y[1, 1]
U[0, 1] += W[0, 2] * Y[2, 1]
U[0, 2] += W[0, 0] * Y[0, 2]
U[0, 2] += W[0, 1] * Y[1, 2]
U[0, 2] += W[0, 2] * Y[2, 2]
U[1, 0] += W[1, 0] * Y[0, 0]
U[1, 0] += W[1, 1] * Y[1, 0]
U[1, 0] += W[1, 2] * Y[2, 0]
U[1, 1] += W[1, 0] * Y[0, 1]
U[1, 1] += W[1, 1] * Y[1, 1]
U[1, 1] += W[1, 2] * Y[2, 1]
U[1, 2] += W[1, 0] * Y[0, 2]
U[1, 2] += W[1, 1] * Y[1, 2]
U[1, 2] += W[1, 2] * Y[2, 2]
U[2, 0] += W[2, 0] * Y[0, 0]
U[2, 0] += W[2, 1] * Y[1, 0]
U[2, 0] += W[2, 2] * Y[2, 0]
U[2, 1] += W[2, 0] * Y[0, 1]
U[2, 1] += W[2, 1] * Y[1, 1]
U[2, 1] += W[2, 2] * Y[2, 1]
U[2, 2] += W[2, 0] * Y[0, 2]
U[2, 2] += W[2, 1] * Y[1, 2]
U[2, 2] += W[2, 2] * Y[2, 2]
```

## middle-product method

jki order; N dot products can be executed in parallel
```code
for (j = 0; j < N; ++j) {
	for (k = 0; k < N; ++k) {
		for (i = 0; i < N; ++i) {
			U[i,j] = U[i,j] + W[i,k] * Y[k,j];
		}
	}
}
```

middle-product jki order; N dot products can be executed in parallel
```text
U[i, j] += W[i, k] * Y[k, j]
U[0, 0] += W[0, 0] * Y[0, 0]
U[1, 0] += W[1, 0] * Y[0, 0]
U[2, 0] += W[2, 0] * Y[0, 0]
U[0, 0] += W[0, 1] * Y[1, 0]
U[1, 0] += W[1, 1] * Y[1, 0]
U[2, 0] += W[2, 1] * Y[1, 0]
U[0, 0] += W[0, 2] * Y[2, 0]
U[1, 0] += W[1, 2] * Y[2, 0]
U[2, 0] += W[2, 2] * Y[2, 0]
U[0, 1] += W[0, 0] * Y[0, 1]
U[1, 1] += W[1, 0] * Y[0, 1]
U[2, 1] += W[2, 0] * Y[0, 1]
U[0, 1] += W[0, 1] * Y[1, 1]
U[1, 1] += W[1, 1] * Y[1, 1]
U[2, 1] += W[2, 1] * Y[1, 1]
U[0, 1] += W[0, 2] * Y[2, 1]
U[1, 1] += W[1, 2] * Y[2, 1]
U[2, 1] += W[2, 2] * Y[2, 1]
U[0, 2] += W[0, 0] * Y[0, 2]
U[1, 2] += W[1, 0] * Y[0, 2]
U[2, 2] += W[2, 0] * Y[0, 2]
U[0, 2] += W[0, 1] * Y[1, 2]
U[1, 2] += W[1, 1] * Y[1, 2]
U[2, 2] += W[2, 1] * Y[1, 2]
U[0, 2] += W[0, 2] * Y[2, 2]
U[1, 2] += W[1, 2] * Y[2, 2]
U[2, 2] += W[2, 2] * Y[2, 2]
```

## outer-product method

kij order; each dot products can be executed in parallel
```code
for (k = 0; k < N; ++k) {
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			U[i,j] = U[i,j] + W[i,k] * Y[k,j];
		}
	}
}
```

outer-product kij order; each dot product can be executed in parallel
```text
U[i, j] += W[i, k] * Y[k, j]
U[0, 0] += W[0, 0] * Y[0, 0]
U[0, 1] += W[0, 0] * Y[0, 1]
U[0, 2] += W[0, 0] * Y[0, 2]
U[1, 0] += W[1, 0] * Y[0, 0]
U[1, 1] += W[1, 0] * Y[0, 1]
U[1, 2] += W[1, 0] * Y[0, 2]
U[2, 0] += W[2, 0] * Y[0, 0]
U[2, 1] += W[2, 0] * Y[0, 1]
U[2, 2] += W[2, 0] * Y[0, 2]
U[0, 0] += W[0, 1] * Y[1, 0]
U[0, 1] += W[0, 1] * Y[1, 1]
U[0, 2] += W[0, 1] * Y[1, 2]
U[1, 0] += W[1, 1] * Y[1, 0]
U[1, 1] += W[1, 1] * Y[1, 1]
U[1, 2] += W[1, 1] * Y[1, 2]
U[2, 0] += W[2, 1] * Y[1, 0]
U[2, 1] += W[2, 1] * Y[1, 1]
U[2, 2] += W[2, 1] * Y[1, 2]
U[0, 0] += W[0, 2] * Y[2, 0]
U[0, 1] += W[0, 2] * Y[2, 1]
U[0, 2] += W[0, 2] * Y[2, 2]
U[1, 0] += W[1, 2] * Y[2, 0]
U[1, 1] += W[1, 2] * Y[2, 1]
U[1, 2] += W[1, 2] * Y[2, 2]
U[2, 0] += W[2, 2] * Y[2, 0]
U[2, 1] += W[2, 2] * Y[2, 1]
U[2, 2] += W[2, 2] * Y[2, 2]
```

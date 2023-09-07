# Mixed-Precision Serialization

*Universal* contains many parameterized types, and the BLAS library can hold aggregate data structures 
of those types. When we need to serialize the content of these data structures, we need to record 

1. a type identifier to declare the custom type parameterization
2. a data structure identifier to declare the aggregate data structure (vector, matrix, tensor, ...)
3. the raw data contained in the data structure

When we need to serialize a collection of data structures, we need to add a token to indicate that a new
data structure segment is contained in the file. This way, we can make these files incremental and we
can produce them incrementally. Restoring them now only requires enumerating the data structure segment 
tokens, and for every new segment, consume the next data structure of the structure described above.

To record structure among the data structures contained in the data structure segments, we would need
to add an identifier list. One example is to serialize a DNN graph of layers and activation functions.
However, data structure is a meta layer on top of raw data, and it is advantageous to separate the two.
Thus, we have a serialization format of a set of data aggregations, such as vectors, matrices, and tensors.
And we have a serialization format for structure that makes references to the data structure identifiers.

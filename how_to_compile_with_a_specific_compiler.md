# How to compile with a specific compiler

Universal has a set of builder containers that package a specific compiler.
Using the .devcontainer/devcontainer.json mechanism of VSCode you can control
which build container you connect to in the VSCode terminal.

Once you are in the terminal, create a build directory for your compiler.
For example, if you are using MSVC, create a directory named `build_msvc`,
if you are using clang, create a directory named `build_clang`, and if
you are using gcc, `build_gcc`.

Then invoke cmake with the correct compiler flags. For example:

```
cmake -DUNIVERSAL_BUILD_ALL=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
```

or

```
cmake -DUNIVERSAL_BUILD_ALL=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
```

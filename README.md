# R3MG-C

Development repository for the code assembling the transfer matrices needed to
implement R3MG-C in any algebraic multigrid context.

The library agglomerates a fine discretization (a cloud of support points, or a
mesh of cells together with its DoF numbering) through an R*-tree, and returns
the prolongation operators between the levels of the resulting hierarchy in
COO format, ready to be assembled by any AMG framework (PETSc, Trilinos,
MATLAB, Python, ...).

**Boost is the only dependency** (header-only components only).

## Status

| Step | Content                                                  | Status |
|------|----------------------------------------------------------|--------|
| 1    | Project skeleton, `Point`, `BoundingBox`, R-tree wrapper | done   |
| 2    | R-tree agglomerator (agglomerates + hierarchy)           |        |
| 3    | Lagrange `Q^p` basis on boxes                            |        |
| 4    | Sparse matrices, COO / Matrix Market output              |        |
| 5    | Point-based transfers                                    |        |
| 6    | Cell-based transfers                                     |        |
| 7    | Composite transfers, multi-component tests               |        |
| 8    | Continuous integration                                   |        |

## Layout

```
include/r3mgc/base/            Point, BoundingBox, exceptions, index types
include/r3mgc/boost_adaptors/  Boost.Geometry traits for Point and BoundingBox
include/r3mgc/rtree/           R-tree construction, traversal, level extraction
tests/<module>/                deal.II-style tests: <name>.cc + <name>.output
cmake/                         test pickup macro and test driver
```

## Building and testing

Requirements: a C++17 compiler, CMake >= 3.16, Boost >= 1.70.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBoost_ROOT=/path/to/boost
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The default build type is `Debug`, in which the internal consistency checks
(`R3MGC_Assert`) are active. Use `-DCMAKE_BUILD_TYPE=Release` to disable them.

### Tests

Tests work like the ones of deal.II. Every file `tests/<module>/<name>.cc` is a
program that writes its results to `r3mglog` (see `tests/tests.h`). The output
is compared with the expected output `<name>.output`, or with
`<name>.debug.output` / `<name>.release.output` if the output depends on the
build type. To add a test, write the `.cc` file, run it once, check its output
*by hand*, and save it as the expected output. Modules are registered in
`tests/CMakeLists.txt`. `ctest -R rtree/` runs the tests of a single module.

### Code style

The code follows the deal.II style, enforced by the `.clang-format` file (the
one of deal.II, with the include categories adapted to R3MG-C):

```sh
clang-format -i $(find include tests -name "*.h" -o -name "*.cc")
```

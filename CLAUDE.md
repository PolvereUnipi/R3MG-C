# R3MG-C — notes for Claude

Standalone C++17 library that builds the R3MG-C transfer (prolongation)
matrices by R*-tree agglomeration and outputs them in COO format, for use in
any AMG framework (PETSc, Trilinos, MATLAB, Python). Method: paper at
`~/GAMG_articolo/699dca351b10396794446382/main.tex`, §4 "Multilevel
preconditioner".

## Reference code (read-only)

- `~/Polydeal`, branch `continuous_agglo_implementation`: the deal.II-based
  reference implementation, and the oracle for golden outputs.
  Key files: `include/continuous_agglo_utils.h`
  (`PointsAgglo`/`CellsAgglo::agglomerate_and_compute_injection_matrices`,
  `fill_injection_matrix`), `include/agglomerator.h` (`Rtree_visitor`,
  `CellsAgglomerator`), `test/polydeal/*continuous*`.
- `~/dealii`: deal.II sources, for style and for code to port. A built copy in
  `~/dealii/build` is used by throw-away cross-check programs (scratchpad only,
  never in the repo).

## Hard constraints

- **Boost is the only dependency**, header-only components only. No deal.II, no
  gtest, no Python in the build. VTK may become an *optional* dependency later.
- Do not reimplement the fine FE or mapping. The user provides the fine DoF data:
  support points, DoFs per support point, cell vertices, cell→DoF connectivity.
- **Coarse basis**: equidistant Lagrange `Q^p'` on each agglomerate box,
  identical to deal.II `FE_DGQ` (nodes and lexicographic numbering). GLL nodes
  only concern the user's fine space.
- **Non-admissible hierarchy ⇒ throw** (never patch boxes). Throw on:
  - a degenerate box, judged by the box's own aspect ratio, not by a global
    length scale;
  - fewer than `(p'+1)^dim` assigned points;
  - a coarse basis function that is zero on all assigned points (zero column);
  - a local interpolation matrix without full column rank.
- **Degrees**: the same degree on all levels by default. A per-level vector is
  allowed, but degrees must never increase towards coarser levels.
- **Transfers**: consecutive prolongations by default. Composite fine→level
  transfers only on request.
- **Multi-component** support from the start (several DoFs on one support point).
- Global indices are `std::uint64_t` (`types::global_dof_index`,
  `types::global_cell_index`); local quantities are `unsigned int`.

## Layout and conventions

- `include/r3mgc/<module>/*.h`, namespace `r3mgc` (opened with
  `R3MGC_NAMESPACE_OPEN/CLOSE`), include guards `r3mgc_<module>_<file>_h`,
  `r3mgc/base/config.h` included first.
- Templated R-tree code stays header-only. Everything templated on `dim` will go
  in `source/*.cc` with explicit instantiations for dim = 1, 2, 3, once the
  library stops being INTERFACE-only.
- Exceptions: `R3MGC_Assert` (debug only, throws) and `R3MGC_AssertThrow`
  (always). Declare new exceptions with `R3MGC_DeclException{Msg,1,2,3}`. Always
  keep the `R3MGC_` prefix.
- deal.II style: Doxygen `/** */` on every class/function explaining *why* it
  exists, three blank lines between definitions, inline/template definitions
  below a `/* --- Inline functions --- */` separator.
- Format with the repo `.clang-format` (deal.II's style), before every commit:
  `clang-format -i $(find include tests -name "*.h" -o -name "*.cc")`.
- License header: the SPDX block used in existing files. No LICENSE file has
  been chosen yet.

## Build and test

There is no system cmake. Use the spack view:

```sh
export PATH=~/spack/var/spack/environments/dealii-env/.spack-env/view/bin:$PATH
V=~/spack/var/spack/environments/dealii-env/.spack-env/view
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBoost_ROOT=$V
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBoost_ROOT=$V
cmake --build build -j8 && (cd build && ctest --output-on-failure)
```

Always run **both** Debug and Release. Tests must build with no warnings
(`-Wall -Wextra -Wpedantic`).

### Tests (deal.II style)

- `tests/<module>/<name>.cc` writes to `r3mglog` (lines prefixed
  `R3MG:<prefixes>::`). After `initlog()`, the log goes to `output`, and ctest
  compares that file with `<name>.output`, or with
  `<name>.debug.output` / `<name>.release.output` when the output depends on
  `R3MGC_Assert`.
- New module: add `r3mgc_pickup_tests(<module>)` to `tests/CMakeLists.txt`.
- Workflow for a new test: create an empty `.output`, build, run, **check the
  output by hand** (and against deal.II/Polydeal when possible), then copy
  `build/tests/<module>/<name>/output` over it. Never accept an output blindly.
- Use `check_throw(stmt)` to log expected exceptions (class name + message).
  Random input: use the raw `std::mt19937_64` output, never
  `std::uniform_real_distribution`, which differs between standard libraries.
- R*-tree packing is deterministic for a fixed input order. Keep input order
  fixed in tests.

## Roadmap

Full plan: `~/.claude/plans/hello-what-i-want-enumerated-kitten.md`.

1. [done] Skeleton, `Point`, `BoundingBox`, Boost adaptors, R-tree wrapper
   (`pack_rtree`, `pack_rtree_of_indices`, `visit_rtree`, `n_levels`,
   `extract_rtree_level`), test infrastructure.
2. R-tree agglomerator: port `Rtree_visitor`/`CellsAgglomerator` with a generic
   leaf type (no deal.II iterators). Output: agglomerates and the parent→children
   hierarchy per level. Golden test vs Polydeal.
3. Lagrange `Q^p` on the reference box (equidistant, = `FE_DGQ`), golden values
   from deal.II.
4. LAC: `FullMatrix` (column-pivoted QR rank), dynamic sparsity → CSR,
   `distribute_local_to_global`, COO export, Matrix Market writer.
5. Point-based transfers + admissibility checks. Golden test vs Polydeal.
6. Fine mesh input description + cell-based transfers (first-come DoF
   assignment). Golden test vs Polydeal.
7. Composite prolongation (optional mode), multi-component test cases.
8. GitHub Actions CI (GCC + Clang, Debug/Release, clang-format check).
9. Later: optional VTK output, C API / Python bindings, MPI.

Bug to **not** port: Polydeal's `fill_injection_matrix` uses the coarse FE's
support points and dofs-per-cell for the fine box too, which is wrong when the
degrees of consecutive levels differ.

Open decision (step 5): coarse DoF numbering for multi-component spaces
(interleaved `(agglo*n_dofs_per_cell + k)*n_comp + c`, or block-by-component).

Work one step at a time. Commit after each step, and update the roadmap
status above when a step is done.

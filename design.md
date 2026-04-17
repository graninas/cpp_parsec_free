# Design notes and recommendations

This document captures concise observations and actionable recommendations for the current parser library implementation (free-monad style parser represented with `ParserADT`, `ParserL`, `PureF`/`FreeF`, and visitors for `fmap` / `bind`). The tone is factual and focused on maintainability, correctness and performance.

## Overview

- The library models parsers as a small free-language: method ADTs (e.g. `ParseSymbolCond`, `ParseMany`) plus a Free layer (`ParserL` variant of `PureF`/`FreeF`).
- `fmap` and `bind` are implemented by visitors over `std::variant`, producing transformed ADTs and composing `ParserL` values.
- Execution happens via a separate runtime/interpreter (not modified here) which consumes the ADT representation.

## Strengths

- Clear separation of concerns: ADT (methods) vs Free layer vs interpreter. This makes reasoning about effects and transformations straightforward.
- `std::variant` + visitors provide explicit, type-safe handling of cases and make it easy to extend the language with new methods.
- `fmap`/`bind` implemented directly on ADT enables programmatic transformations and potential optimizations at the method level.
- Public API is small and functional: `pure`, `fmap`, `bind`, plus combinators; this is familiar to functional programmers.

## Risks and pain points

- Heavy use of `std::function` and capturing lambdas may cause hidden heap allocations and degrade performance in hot parsing paths.
- Use of `std::shared_ptr<ParserL>` inside `ParseMany::raw_parser` combined with closures could lead to reference cycles or unexpected lifetime extensions.
- Semantics of `raw_parser` in `ParseMany` are not fully documented: copied as-is in `fmap`/`bind` which may or may not be correct in all cases.
- Error and diagnostic information is minimal: no structured error type, and limited position/trace metadata in ADTs.
- No explicit validation of monad laws; subtle bugs can appear in bind/fmap implementations without property tests.

## Performance recommendations

- Replace `std::function` with template-based callables where possible (templated visitors or function objects) to enable inlining and avoid dynamic allocations. Consider `std::function` only at public boundaries or where type erasure is necessary.
- If type-erasure is required but `std::function` is too costly, consider `function_ref` (non-owning) for short-lived callbacks or a small-buffer-optimized function wrapper.
- Reduce allocations by avoiding captured-by-value heavy closures. Capture by reference where lifetime is guaranteed, or store light-weight indices/ids instead of large objects.
- Profile typical parsing workloads and benchmark chains of `fmap`/`bind` to find fusion opportunities and eliminate intermediate allocations.

## Memory / lifetime recommendations

- Audit ownership: where `shared_ptr` is used, verify whether `unique_ptr` or `weak_ptr` would be more appropriate to prevent cycles. Document lifetime invariants for `ParserL` instances.
- Consider separating the static AST (immutable parser description) from runtime state pointers so that ASTs can be trivially shared and cheaply copied.

## Correctness and API suggestions

- Add property tests for monad laws (left identity, right identity, associativity) to ensure `bind` / `fmap` obey expected behavior.
- Document semantics for `ParseMany::raw_parser` and how method-level `next` functions are invoked and combined.
- Consider an explicit `Result` or `Either` type in ADTs for richer error propagation and better error messages from the interpreter.

## Structural / architectural suggestions

- Consider a 2-stage design: build an immutable AST representation of composed parsers (the `Free` layer) then run a separate interpreter. This is already close to current design; refining the boundaries makes optimizations (fusion, specialization) easier.
- Provide an optimizer/fusion pass that rewrites common patterns of `fmap`/`bind` into a more efficient single ADT or direct interpreter steps.
- Expose a `pretty-print` / serialization facility for ADT structures to aid debugging and tests.

## Testing and validation

- Add unit tests covering: individual ADT constructors, `methods_fmap`, `fmap`, `bind`, and end-to-end parsing for common combinator compositions.
- Add property-based tests for monad laws (e.g., with RapidCheck/QuickCheck-style testing) to detect law violations.
- Add benchmarks that measure: single parser performance, deep chains of `fmap`/`bind`, and high-throughput `many`/`ParseMany` scenarios.

## Backwards-compatibility and ergonomics

- Keep the simple functional API (`pure`, `fmap`, `bind`) for consumers but optimize the internal representations incrementally.
- Provide convenience overloads and type aliases to reduce verbosity of `std::function` types in user code.

## Next incremental tasks (practical roadmap)

1. Add monad-law unit/property tests for `bind`/`fmap`.
2. Replace a hot-path `std::function` with a templated callable (proof-of-concept) and benchmark the improvement.
3. Audit `shared_ptr` usage and remove cycles (or document and switch to `weak_ptr` where needed).
4. Add a debug pretty-printer for `ParserL` / `ParserADT`.
5. Implement a small optimizer pass that fuses successive `fmap` operations.

## Closing note

The current design is conceptually solid and matches common functional/free-monad approaches. The main work remaining is pragmatic: tightening lifetime semantics, reducing dynamic allocations in hot paths, and adding tests/benchmarks to validate both correctness and performance.

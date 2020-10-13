# ce
Useful tools for constexpr work.

## cvector

A fixed-capacity constexpr vector that supports types without trivial
constructors, while preserving their `is_trivial*` set of types. Requires C++20
and a compiler that supports concepts, with the `concepts` header.

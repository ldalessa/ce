# ce
Useful tools for constexpr work.

## cvector

A fixed-capacity constexpr vector that supports types without trivial
constructors, while preserving their `is_trivial*` set of types. Requires C++20
and a compiler that supports concepts, with the `concepts` header.

The general idea is to use an array of `union { T t }` to store the elements of
the array, as such unions can be created without initializing the underlying
`T`. This roughly approximates the same behavior you'd expect from a `std::byte`
array that you cast to `T*`.

### `P0848` issues

The complication of the `union` approach is preserving the trivial traits. This
requires logic to declare or default the `union`'s special member functions, and
the `cvector`'s as well. This is relatively trivial to do if the compiler
properly supports `P0848` 
(http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0848r3.html), however
`clang` is missing such support at the moment.

Working around missing `P0848` requires some annoying policy-based inheritance
design, which I've attempted to encapsulate in the `include/ce/P0848.hpp`
header. The `cvector_base` class template implements the vector using a `union`
array that is declared using `P0848` helpers, and then the `cvector` class
template sorts out the `T`-relative `P0848` support via policy inheritance.

### Iterator issues

There's one additional complication that the library deals with, an array of
`union { T t; }` can't be treated as an array of `T`, so the `cvector` has to
use a random-access iterator wrapper class template, also provided in `P0848`
even though it's not really germane to `P0848` (even with support, we still need
the iterators).

This provided iterator is _only_ `random access`. It is not `contiguous` because
of the `union`'s presence (even though there's not a world where the _actual_
underlying stored `T`s are not contiguous). This means that algorithms and data
structures that require `contiguous` iterators (like `string_view`) cannot be
used.

I have made two compromises to deal with this restriction.

1. I have a `cvector` specialization for truly trivial type (e.g., `T=char`)
   that uses a raw array rather than a `union` array, and exports raw `T*` as
   iterators. These are `contiguous` and can be used in either `constexpr` or
   normal contexts as such.

2. I have provided a `data()` function for the non-trivial `cvector_base` that
   returns a `T*` by reinterpreting the `union { T t; } storage[]` as a `T*`. As
   far as I understand this is 100% safe and provides a `contiguous` interface
   to the underlying array, but it is not valid in `constexpr` context as
   `reinterpret_cast` is unavailable (which is the reason that this project is
   hard in the first place).

### API notes

My `cvector` API is modeled on, but not a clone of, `std::vector`. I changed
some things that I don't like (e.g., `push_back` returns references and such). I
don't use unsigned types so vector size and capacity are stored as `int`---these
could be `long` but in  `constexpr`-land that didn't seem necessary. I haven't
implemented a bunch of the `insert`-style API either, as it hasn't been
necessary for me (pull requests would be fine).

Finally I have only a limited set of constructors.

1. The default constructor allocates an empty, `0`-sized vector.
2. The single `int` constructor allocates an `n`-sized vector, but is only
   available if `T` has a default constructor.
3. There are two tagged constructors for in-place initialize (tagged with
   `std::in_place` and `std::in_place_type`). These also provide CTAD
   inference. 

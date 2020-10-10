#pragma once

// A C++20 constexpr vector implementation.
//
// This is not an implementation of std::vector for constexpr context, but
// simply an implementation of a constexpr vector with a fixed capacity. The
// only thing special about it is that it is intended to properly support types
// without default constructors, while preserving the `is_trivially_*` set of
// traits from its storage type and providing contiguous iterators as if it were
// just an array of `T`.
//
// The vector itself is mostly implemented in `cvector_impl`, but should be
// accessed in user code via the `cvector` strong typedef (technically more than
// just a typedef, but only barely more). The rest of the file contains support
// code to workaround missing P0848 in clang, as well as to find some
// conservative point in the `constexpr array of unions` methodology that both
// clang and gcc are happy with.
//
// It _does_ rely on concepts, both in the compiler and the <concepts> header,
// however with some extra work the header could be traded for <type_traits>.
//
// Hopefully the API is fairly straightforward. The cvector supports CTAD much
// like the std::vector, but does not provide any other sophisticated
// initialization. Its push and pop operations will return references and values
// for your convenience.
//
// constexpr auto foo()
// {
//   const char* hello = "hello world\n";
//   cvector<char, 24> v;
//   for (char c : hello) {
//     v.push_back(c);
//   }
//   return v;
// }
//
// The vector is only lightly tested.
//
// ******** NOTE ABOUT STYLE ********
// I assert preconditions on the same line as the function declaration. This
// produces useful text output from constexpr errors, as both the function
// declaration and assertion will be printed. I also like to do some manual
// horizontal alignment where I feel like it looks more consistent.
// **********************************

#include <cassert>
#include <concepts>
#include <memory>

namespace ce
{
// Everything in this namespace exists because clang has not yet implemented
// P0848 at the time that I'm writing this. The result is that we need a bunch
// of extra nonsense to make sure that our storage type and vector don't
// unintentionally restrict the set of `is_trivially_*` traits more than they
// have to. I believe they are all preserved other than the fact that the
// vector is not `trivially_constructible` because it has default initializers
// for its data.
namespace P0848
{

// The storage union is an integral part of our vector implementation. It wraps
// a single instance of a `T` and allows us to allocate an array of them
// without actually initializing any of them, which is what we want when `T` is
// not `trivially_constructible`.
//
// We need 2^4 different versions of the union depending on the underlying
// traits of the `T`. We can't do fancy meta-programming with the union because
// we don't have inheritance available, so instead we define the 16 partial
// specialization via set of P0848_MACRO expansions.
template <typename T,
          bool = std::is_trivially_constructible_v<T>,
          bool = std::is_trivially_destructible_v<T>,
          bool = std::is_trivially_copy_constructible_v<T>,
          bool = std::is_trivially_move_constructible_v<T>>
union storage;
// {
//   T t;
//
//   constexpr constructors
//   cosntexpr assignment
// }

// A basic macro to protect a parameter from expansion.
#define P0848_WRAP(x) x

// These macros just pick either a defaulted or empty version of the relevant
// constructor, based on the passed defaulted flag. The `type` is only
// `storage` in this application, but parameterizing it makes it a bit more
// generic.
#define P0848_MAKE_CTOR_true(type)  constexpr type() = default
#define P0848_MAKE_CTOR_false(type) constexpr type() {}
#define P0848_MAKE_CTOR(defaulted, type)        \
  P0848_MAKE_CTOR_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_DTOR_true(type)  constexpr ~type() = default
#define P0848_MAKE_DTOR_false(type) constexpr ~type() {}
#define P0848_MAKE_DTOR(defaulted, type)        \
  P0848_MAKE_DTOR_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_COPY_true(type)  constexpr type(const type&) = default
#define P0848_MAKE_COPY_false(type) constexpr type(const type&) {}
#define P0848_MAKE_COPY(defaulted, type)        \
  P0848_MAKE_COPY_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_MOVE_true(type)  constexpr type(type&&) = default
#define P0848_MAKE_MOVE_false(type) constexpr type(type&&) {}
#define P0848_MAKE_MOVE(defaulted, type)        \
  P0848_MAKE_MOVE_##defaulted(P0848_WRAP(type))

// Older clang has trouble with tracking the union's active member when it
// doesn't include a monostate. It's not really germane to P0848---we would
// need it even if clang didn't need the rest of this mess.
#if (__clang_major__ < 11)
#define CLANG_NEEDS_MONOSTATE(id) struct {} id = {}
#else
#define CLANG_NEEDS_MONOSTATE(id)
#endif

// Expands into one of the storage union types.
#define P0848_MAKE(type, ctor, dtor, cctor, mctor)                      \
  template <typename T>                                                 \
  union type<T, ctor, dtor, cctor, mctor> {                             \
    T t;                                                                \
    CLANG_NEEDS_MONOSTATE(_);                                           \
    P0848_MAKE_CTOR(ctor,  P0848_WRAP(type));                           \
    P0848_MAKE_DTOR(dtor,  P0848_WRAP(type));                           \
    P0848_MAKE_COPY(cctor, P0848_WRAP(type));                           \
    P0848_MAKE_MOVE(mctor, P0848_WRAP(type));                           \
                                                                        \
    constexpr static bool copyable = std::is_trivially_copy_assignable_v<T>; \
    constexpr type& operator=(const type&)   requires( copyable) = default; \
    constexpr type& operator=(const type& s) requires(!copyable) {      \
      return (t = s.t, *this);                                          \
    }                                                                   \
                                                                        \
    constexpr static bool movable = std::is_trivially_copy_assignable_v<T>; \
    constexpr type& operator=(type&&)   requires( movable) = default;   \
    constexpr type& operator=(type&& s) requires(!movable) {            \
      return (t = std::move(s).t, *this);                               \
    }                                                                   \
  }

// Expand the 16 storage union configurations.
P0848_MAKE(storage, false, false, false, false);
P0848_MAKE(storage, false, false, false, true);
P0848_MAKE(storage, false, false, true,  false);
P0848_MAKE(storage, false, false, true,  true);
P0848_MAKE(storage, false, true,  false, false);
P0848_MAKE(storage, false, true,  false, true);
P0848_MAKE(storage, false, true,  true,  false);
P0848_MAKE(storage, false, true,  true,  true);
P0848_MAKE(storage, true,  false, false, false);
P0848_MAKE(storage, true,  false, false, true);
P0848_MAKE(storage, true,  false, true,  false);
P0848_MAKE(storage, true,  false, true,  true);
P0848_MAKE(storage, true,  true,  false, false);
P0848_MAKE(storage, true,  true,  false, true);
P0848_MAKE(storage, true,  true,  true,  false);
P0848_MAKE(storage, true,  true,  true,  true);

#undef P0848_MAKE
#undef CLANG_NEEDS_MONOSTATE
#undef P0848_MAKE_MOVE
#undef P0848_MAKE_COPY
#undef P0848_MAKE_DTOR
#undef P0848_MAKE_CTOR
#undef P0848_WRAP

// The following section of code implements the P0848 functionality for the
// vector itself, through a linear chain of specialized inheritance. From most
// to least derived, the chain looks like:
// cvector->[move->copy->dtor]->cvector_impl, where move, copy, and dtor are
// policies in the P0848 namespace and selectively add = default or {} versions
// of their respective constructors based on the vector's `value_type`.
template <typename Base,
          bool = std::is_trivially_destructible_v<typename Base::value_type>>
struct dtor : Base {};

template <typename Base>
struct dtor<Base, false> : Base
{
  constexpr  dtor() = default;
  constexpr ~dtor() { this->on_dtor(); }
  constexpr  dtor(const dtor&) = default;
  constexpr  dtor(dtor&&) = default;
  constexpr  dtor& operator=(const dtor&) = default;
  constexpr  dtor& operator=(dtor&&) = default;
};

template <typename Base,
          bool = std::is_trivially_copy_constructible_v<typename Base::value_type>>
struct copy : dtor<Base> {};

template <typename Base>
struct copy<Base, false> : dtor<Base>
{
  constexpr  copy() = default;
  constexpr ~copy() = default;
  constexpr  copy(const copy& d) { this->on_copy(d); }
  constexpr  copy(copy&&) = default;
  constexpr  copy& operator=(const copy&) = default;
  constexpr  copy& operator=(copy&&) = default;
};

template <typename Base,
          bool = std::is_trivially_move_constructible_v<typename Base::value_type>>
struct move : copy<Base> {};

template <typename Base>
struct move<Base, false> : copy<Base>
{
  constexpr  move() = default;
  constexpr ~move() = default;
  constexpr  move(const move&) = default;
  constexpr  move(move&& d) { this->on_move(std::move(d)); }
  constexpr  move& operator=(const move&) = default;
  constexpr  move& operator=(move&&) = default;
};
}

// The core vector implementation.
//
// This class template contains the definition of the vector's data and all of
// its API with the exception of the various special members (constructors,
// assignments) that we want to provide.
template <typename T, int N>
struct cvector_impl
{
  using   value_type = T;
  using storage_type = P0848::storage<T>;

  // The storage array and current size.
  storage_type storage[N] = {};
  int n = 0;

  // Underlying data (not constexpr because of the cast)
  const T* data() const { return reinterpret_cast<const T*>(&storage); }
        T* data()       { return reinterpret_cast<      T*>(&storage); }

  // Capacity and size queries.
  constexpr static int capacity()                        { return N; }
  constexpr        int size()                      const { return n; }
  constexpr friend int size(const cvector_impl& v)       { return v.n; }

  // Resizing and clearing.
  constexpr void resize(int i) { assert(0 <= i && i <= N);
    for (int j = i; j < n; ++j) {
      destroy(storage[j]);                      // shrinking
    }
    for (int j = n; j < i; ++j) {
      if constexpr (std::is_default_constructible_v<T>) {
        construct(storage[j]);                  // growing
      }
      else {
        // Can't increase size without default constructor, using the trait
        // leads to a reasonably useful error. Could just uniformly prohibit
        // resizing for non-default_constructible, but I decided not to. I
        // mostly care about `constexpr` use anyway, and this is adequate for
        // that.
        assert(std::is_default_constructible_v<T>);
      }
    }
    n = i;
  }

  constexpr void clear() {
    resize(0);
  }

  // Element access.
  constexpr const T& operator[](int i) const { assert(0 <= i && i < n);
    return storage[i].t;
  }

  constexpr T& operator[](int i) { assert(0 <= i && i < n);
    return storage[i].t;
  }

  constexpr const T& front() const { return storage[0].t; }
  constexpr       T& front()       { return storage[0].t; }

  constexpr const T& back()  const { return storage[n].t; }
  constexpr       T& back()        { return storage[n].t; }

  // Stacklike operations.
  template <typename... Ts> requires(std::constructible_from<T, Ts...>)
  constexpr T& emplace_back(Ts&&... ts) { assert(n < N);
    return construct(storage[n++], std::forward<Ts>(ts)...);
  }

  constexpr T& push_back(const T& t) { assert(n < N);
    return construct(storage[n++], t);
  }

  constexpr T& push_back(T&& t) { assert(n < N);
    return construct(storage[n++], std::move(t));
  }

  constexpr T pop_back() { assert(n > 0);
    return std::move(storage[--n].t);
  }

  // A contiguous iterator template class.
  //
  // This is a simple iterator implementation that captures a pointer to some
  // "array-like" type (a random access type that can be indexed via
  // operator[]), and indexes it. The only reason we need it is to present a
  // view of our contiguous storage union array as if it was a contiguous `T`
  // array.
  //
  // It's defined as a template so that we can use it for both our normal and
  // const iterator implementation by simply changing U.
  template <typename U>
  struct it {
    using iterator_category = std::random_access_iterator_tag;
    using      element_type = std::decay_t<decltype(std::declval<U>()[0])>;

    U*  p = nullptr;
    int i = 0;

    constexpr auto& operator[](int n) const { return (*p)[i + n]; }
    constexpr auto& operator*()       const { return (*p)[i]; }
    constexpr auto* operator->()      const { return std::addressof((*p)[i]); }

    constexpr it& operator++() { return (++i, *this); }
    constexpr it& operator--() { return (--i, *this); }

    constexpr it operator++(int) { return {p, i++}; }
    constexpr it operator--(int) { return {p, i--}; }

    constexpr it& operator+=(int n) { return (i += n, *this); }
    constexpr it& operator-=(int n) { return (i -= n, *this); }

    constexpr friend it operator+(const it& a, int n) { return {a.p, a.i + n}; }
    constexpr friend it operator+(int n, const it& a) { return {a.p, n + a.i}; }
    constexpr friend it operator-(const it& a, int n) { return {a.p, a.i - n}; }

    constexpr bool operator==(const it& b)  const { return i == b.i; }
    constexpr auto operator<=>(const it& b) const { return i <=> b.i; }

    constexpr int operator-(const it& b) const { return i - b.i; }
  };

  using iterator = it<cvector_impl>;
  using const_iterator = it<const cvector_impl>;

  // Iterators.
  constexpr const_iterator begin() const { return {this, 0}; }
  constexpr       iterator begin()       { return {this, 0}; }
  constexpr const_iterator   end() const { return {this, this->n}; }
  constexpr       iterator   end()       { return {this, this->n}; }

  constexpr auto rbegin() const { return std::reverse_iterator(end()); }
  constexpr auto rbegin()       { return std::reverse_iterator(end()); }
  constexpr auto   rend() const { return std::reverse_iterator(begin()); }
  constexpr auto   rend()       { return std::reverse_iterator(begin()); }

  // Helpers to manage the lifetime of storage elements. In truth, I'd like
  // these to be members of the union itself, however if they're regular
  // members then clang complains when I use them because it has no active
  // union, and if they're friends then gcc crashes
  // (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85576). So here they are.
 private:
  template <typename... Ts>
  requires(std::constructible_from<T, Ts...>)
  constexpr static T& construct(storage_type& s, Ts&&... ts)
  {
    // clang is correct, but gcc isn't ready at the time I'm doing this
#ifdef __clang__
    return *std::construct_at(std::addressof(s.t), std::forward<Ts>(ts)...);
#else
    return s.t = T(std::forward<Ts>(ts)...);
#endif
  }

  constexpr static void destroy(storage_type& s) {
    std::destroy_at(std::addressof(s.t));
  }

  // Protected handlers are called by our subclasses as needed in order to
  // clear, copy, and/or move the array properly. Copying and moving require us
  // to properly manage the active member of the storage array.
 protected:
  constexpr void on_dtor() {
    clear();
  }

  constexpr void on_copy(const cvector_impl& b) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i] = b.storage[i];
    }
    for (; i < b.n; ++i) {
      construct(storage[i], b.storage[i].t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = b.n;
  }

  constexpr void on_move(cvector_impl&& b) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i] = std::move(b.storage[i]);
    }
    for (; i < b.n; ++i) {
      construct(storage[i], std::move(b.storage[i]).t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = std::exchange(b.n, 0);
  }
};

// The externally-visible cvector class.
//
// This class provides the move and copy assignment operators in a
// trivially-aware way, and then forwards into the P0848 mess in order to deal
// with clang's missing functionality. It passes the cvector_impl template as
// the base of that inheritance hierarchy.
//
// It also provides the only non-default constructor which is a variadic
// constructor that initializes a subset of the values. We provide CTAD for
// that variadic constructor which can infer the type and size of the cvector
// for those use cases.
template <typename T, int N>
struct cvector : P0848::move<cvector_impl<T, N>>
{
  constexpr cvector() = default;

  template <std::convertible_to<T>... Ts>
  constexpr cvector(Ts&&... ts) { static_assert(sizeof...(ts) <= N);
    (this->emplace_back(std::forward<Ts>(ts)), ...);
  }

  template <std::convertible_to<T>... Ts>
  constexpr cvector(std::in_place_t, Ts&&... ts)
      : cvector(std::forward<Ts>(ts)...)
  {}

  template <std::convertible_to<T>... Ts>
  constexpr cvector(std::in_place_type_t<T>, Ts&&... ts)
      : cvector(std::forward<Ts>(ts)...)
  {}

  // need these because otherwise we lose trivial traits
  constexpr cvector(const cvector&) = default;
  constexpr cvector(cvector&&) = default;

  // clang handles this properly
  constexpr static bool copyable = std::is_trivially_copy_assignable_v<T>;
  constexpr cvector& operator=(const cvector&)   requires( copyable) = default;
  constexpr cvector& operator=(const cvector& v) requires(!copyable) {
    this->on_copy(v);
    return *this;
  }

  // clang handles this properly
  constexpr static bool movable = std::is_trivially_move_assignable_v<T>;
  constexpr cvector& operator=(cvector&&)   requires( movable) = default;
  constexpr cvector& operator=(cvector&& v) requires(!movable) {
    this->on_move(std::move(v));
    return *this;
  }
};

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_t, T&&, Ts&&...) -> cvector<T, 1 + sizeof...(Ts)>;

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_type_t<T>, Ts&&...) -> cvector<T, sizeof...(Ts)>;
}

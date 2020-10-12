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

#include <algorithm>
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
          int = std::is_trivially_constructible_v<T>,
          int = std::is_trivially_destructible_v<T>,
          int = std::is_trivially_copy_constructible_v<T>,
          int = std::is_trivially_move_constructible_v<T>>
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
// constructor, based on the passed defaulted flag. None of the operators
// directly interact with the underlying storage, we assume that the hosted
// class deals with everything explicitly.
#define P0848_MAKE_CTOR_1(type) constexpr type() = default
#define P0848_MAKE_CTOR_0(type) constexpr type() {}
#define P0848_MAKE_CTOR(defaulted, type)        \
  P0848_MAKE_CTOR_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_DTOR_1(type) constexpr ~type() = default
#define P0848_MAKE_DTOR_0(type) constexpr ~type() {}
#define P0848_MAKE_DTOR(defaulted, type)        \
  P0848_MAKE_DTOR_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_COPY_CTOR_1(type) constexpr type(const type&) = default
#define P0848_MAKE_COPY_CTOR_0(type) constexpr type(const type& b) {}
#define P0848_MAKE_COPY_CTOR(defaulted, type)           \
  P0848_MAKE_COPY_CTOR_##defaulted(P0848_WRAP(type))

#define P0848_MAKE_MOVE_CTOR_1(type) constexpr type(type&&) = default
#define P0848_MAKE_MOVE_CTOR_0(type) constexpr type(type&&) {}
#define P0848_MAKE_MOVE_CTOR(defaulted, type)           \
  P0848_MAKE_MOVE_CTOR_##defaulted(P0848_WRAP(type))

// Clang 10.0.1 on my Debian testing system wants monostate.
#ifdef __clang__
#define CLANG_NEEDS_MONOSTATE(id) struct {} id = {}
#else
#define CLANG_NEEDS_MONOSTATE(id)
#endif

// Expands into one of the storage union types.
#define P0848_MAKE(type, triv_ctor, triv_dtor, triv_cctor, triv_mctor)  \
  template <typename T>                                                 \
  union type<T, triv_ctor, triv_dtor, triv_cctor, triv_mctor>           \
  {                                                                     \
    T t;                                                                \
    CLANG_NEEDS_MONOSTATE(_);                                           \
    P0848_MAKE_CTOR(triv_ctor,       P0848_WRAP(type));                 \
    P0848_MAKE_DTOR(triv_dtor,       P0848_WRAP(type));                 \
    P0848_MAKE_COPY_CTOR(triv_cctor, P0848_WRAP(type));                 \
    P0848_MAKE_MOVE_CTOR(triv_mctor, P0848_WRAP(type));                 \
                                                                        \
    constexpr static bool triv_copy = std::is_trivially_copy_assignable_v<T>; \
    constexpr type& operator=(const type&) requires(triv_copy) = default; \
    constexpr type& operator=(const type&) requires(!triv_copy) {}      \
                                                                        \
    constexpr static bool triv_move = std::is_trivially_move_assignable_v<T>; \
    constexpr type& operator=(type&&) requires(triv_move) = default;    \
    constexpr type& operator=(type&&) requires(!triv_move) {}           \
  }

// Expand the 16 storage union configurations.
P0848_MAKE(storage, 0, 0, 0, 0);
P0848_MAKE(storage, 0, 0, 0, 1);
P0848_MAKE(storage, 0, 0, 1, 0);
P0848_MAKE(storage, 0, 0, 1, 1);
P0848_MAKE(storage, 0, 1, 0, 0);
P0848_MAKE(storage, 0, 1, 0, 1);
P0848_MAKE(storage, 0, 1, 1, 0);
P0848_MAKE(storage, 0, 1, 1, 1);
P0848_MAKE(storage, 1, 0, 0, 0);
P0848_MAKE(storage, 1, 0, 0, 1);
P0848_MAKE(storage, 1, 0, 1, 0);
P0848_MAKE(storage, 1, 0, 1, 1);
P0848_MAKE(storage, 1, 1, 0, 0);
P0848_MAKE(storage, 1, 1, 0, 1);
P0848_MAKE(storage, 1, 1, 1, 0);
P0848_MAKE(storage, 1, 1, 1, 1);

#undef P0848_MAKE
#undef CLANG_NEEDS_MONOSTATE
#undef P0848_MAKE_MOVE
#undef P0848_MAKE_COPY
#undef P0848_MAKE_DTOR
#undef P0848_MAKE_CTOR
#undef P0848_WRAP

template <typename Base,
          bool = std::is_trivially_destructible_v<typename Base::value_type>>
struct dtor : Base {
  using Base::Base;
};

template <typename Base>
struct dtor<Base, false> : Base
{
  using Base::Base;

  constexpr ~dtor() {
    this->on_dtor();
  }

  constexpr dtor()                       = default;
  constexpr dtor(const dtor&)            = default;
  constexpr dtor(dtor&&)                 = default;
  constexpr dtor& operator=(const dtor&) = default;
  constexpr dtor& operator=(dtor&&)      = default;
};

template <typename Base,
          bool = std::is_trivially_copy_constructible_v<typename Base::value_type>>
struct copy_ctor : dtor<Base> {
  using dtor<Base>::dtor;
};

template <typename Base>
struct copy_ctor<Base, false> : dtor<Base>
{
  using dtor<Base>::dtor;

  constexpr copy_ctor(const copy_ctor& d) {
    this->on_copy_ctor(d);
  }

  constexpr ~copy_ctor()                            = default;
  constexpr  copy_ctor()                            = default;
  constexpr  copy_ctor(copy_ctor&&)                 = default;
  constexpr  copy_ctor& operator=(const copy_ctor&) = default;
  constexpr  copy_ctor& operator=(copy_ctor&&)      = default;
};

template <typename Base,
          bool = std::is_trivially_move_constructible_v<typename Base::value_type>>
struct move_ctor : copy_ctor<Base> {
  using copy_ctor<Base>::copy_ctor;
};

template <typename Base>
struct move_ctor<Base, false> : copy_ctor<Base>
{
  using copy_ctor<Base>::copy_ctor;

  constexpr move_ctor(move_ctor&& d) {
    this->on_move_ctor(std::move(d));
  }

  constexpr ~move_ctor()                            = default;
  constexpr  move_ctor()                            = default;
  constexpr  move_ctor(const move_ctor&)            = default;
  constexpr  move_ctor& operator=(const move_ctor&) = default;
  constexpr  move_ctor& operator=(move_ctor&&)      = default;
};

template <typename Base>
struct ops : move_ctor<Base> {
  using move_ctor<Base>::move_ctor;
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

  constexpr cvector_impl()                    = default;
  constexpr cvector_impl(const cvector_impl&) = default;
  constexpr cvector_impl(cvector_impl&&)      = default;

  constexpr cvector_impl(int n) : n(n) { assert(0 <= n && n <= N);
    static_assert(std::is_default_constructible_v<T>);
    for (int i = 0; i < n; ++i) {
      construct(storage[i]);
    }
  }

  template <std::convertible_to<T>... Ts>
  constexpr cvector_impl(std::in_place_t, Ts&&... ts) {
    static_assert(sizeof...(ts) <= N);
    (emplace_back(std::forward<Ts>(ts)), ...);
  }

  template <std::convertible_to<T>... Ts>
  constexpr cvector_impl(std::in_place_type_t<T>, Ts&&... ts)
      : cvector_impl(std::in_place, std::forward<Ts>(ts)...)
  {}

  // If the underlying type is trivially copy assignable, then the default copy
  // operator is fine, otherwise we manually manage the underlying storage `t`.
  constexpr static bool triv_copy = std::is_trivially_copy_assignable_v<T>;
  constexpr cvector_impl& operator=(const cvector_impl&) requires(triv_copy) = default;
  constexpr cvector_impl& operator=(const cvector_impl& b) requires(!triv_copy) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i].t = b.storage[i].t;
    }
    for (; i < b.n; ++i) {
      construct(storage[i], b.storage[i].t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = b.n;
    return *this;
  }

  // If the underlying type is trivially move assignable, then the default move
  // operator is fine, otherwise we manually manage the underlying storage `t`.
  constexpr static bool triv_move = std::is_trivially_move_assignable_v<T>;
  constexpr cvector_impl& operator=(cvector_impl&&) requires(triv_move) = default;
  constexpr cvector_impl& operator=(cvector_impl&& b) requires(!triv_move) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i].t = std::move(b.storage[i]).t;
    }
    for (; i < b.n; ++i) {
      construct(storage[i], std::move(b.storage[i]).t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = std::exchange(b.n, 0);
    return *this;
  }

  // Underlying data (not constexpr because of the reinterpret cast)
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
  template <typename... Ts>
  constexpr T& emplace_back(Ts&&... ts) { assert(n < N);
    static_assert(std::is_constructible_v<T, Ts...>);
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
  // (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85576).
 private:
  template <typename... Ts>
  constexpr static T& construct(storage_type& s, Ts&&... ts)
  {
    static_assert(std::is_constructible_v<T, Ts...>);
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
  // clear, copy construct, and/or move construct the array properly.
 protected:
  constexpr void on_dtor() {
    clear();
  }

  constexpr void on_copy_ctor(const cvector_impl& b) {
    for (n = 0; n < b.n; ++n) {
      construct(storage[n], b.storage[n].t);
    }
  }

  constexpr void on_move_ctor(cvector_impl&& b) {
    for (n = 0; n < b.n; ++n) {
      construct(storage[n], std::move(b.storage[n]).t);
    }
    b.n = 0;
  }
};

// The externally-visible cvector class.
template <typename T, int N>
struct cvector : P0848::ops<cvector_impl<T, N>>
{
  using P0848::ops<cvector_impl<T, N>>::ops;
};

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_t, T&&, Ts&&...) -> cvector<T, 1 + sizeof...(Ts)>;

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_type_t<T>, Ts&&...) -> cvector<T, sizeof...(Ts)>;
}

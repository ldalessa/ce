// BSD 3-Clause License
//
// Copyright (c) 2020, Trustees of Indiana University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ----------------------------------------------------------------------------
// Utilities to workaround missing support for P0848 in clang.
// ----------------------------------------------------------------------------
#pragma once

#include <type_traits>
#include <iterator>
#include <utility>

namespace ce::P0848
{
// Destructor policy injects an explicit destructor if necessary.
template <typename Base, bool = std::is_trivially_destructible_v<typename Base::value_type>>
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

// Copy constructor policy forwards to the base class on_copy_ctor if
// necessary.
template <typename Base, bool = std::is_trivially_copy_constructible_v<typename Base::value_type>>
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

// Move constructor policy forwards to the base class on_move_ctor if
// necessary.
template <typename Base, bool = std::is_trivially_move_constructible_v<typename Base::value_type>>
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

// Injects P0848 operations into Base.
template <typename Base>
struct ops : move_ctor<Base> {
  using move_ctor<Base>::move_ctor;
};

// The storage union is an integral part of our vector implementations. It wraps
// a single instance of a `T` and allows us to allocate an array of them without
// actually initializing any of them, which is what we want when `T` is not
// `trivially_constructible`.
//
// We need 2^4 different versions of the union depending on the underlying
// traits of the `T`. Unions do not participate in inheritance, so instead we
// define the 16 partial specialization via set of P0848_MACRO expansions.
template <typename T,
          int = std::is_trivially_constructible_v<T>,
          int = std::is_trivially_destructible_v<T>,
          int = std::is_trivially_copy_constructible_v<T>,
          int = std::is_trivially_move_constructible_v<T>>
union storage;
// {
//   T t;
//   using stored_type = T;
// }

template <typename>
struct is_storage_trait : std::false_type {};

template <typename T, int A, int B, int C, int D>
struct is_storage_trait<storage<T, A, B, C, D>> : std::true_type {};

template <typename T>
concept is_storage = is_storage_trait<T>::value;

// Wrappers for constructing and destructing the stored element.
template <is_storage U, typename... Ts>
constexpr static typename U::stored_type& construct(U& u, Ts&&... ts)
{
  static_assert(std::is_constructible_v<typename U::stored_type, Ts...>);
  return *std::construct_at(std::addressof(u.t), std::forward<Ts>(ts)...);
}

constexpr static auto& construct(is_storage auto& u, is_storage auto const& v) {
  return construct(u, v.t);
}

constexpr static auto& construct(is_storage auto& u, is_storage auto&& v) {
  return construct(u, std::move(v).t);
}

constexpr static void destroy(is_storage auto& u) {
  std::destroy_at(std::addressof(u.t));
}

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

// Expands into one of the storage union types.
#define P0848_MAKE(type, triv_ctor, triv_dtor, triv_cctor, triv_mctor)  \
  template <typename T>                                                 \
  union type<T, triv_ctor, triv_dtor, triv_cctor, triv_mctor>           \
  {                                                                     \
    using stored_type = T;                                              \
    struct {} _monotype = {};                                           \
    T t;                                                                \
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
#undef P0848_MAKE_MOVE
#undef P0848_MAKE_COPY
#undef P0848_MAKE_DTOR
#undef P0848_MAKE_CTOR
#undef P0848_WRAP

// A random access iterator template class for a storage array.
template <typename U>
struct storage_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using      element_type = typename U::stored_type;

  U* ptr = nullptr;

  constexpr auto& operator[](int n) const { return ptr[n].t; }
  constexpr auto& operator*()       const { return (*ptr).t; }
  constexpr auto* operator->()      const { return std::addressof((*ptr).t); }

  constexpr storage_iterator& operator++() { return (++ptr, *this); }
  constexpr storage_iterator& operator--() { return (--ptr, *this); }

  constexpr storage_iterator operator++(int) { return {ptr++}; }
  constexpr storage_iterator operator--(int) { return {ptr--}; }

  constexpr storage_iterator& operator+=(int n) { return (ptr += n, *this); }
  constexpr storage_iterator& operator-=(int n) { return (ptr -= n, *this); }

  constexpr storage_iterator operator+(int n) const { return {ptr + n}; }
  constexpr storage_iterator operator-(int n) const { return {ptr - n}; }
  constexpr friend storage_iterator operator+(int n, const storage_iterator& a)
  {
    return {n + a.ptr};
  }

  constexpr bool operator==(const storage_iterator& b)  const {
    return ptr == b.ptr;
  }

  constexpr auto operator<=>(const storage_iterator& b) const {
    return ptr <=> b.ptr;
  }

  constexpr int operator-(const storage_iterator& b) const {
    return int(ptr - b.ptr);
  }
};

template <typename T>
using storage_type = storage<T>;

template <typename T>
using storage_iterator_type = storage_iterator<T>;
}

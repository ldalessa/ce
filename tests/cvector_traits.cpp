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

#include "ce/cvector.hpp"
#include "common.hpp"
#include "Foo.hpp"

using namespace ce::tests;

template <typename T>
constexpr static bool check() {
  using V = ce::cvector<T, 8>;
  CE_CHECK(std::is_trivially_destructible_v<T> == std::is_trivially_destructible_v<V>);
  CE_CHECK(std::is_trivially_copy_constructible_v<T> == std::is_trivially_copy_constructible_v<V>);
  CE_CHECK(std::is_trivially_move_constructible_v<T> == std::is_trivially_move_constructible_v<V>);
  CE_CHECK(std::is_trivially_copy_assignable_v<T> == std::is_trivially_copy_assignable_v<V>);
  CE_CHECK(std::is_trivially_move_assignable_v<T> == std::is_trivially_move_assignable_v<V>);

  constexpr V decl;
  constexpr V ctor{};
  unused(decl, ctor);
  return true;
};

#define CE_EXPAND(...) static_assert(check<Foo<__VA_ARGS__>>())
#include "common.decl"
#undef CE_EXPAND

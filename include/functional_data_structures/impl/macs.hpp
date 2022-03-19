#ifndef FUNCTIONAL_DATA_STRUCTURES_IMPL_MACS_INCLUDE_GUARD
#define FUNCTIONAL_DATA_STRUCTURES_IMPL_MACS_INCLUDE_GUARD

#include "hedley.h"

#define RET(...)                                                          \
  { return __VA_ARGS__; }
#define NOEX(...) noexcept(noexcept(__VA_ARGS__)) RET(__VA_ARGS__)
#define NOEX_DECL(...)                                                    \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) RET(__VA_ARGS__)
#define NOEXDECL(...) NOEX_DECL(__VA_ARGS__)

#define FWD(...) (static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__))

#include <concepts>
namespace functional_data_structures { namespace impl { namespace macs {
  constexpr bool and_(std::convertible_to<bool> auto... args)
      RET((static_cast<bool>(args) && ...))

  template<class>
  constexpr auto delay_id(auto x) RET(x)

  struct errors_on_use {
    template<class T>
    operator T() {
      static_assert(delay_id<T>(false),
                    "FN called with insufficient arguments");
    }
  };
}}} // namespace functional_data_structures::impl::macs

#define GENSYM(name)                                                      \
  HEDLEY_CONCAT3(functional_data_structures_gensym, __COUNTER__, name)

#define FN_(type, ...)                                                    \
  <class HEDLEY_CONCAT(type, 0) =                                         \
       ::functional_data_structures::impl::macs::errors_on_use,           \
   class HEDLEY_CONCAT(type, 1) =                                         \
       ::functional_data_structures::impl::macs::errors_on_use,           \
   class HEDLEY_CONCAT(type, 2) =                                         \
       ::functional_data_structures::impl::macs::errors_on_use,           \
   class HEDLEY_CONCAT(type, 3) =                                         \
       ::functional_data_structures::impl::macs::errors_on_use>(          \
      [[maybe_unused]] HEDLEY_CONCAT(type, 0) _0 = {},                    \
      [[maybe_unused]] HEDLEY_CONCAT(type, 1) _1 = {},                    \
      [[maybe_unused]] HEDLEY_CONCAT(type, 2) _2 = {},                    \
      [[maybe_unused]] HEDLEY_CONCAT(type, 3) _3 = {}) {                  \
    auto& _ = _0;                                                         \
    return __VA_ARGS__;                                                   \
  }

#define FN(...) FN_(GENSYM(fn_type), __VA_ARGS__)

#define LIFT_(args, ...)                                                  \
  [](auto&&... args) NOEX_DECL((__VA_ARGS__)(FWD(args)...))
#define LIFT(...) LIFT_(GENSYM(lift_args), __VA_ARGS__)

#define IMPLICIT(...)                                                     \
  explicit(!::functional_data_structures::impl::macs::and_(__VA_ARGS__))

#ifndef ASSERT_NOEXCEPT
#  define ASSERT_NOEXCEPT true
#endif
#define ANOEXCEPT(...)                                                    \
  noexcept(ASSERT_NOEXCEPT                                                \
           && ::functional_data_structures::impl::macs::and_(__VA_ARGS__))

#define ASSERT(...)

#include "pass.hpp"
#define READER(sym)                                                       \
  constexpr ::functional_data_structures::impl::pass::ReadOut<            \
      decltype(HEDLEY_CONCAT(sym, _))>                                    \
      sym() NOEX(HEDLEY_CONCAT(sym, _))

#include <utility>
#define WRITER_(var, sym)                                                 \
  constexpr auto& sym(decltype(HEDLEY_CONCAT(sym, _)) var) {              \
    HEDLEY_CONCAT(sym, _) = std::move(var);                               \
    return *this;                                                         \
  }
#define WRITER(sym)   WRITER_(GENSYM(var), sym)
#define ACCESSOR(sym) READER(sym) WRITER(sym)
#endif // FUNCTIONAL_DATA_STRUCTURES_IMPL_MACS_INCLUDE_GUARD

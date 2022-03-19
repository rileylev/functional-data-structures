#ifndef functional_data_structures_impl_pass_hpp_INCLUDE_GUARD
#define functional_data_structures_impl_pass_hpp_INCLUDE_GUARD

#include <type_traits>

namespace functional_data_structures { namespace impl { namespace pass {

  template<class T>
  constexpr bool easy_to_copy =
      std::is_trivially_copyable_v<T> && (sizeof(T) <= sizeof(void*) * 2);

  template<class T>
  using ReadIn = std::conditional_t<easy_to_copy<T>, T const, T const&>;

  template<class T>
  using ReadOut = std::conditional_t<easy_to_copy<T>, T, T const&>;
}}} // namespace functional_data_structures::impl::pass

#endif

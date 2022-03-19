#ifndef functional_data_structures_lowlevel_data_pointer_through_uninit_hpp_INCLUDE_GUARD
#define functional_data_structures_lowlevel_data_pointer_through_uninit_hpp_INCLUDE_GUARD

#include "../impl/macs.hpp"
#include "uninit.hpp"

#include <iterator>
#include <concepts>

namespace functional_data_structures { namespace lowlevel_data {

  enum { cns = false, mut = true };

  template<class T, bool is_mutable, template<class> class uninit = uninit>
  struct ptr_through_uninit : std::contiguous_iterator_tag {
   public:
    using base_t =
        std::conditional_t<is_mutable, uninit<T>*, uninit<T> const*>;
    base_t base;

    using value_type        = T;
    using reference         = std::conditional_t<is_mutable, T&, T const&>;
    using pointer           = std::conditional_t<is_mutable, T*, T const*>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::contiguous_iterator_tag;

    constexpr ptr_through_uninit()                            = default;
    constexpr ptr_through_uninit(ptr_through_uninit const& x) = default;
    constexpr IMPLICIT()
        ptr_through_uninit(ptr_through_uninit<T, mut> x) noexcept
        : base{x.base} {}
    constexpr ptr_through_uninit(uninit<T> const* ptr) noexcept
        requires(!is_mutable)
        : base{ptr} {}
    constexpr ptr_through_uninit(uninit<T>* ptr) noexcept : base{ptr} {}

    constexpr reference  operator*() const noexcept RET(**base)
    constexpr value_type operator->() const noexcept RET(&(*(*this)))

    constexpr friend difference_type
        operator-(ptr_through_uninit x, ptr_through_uninit y) noexcept
        RET(x.base - y.base)

    constexpr ptr_through_uninit& operator+=(difference_type N) noexcept {
      base += N;
      return *this;
    }
    constexpr ptr_through_uninit& operator-=(difference_type N) noexcept {
      base -= N;
      return *this;
    }

    constexpr friend ptr_through_uninit
        operator+(ptr_through_uninit iter, difference_type N)
            NOEX(iter += N)
    constexpr friend ptr_through_uninit
        operator-(ptr_through_uninit iter, difference_type N)
            NOEX(iter -= N)

    constexpr reference operator[](difference_type N) const
        NOEX(*(*this + N))

    constexpr bool operator==(ptr_through_uninit) const = default;

    constexpr friend auto
        operator<=>(ptr_through_uninit x, ptr_through_uninit y) noexcept
        RET(x.base <=> y.base);
  };
}} // namespace functional_data_structures::lowlevel_data
#endif

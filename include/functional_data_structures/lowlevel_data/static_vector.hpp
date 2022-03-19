#ifndef functional_data_structures_low_level_data_static_vector_hpp_INCLUDE_GUARD
#define functional_data_structures_low_level_data_static_vector_hpp_INCLUDE_GUARD

#include "pointer_through_uninit.hpp"

#include <tuple>
#include <array>

namespace functional_data_structures { namespace lowlevel_data {

  namespace impl {
    template<class Pred, class... Args>
    struct find_if_t_ {
      constexpr static std::size_t idx = [] {
        std::size_t n{};
        ((Pred{}.template operator()<Args>() || (++n, false)) || ...);
        return n;
      }();

      using type = std::tuple_element_t<idx, std::tuple<Args...>>;
    };
    template<class Pred, class... Args>
    using find_if_t = typename find_if_t_<Pred, Args...>::type;
  } // namespace impl

  template<class T, std::size_t Capacity>
  class static_vector {
    using can_hold_capacity = decltype([]<class X>() RET(
        std::cmp_less_equal(Capacity, std::numeric_limits<X>::max)));

   public:
    using size_type =
        impl::find_if_t<can_hold_capacity,
                        unsigned char,
                        unsigned short,
                        unsigned int,
                        unsigned long,
                        unsigned long long>;
    using ssize_type =
        impl::find_if_t<can_hold_capacity,
                        signed char,
                        signed short,
                        signed int,
                        signed long,
                        signed long long>;

    using value_type      = T;
    using difference_type = ssize_type; // is this correct?
    using reference       = value_type&;
    using const_reference = value_type const&;
    using pointer         = value_type*;
    using const_pointer   = value_type const*;
    using iterator        = ptr_through_uninit<T, true, uninit>;
    using const_iterator  = ptr_through_uninit<T, false, uninit>;

   private:
    size_type                       size_{};
    std::array<uninit<T>, Capacity> storage_;

    constexpr void destroy_members() {
      if constexpr(std::is_trivially_destructible_v<T>) return;
      else
        for(size_type i = size_ - 1; i != 0; --i) storage_[i].destroy();
    }

   public:
    READER(size)
    constexpr ssize_type  ssize() const NOEX(ssize_type{size()})
    constexpr std::size_t capacity() const NOEX(Capacity)
    constexpr bool        full() const NOEX(size() == capacity())

    // clang-format off
    constexpr const_iterator cbegin() const NOEX(const_iterator{&storage_[0]});
    constexpr       iterator  begin()       NOEX(      iterator{&storage_[0]});

    constexpr auto cend() const NOEXDECL(cbegin() + size())
    constexpr auto end()        NOEXDECL( begin() + size())
    // clang-format on
    constexpr auto begin() const NOEXDECL(cbegin());
    constexpr auto end() const NOEXDECL(cend())

    constexpr T& operator[](std::size_t n) {
      ASSERT(n < size_);
      return *(storage_[n]);
    }
    constexpr T const& operator[](std::size_t n) const {
      ASSERT(n < size_);
      return *(storage_[n]);
    }

    constexpr void lazy_insert_idx(size_type idx, auto thunk) {
      ASSERT(!full());
      ASSERT(idx <= size());

      for(size_type i = size(); i != idx; --i)
        storage_[i] = dmove(storage_[i - 1]);
      storage_[idx].construct(thunk());
      ++size_;
    }

    constexpr void erase_idx(size_type idx) {
      storage_[idx].destroy();
      auto const sz = size();
      while(idx < sz) storage_[idx].construct(dmove(storage_[idx + 1]));
    }

    constexpr void clear() {
      destroy_members();
      size_ = 0;
    }

    // special member functions
    constexpr static_vector() = default;

    constexpr static_vector(static_vector const&) requires(
        std::is_trivially_copy_constructible_v<T>) = default;
    constexpr static_vector(static_vector const& other)
        : size_{other.size()} {
      for(size_type i = 0; i != other.size(); ++i)
        storage_[i].construct(other[i]);
    }

    constexpr static_vector& operator=(static_vector const&) requires(
        std::is_trivially_copy_assignable_v<T>) = default;
    constexpr static_vector& operator=(static_vector const& other) {
      destroy_members();
      size_ = other.size();
      for(size_type i = 0; i != size_; ++i)
        storage_[i].construct(*(other.storage_[i]));
    }

    constexpr static_vector(static_vector&&) requires(
        std::is_trivially_move_constructible_v<T>) = default;
    constexpr static_vector(static_vector&& other) : size_{other.size()} {
      for(size_type i = 0; i != size_; ++i)
        storage_[i].construct(dmove(other.storage_.storage[i]));
      other.size_ = 0;
    }

    constexpr static_vector& operator=(static_vector&&) requires(
        std::is_trivially_move_assignable_v<T>) = default;
    //
    constexpr static_vector& operator=(static_vector&& other) {
      destroy_members();
      size_ = std::exchange(other.size_, 0);
      for(size_type i = size_ - 1; i != 0; --i)
        storage_[i].construct(dmove(other.storage_[i]));
    }

    constexpr ~static_vector() requires std::is_trivially_destructible_v<T>
    = default;
    constexpr ~static_vector() { destroy_members(); }
  };
}} // namespace functional_data_structures::lowlevel_data
#endif

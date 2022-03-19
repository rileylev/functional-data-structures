#ifndef FUNCTIONAL_DATA_STRUCTURES_LOWLEVEL_DATA_UNINIT_HPP_INCLUDE_GUARD
#define FUNCTIONAL_DATA_STRUCTURES_LOWLEVEL_DATA_UNINIT_HPP_INCLUDE_GUARD

#include "../impl/macs.hpp"

#include <utility>
#include <memory>
#include <optional>

namespace functional_data_structures { namespace lowlevel_data {

  // disables lifetime management
  template<class T>
  union uninit {
   private:
    T    init;

   public:
    constexpr T&       operator*() NOEX(&init)
    constexpr T const& operator*() const NOEX(&init)

    constexpr auto operator->() const NOEX(std::addressof(this->init))
    constexpr auto operator->() NOEX(std::addressof(this->init))

    constexpr void construct(auto&&... args)
        NOEX((void)std::construct_at(&init, FWD(args)...))

    constexpr void destroy() NOEX(std::destroy_at(&init))
  };

  template<class T>
  struct debug_uninit : std::optional<T> {
    constexpr void construct(auto&&... args) {
      ASSERT(!this->has_value())
      emplace(FWD(args)...);
    }

    constexpr void destroy() {
      ASSERT(this->has_value());
      this->reset();
    }
  };

  template<class T, template<class> class uninit>
  constexpr T dmove(uninit<T>& x)
      ANOEXCEPT(noexcept(std::move(*x), x.destroy())) {
    T ret = std::move(*x);
    x.destroy();
    return ret;
  }
}} // namespace functional_data_structures::lowlevel_data

#endif

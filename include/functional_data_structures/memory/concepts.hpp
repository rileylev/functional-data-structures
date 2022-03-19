#ifndef functional_data_structures_memory_concepts_hpp_INCLUDE_GUARD
#define functional_data_structures_memory_concepts_hpp_INCLUDE_GUARD

#include "../impl/macs.hpp"

namespace functional_data_structures { namespace memory {

  namespace impl {
    template<class Mem, class T>
    concept can_store1 =
        requires(Mem m, typename Mem::handles::template handle<T> h) {
      m.template make<T>;
      { m[h] } -> std::convertible_to<T>;
      { m.is_null(h) } -> std::same_as<bool>;
      {
        m.template null<T>()
        } -> std::same_as<typename Mem::handles::template handle<T>>;
    };

    template<class Mem, class T>
    concept mutable_can_store1 = requires(
        Mem                                               m,
        typename Mem::handles::template mutable_handle<T> h) {
      m.template make_mutable<T>;
      { m[h] } -> std::same_as<T&>;
      {
        m.freeze(h)
        } -> std::same_as<typename Mem::handles::template handle<T>>;
      { m.is_null(h) } -> std::same_as<bool>;
    };

    template<class Mem, class T>
    concept can_type_erase1 = requires(
        Mem                                       m,
        typename Mem::handles::template handle<T> h,
        typename Mem::handles::erased             e) {
      { m.erase(h) } -> std::same_as<typename Mem::handles::erased>;
      {
        m.template cast<T>(e)
        } -> std::same_as<typename Mem::handles::template handle<T>>;
    };

    template<class Mem, class T, class... Ts>
    concept can_vary1 = requires(
        Mem                                            m,
        typename Mem::handles::template variant<Ts...> v,
        typename Mem::handles::template handle<T>      h) {
      {
        m.template vary<Ts...>(h)
        } -> std::same_as<typename Mem::handles::template variant<Ts...>>;
      {
        m.template get<T>(v)
        } -> std::same_as<typename Mem::handles::template variant<Ts...>>;
    };
  } // namespace impl

  template<class Mem, class... Ts>
  concept can_store = (impl::can_store1<Mem, Ts> && ...);
  template<class Mem, class... Ts>
  concept mutable_can_store = (impl::mutable_can_store1<Mem, Ts> && ...);
  template<class Mem, class... Ts>
  concept can_type_erase = (impl::can_type_erase1<Mem, Ts> && ...);
  template<class Mem, class... Ts>
  concept can_vary = (impl::can_vary1<Mem, Ts, Ts...> && ...);

}}     // namespace functional_data_structures::memory
#endif //

#ifndef functional_data_structures_memory_shared_ptr_mem_hpp_INCLUDE_GUARD
#define functional_data_structures_memory_shared_ptr_mem_hpp_INCLUDE_GUARD

#include "../impl/macs.hpp"
#include "../impl/pass.hpp"

#include <memory>
#include <variant>

namespace functional_data_structures { namespace memory {
  struct shared_ptr_handles {
    template<class T>
    using handle = std::shared_ptr<T const>;

    template<class T>
    using mutable_handle = std::shared_ptr<T>;

    // should I change this to a handle<void>?
    using erased = std::shared_ptr<void const>;

    template<class... Ts>
    using variant = std::variant<std::shared_ptr<Ts>...>;
  };

  using namespace functional_data_structures::impl::pass;
  struct shared_ptr_mem {
    using handles = shared_ptr_handles;
    template<class T>
    handles::handle<T> make(auto&&... args)
        NOEX(make_shared<T const>(FWD(args)...))

    template<class T>
    bool is_null(handles::handle<T> x) NOEX(x == nullptr)
    template<class T>
    bool is_null(handles::mutable_handle<T> x) NOEX(x == nullptr)

    template<class T>
    handles::handle<T> null() NOEX(std::shared_ptr<T>{nullptr})

    template<class T>
    handles::mutable_handle<T> make_mutable(auto&&... args)
        NOEX(make_shared<T>(FWD(args)...))

    template<class T>
    handles::handle<T> freeze(handles::mutable_handle<T> h) const
        NOEX(std::static_pointer_cast<T const>(h))

    template<class T>
    handles::erased erase(handles::handle<T> h) const
        NOEX(std::static_pointer_cast<void const>(h))

    template<class T>
    handles::handle<T> cast(handles::erased e) const
        NOEX(std::static_pointer_cast<T const>(e))

    template<class T, class... Ts>
    handles::variant<Ts...> vary(handles::handle<T> h) const
        NOEX(std::variant<Ts...>{h})

    template<class T, class... Ts>
    handles::handle<T> get(handles::variant<Ts...> v) const
        NOEX(std::get<T>(v))

    template<class T>
    ReadOut<T> operator[](ReadIn<handles::handle<T>> h) const NOEX(*h)
    template<class T>
    T& operator[](ReadIn<handles::mutable_handle<T>> h) NOEX(*h)
  };
}} // namespace functional_data_structures::memory
#endif

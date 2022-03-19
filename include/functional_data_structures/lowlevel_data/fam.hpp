#ifndef functional_data_structures_00_lowlevel_data_00_fam_hpp_INCLUDE_GUARD
#define functional_data_structures_00_lowlevel_data_00_fam_hpp_INCLUDE_GUARD

#include "../impl/macs.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <type_traits>

// approaches to create FAMs
// - use vendor extensions
// - use array of length 1 and write off the end (technically UB)
// - use a properly aligned dummy byte and treat that as an array (I think
// this is not UB because the array's implicit lifetime starts when you
// create the byte array for storage )
// - cast *this to byte*; find end + alignment

// TODO: provide a more unit-testable way to select these (probably
// template args) brainstorm: how to implement fams? compiler extensions:
//   -  T[]
//   -  T[0]
//   - UB T[1]
//   by hand?
#define FUNCTIONAL_DATA_STRUCTURES_FLEX
//#define FUNCTIONAL_DATA_STRUCTURES_FLEX 0
//#define FUNCTIONAL_DATA_STRUCTURES_FLEX 1

namespace functional_data_structures { namespace lowlevel_data {
  namespace fam {
    namespace here = ::functional_data_structures::lowlevel_data::fam;

    template<class T>
    using flex = T[FUNCTIONAL_DATA_STRUCTURES_FLEX];

    template<class T>
    struct alignas(T) storage_for {
      std::byte bytes[sizeof(T)];
    };

    template<class T>
    T* ptr_cast(auto* x) NOEX(static_cast<T*>(static_cast<void*>(x)))
    template<class T>
    requires(std::is_const_v<T>) //
        T* ptr_cast(auto const* x)
            NOEX(static_cast<T*>(statc_cast<void const*>(x)))

    template<class T>
    struct flex_storage_implicit {
      alignas(T) std::byte data;
      constexpr operator T*() NOEX(ptr_cast<T>(&data))
      constexpr operator T const*() const NOEX(ptr_cast<T const>(data))
    };

    template<class T>
    using flex_storage = flex<storage_for<T>>;

    namespace impl {
      constexpr std::size_t flex_offset{FUNCTIONAL_DATA_STRUCTURES_FLEX};
    }

    struct fam_of_tag {};
    struct fam_count_of_tag {};
    constexpr auto tag_invoke(fam_of_tag, auto&& x) NOEXDECL(FWD(x).fam())
    constexpr auto tag_invoke(fam_count_of_tag, auto&& x)
        NOEXDECL(FWD(x).fam_count())
    inline constexpr auto fam_of =
        [](auto&& x) NOEXDECL(tag_invoke(fam_of_tag{}, FWD(x)));
    inline constexpr auto fam_count_of =
        [](auto&& x) NOEXDECL(tag_invoke(fam_count_of_tag{}, FWD(x)));

    template<class T>
    using fam_elt_t = decltype(fam_of(std::declval<T>())[0]);

    template<std::unsigned_integral N>
    constexpr N sub_nowrap(N x, N y) NOEX(std::max(x, y) - y)

    constexpr std::size_t extra_fam_count(std::size_t n)
        NOEX(sub_nowrap(n, impl::flex_offset))

    constexpr std::size_t extra_fam_count_of(auto& x)
        NOEX(extra_fam_count(fam_count_of(x)))

    template<class T>
    constexpr std::size_t bytesize_for_n(std::size_t n)
        NOEX(sizeof(T) + extra_fam_count(n) * sizeof(fam_elt_t<T>))

    constexpr auto extra_begin(auto& x)
        NOEXDECL(fam_of(x) + impl::flex_offset)
    constexpr auto fam_end(auto& x) NOEXDECL(fam_of(x) + fam_count_of(x))

    struct disable_new_and_delete {
      void* operator new(std::size_t)                     = delete;
      void* operator new[](std::size_t)                   = delete;
      void* operator new(std::size_t, std::align_val_t)   = delete;
      void* operator new[](std::size_t, std::align_val_t) = delete;

      void operator delete(void*)   = delete;
      void operator delete[](void*) = delete;
    };

    constexpr auto destroy_backwards_n(auto end, auto n)
        NOEXDECL(destroy_n(std::reverse_iterator(end), n))
    constexpr void destroy_extras(auto& x)
        NOEX(destroy_backwards_n(fam_end(x), extra_fam_count_of(x)))

    template<class Derived>
    struct fam_initializer {
     private:
      constexpr Derived& This() NOEX(*static_cast<Derived*>(this))
     public:
      // https://en.cppreference.com/w/cpp/language/constructor
      //
      // 1) If the constructor is for the most-derived class, virtual bases
      // are initialized in the order in which they appear in depth-first
      // left-to-right traversal of the base class declarations
      // (left-to-right refers to the appearance in base-specifier lists)
      //
      // 2) Then, direct bases are initialized in left-to-right order as
      // they appear in this class's base-specifier list
      //
      // 3) Then, non-static data member are initialized in order of
      // declaration in the class definition.
      //
      // 4) Finally, the body of the constructor is executed
      fam_initializer(std::size_t n)
          NOEX(std::uninitialized_default_construct_n(extras_begin(This()),
                                                      extra_fam_count(n)))
      fam_initializer(std::size_t) requires
          std::is_trivially_default_constructible_v<fam_elt_t<Derived>>
      = default;
    };

    template<class Base>
    struct fam_wrapper
        : disable_new_and_delete,
          fam_initializer<fam_wrapper<Base>>,
          Base {
     private:
      constexpr Base&       This() NOEX(*static_cast<Base*>(this))
      constexpr static bool base_has_destroy =
          (requires(Base b) { b.destroy(); });

     public:
      template<class T>
      explicit(std::is_convertible_v<T&&, Base>) constexpr fam_wrapper(T&& x)
          : fam_initializer<fam_wrapper<Base>>{Base::count_for_args(x)},
            Base(FWD(x)) {}
      constexpr fam_wrapper(auto&&... args)
          : fam_initializer<fam_wrapper<Base>>{Base::count_for_args(
              args...)},
            Base(FWD(args)...) {}
      // https://en.cppreference.com/w/cpp/language/destructor
      // Destruction sequence
      //
      // For both user-defined or implicitly-defined destructors, after
      // executing the body of the destructor and destroying any automatic
      // objects allocated within the body,
      //
      // [this is where we want to destroy the fam]
      // the compiler calls the destructors for all non-static non-variant
      // members of the class, in reverse order of declaration,
      //
      // then it calls the destructors of all direct non-virtual base
      // classes in reverse order of construction (which in turn call the
      // destructors of their members and their base classes, etc),
      //
      // and then, if this object is of most-derived class, it calls the
      // destructors of all virtual bases.
      constexpr ~fam_wrapper() {
        if constexpr(base_has_destroy) This().destroy();
        destroy_extras(This());
      }
      constexpr ~fam_wrapper() requires
          std::is_trivially_destructible_v<fam_elt_t<Base>> //
          and(!base_has_destroy) = default;
    };

#define FAM(class_name, ...)                                              \
  class_name /**/ : public ::functional_data_structures::lowlevel_data::  \
                        fam::fam_wrapper<decltype([]() {                  \
                          class_name __VA_ARGS__;                         \
                          return ::std::declval<class_name>();            \
                        }())>{};

    template<class T>
    FAM(struct fam_mac_test,
        {
          std::size_t size;
          flex<T>     ns;

          constexpr static std::size_t count_for_args(std::size_t n) NOEX(n)
          constexpr T*                 fam() NOEX(ns)
          constexpr std::size_t        fam_count() NOEX(size)
        })

    template<class T>
    T* famnew(auto&&... args) {
      auto const famcount = T::count_for_args(args...);
      auto       buf      = std::make_unique<std::byte[]>(famcount);
      T* const   obj      = new(buf.get()) T(FWD(args)...);
      buf.release();
      return obj;
    }

    template<class T>
    void famdel(T* p) noexcept(noexcept(std::destroy_at(p))) {
      std::unique_ptr<std::byte[]> buf = ptr_cast<std::byte>(p);
      std::destroy_at(p);
    }

    template<class T>
    constexpr auto make_unique_fam(auto&&... args)
        NOEXDECL(std::unique_ptr{famnew<T>(FWD(args)...), famdel<T>})

    template<class T>
    constexpr auto make_shared_fam(auto&&... args)
        NOEXDECL(std::shared_ptr{make_unique_fam<T>(FWD(args)...)})

}}} // namespace functional_data_structures::lowlevel_data::fam
#endif

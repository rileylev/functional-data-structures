#ifndef functional_data_structures_00_lowlevel_data_00_fam_hpp_INCLUDE_GUARD
#define functional_data_structures_00_lowlevel_data_00_fam_hpp_INCLUDE_GUARD

#include "../impl/macs.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>

namespace functional_data_structures { namespace lowlevel_data {
  namespace fam {
#if true
#  if true
    namespace here = ::functional_data_structures::lowlevel_data::fam;
    namespace rng  = std::ranges;

    template<class T>
    struct fam_storage {
#    if true
      alignas(T) std::byte sentinel;
#      define DEFS(const_)                                                \
        constexpr T const_* get() const_ NOEX(                            \
            std::launder(reinterpret_cast<T const_*>(&sentinel)))         \
        /* TODO: need launder? */                                         \
        constexpr operator T const_*() const_ NOEX(get())                 \
        /* we don't know the size, so use void* */                        \
        constexpr void const_* operator&()                                \
            const_             NOEX(static_cast<void const_*>(this))
      DEFS()
      DEFS(const)
#      undef DEFS
#    endif
    };

    struct famsize {
      std::size_t size;
      constexpr   operator std::size_t() NOEX(size)
    };

    template<class T>
    struct fam : fam_storage<T> {
      constexpr fam() = delete;
      constexpr fam() requires std::is_trivially_default_constructible_v<T>
      = default;
      constexpr fam(std::size_t) //
          requires std::is_trivially_default_constructible_v<T> {}
      constexpr fam(std::size_t n)
          NOEX(std::uninitialized_default_construct_n(this->get(), n))
      constexpr fam(rng::input_range auto const& src) NOEX(
          rng::uninitialized_copy(src, rng::subrange(this->get(), nullptr)))
      constexpr fam(auto scribble_my_memory)
          NOEX(scribble_my_memory(this->get())) // 🤷
    };

#    define TI_DECLARE_(args, name)                                       \
      namespace tag {                                                     \
        struct name {};                                                   \
      }                                                                   \
      inline constexpr auto name = [](auto&&... args)                     \
          NOEXDECL(tag_invoke(tag::name{}, FWD(args)...))
#    define TI_DECLARE(name) TI_DECLARE_(GENSYM(args), name);

    TI_DECLARE(fam_storage_of)
    constexpr auto tag_invoke(tag::fam_storage_of, auto&& x)
        NOEXDECL(FWD(x).fam_storage())
    TI_DECLARE(fam_count_of)
    constexpr auto tag_invoke(tag::fam_count_of, auto&& x)
        NOEXDECL(FWD(x).fam_count())

    template<class T>
    using fam_elt_t = decltype(fam_of(std::declval<T>())[0]);

    template<std::unsigned_integral N>
    constexpr N sub_nowrap(N x, N y) NOEX(std::max(x, y) - y)
    template<std::unsigned_integral N>
    constexpr N dec_nowrap(N x) NOEX(sub_nowrap(x, N{1}))

    template<class T>
    constexpr std::size_t bytesize_for_n(std::size_t n)
        NOEX(dec_nowrap(sizeof(T)) + n * sizeof(fam_elt_t<T>))

    template<class T>
    constexpr std::size_t bytesize_for_args(auto&&... args)
        NOEX(bytesize_for_n(T::count_for_args(FWD(args)...)))

    constexpr auto fam_begin(auto& x) NOEXDECL(fam_of(x))
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
    constexpr void destroy_fam(auto& x)
        NOEX(destroy_backwards_n(fam_end(x), fam_count_of(x)))

    // mixin the base so that local variables are still alive when we
    // destroy the fam
    template<class Base>
    struct fam_destroy_wrapper : disable_new_and_delete, Base {
     private:
      constexpr Base&       This() NOEX(*static_cast<Base*>(this))
      constexpr static bool base_has_destroy =
          (requires(Base b) { b.destroy(); });

     public:
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
      constexpr ~fam_destroy_wrapper() {
        if constexpr(base_has_destroy) This().destroy();
        destroy_fam(This());
      }
      constexpr ~fam_destroy_wrapper() requires
          std::is_trivially_destructible_v<fam_elt_t<Base>> //
          and(!base_has_destroy) = default;
    };

#    define FAM(class_name, ...)                                             \
      class_name /**/ : public ::functional_data_structures::lowlevel_data:: \
                            fam::fam_destroy_wrapper<decltype([]() {         \
                              class_name __VA_ARGS__;                        \
                              return ::std::declval<class_name>();           \
                            }())> {}

    template<class T, std::size_t N>
    using c_array = T[N];

    template<class T, class size_type = std::size_t>
    class array_impl {
      size_type size;
      fam<T>    data;

     public:
      constexpr array_impl(famsize n) : size{n}, data{n} {}
      constexpr static std::size_t count_for_args(famsize n) NOEX(n)

      constexpr array_impl(rng::sized_range auto const&& r)
          : size{rng::size(r)}, data{FWD(r)} {}
      constexpr static std::size_t
          count_for_args(rng::sized_range auto const&& r)
              NOEX(std::size(FWD(r)))

      template<std::size_t N>
      constexpr array_impl(c_array<T, N>&& src)
          : array_impl(std::span{std::begin(src), N}) {}

      constexpr T*          fam_storage() NOEX(size)
      constexpr std::size_t fam_count() NOEX(size)

      constexpr T&       operator[](std::size_t n) NOEX(data[n])
      constexpr T const& operator[](std::size_t n) const NOEX(data[n])
    };

    template<class T, class size_type = std::size_t>
    struct array : fam_destroy_wrapper<array_impl<T, size_type>> {};

    template<class T>
    T* famnew(auto&&... args) {
      auto const bytesize = bytesize_for_args<T>(args...);
      auto       buf      = std::make_unique<std::byte[]>(bytesize);
      T* const   obj      = new(buf.get()) T(FWD(args)...);
      buf.release();
      return obj;
    }

    template<class T>
    void famdel(T* p) noexcept(noexcept(std::destroy_at(p))) {
      std::unique_ptr<std::byte[]> buf{reinterpret_cast<std::byte*>(p)};
      std::destroy_at(p);
    }

    template<class T>
    constexpr auto make_unique_fam(auto&&... args) NOEXDECL(
        std::unique_ptr{here::famnew<T>(FWD(args)...), here::famdel<T>})

    template<class T>
    constexpr auto make_shared_fam(auto&&... args)
        NOEXDECL(std::shared_ptr{here::make_unique_fam<T>(FWD(args)...)})

#  endif
#endif
}}} // namespace functional_data_structures::lowlevel_data::fam
#endif

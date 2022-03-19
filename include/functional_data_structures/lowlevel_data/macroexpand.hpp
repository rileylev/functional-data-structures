#include <cstdint>
#include <memory>

namespace functional_data_structures { namespace lowlevel_data {
  namespace fam {
    namespace here = ::functional_data_structures::lowlevel_data::fam;
    template<class T>
    using flex = T[];

    namespace impl {
      constexpr std::size_t flex_offset = std::size_t{};
    }

    constexpr auto custom_functional_data_structures_fam_of(
        auto&& x) noexcept(noexcept((static_cast<decltype(x)&&>(x)).fam()))
        -> decltype((static_cast<decltype(x)&&>(x)).fam()) {
      return (static_cast<decltype(x)&&>(x)).fam();
    }
    constexpr auto
        custom_functional_data_structures_fam_count_of(auto&& x) noexcept(
            noexcept((static_cast<decltype(x)&&>(x)).fam_count()))
            -> decltype((static_cast<decltype(x)&&>(x)).fam_count()) {
      return (static_cast<decltype(x)&&>(x)).fam_count();
    }

    inline constexpr auto fam_of =
        [](auto&& x) noexcept(
            noexcept(custom_functional_data_structures_fam_of(
                (static_cast<decltype(x)&&>(x)))))
        -> decltype(custom_functional_data_structures_fam_of(
            (static_cast<decltype(x)&&>(x)))) {
      return custom_functional_data_structures_fam_of(
          (static_cast<decltype(x)&&>(x)));
    };
    inline constexpr auto fam_count_of =
        [](auto&& x) noexcept(
            noexcept(custom_functional_data_structures_fam_count_of(
                (static_cast<decltype(x)&&>(x)))))
        -> decltype(custom_functional_data_structures_fam_count_of(
            (static_cast<decltype(x)&&>(x)))) {
      return custom_functional_data_structures_fam_count_of(
          (static_cast<decltype(x)&&>(x)));
    };

    template<class T>
    using fam_elt_t = decltype(fam_of(std::declval<T>())[0]);

    constexpr std::size_t extra_fam_count(std::size_t n) noexcept(
        noexcept(std::max(n, impl::flex_offset) - impl::flex_offset)) {
      return std::max(n, impl::flex_offset) - impl::flex_offset;
    }

    constexpr std::size_t extra_fam_count_of(auto& x) noexcept(
        noexcept(extra_fam_count(fam_count_of(x)))) {
      return extra_fam_count(fam_count_of(x));
    }

    template<class T>
    constexpr std::size_t bytesize_for_n(std::size_t n) noexcept(
        noexcept(sizeof(T) + extra_fam_count(n) * sizeof(fam_elt_t<T>))) {
      return sizeof(T) + extra_fam_count(n) * sizeof(fam_elt_t<T>);
    }

    constexpr auto extra_begin(auto& x) noexcept(
        noexcept(fam_of(x) + impl::flex_offset))
        -> decltype(fam_of(x) + impl::flex_offset) {
      return fam_of(x) + impl::flex_offset;
    }
    constexpr auto
        fam_end(auto& x) noexcept(noexcept(fam_of(x) + fam_count_of(x)))
            -> decltype(fam_of(x) + fam_count_of(x)) {
      return fam_of(x) + fam_count_of(x);
    }

    struct disable_new_and_delete {
      void* operator new(std::size_t)                     = delete;
      void* operator new[](std::size_t)                   = delete;
      void* operator new(std::size_t, std::align_val_t)   = delete;
      void* operator new[](std::size_t, std::align_val_t) = delete;

      void operator delete(void*)   = delete;
      void operator delete[](void*) = delete;
    };

    constexpr void default_construct_extras(auto& x) noexcept(noexcept(
        std::uninitialized_default_construct_n(extras_begin(x),
                                               extra_fam_count_of(x)))) {
      return std::uninitialized_default_construct_n(extras_begin(x),
                                                    extra_fam_count_of(x));
    }

    constexpr void value_construct_extras(auto& x) noexcept(noexcept(
        std::uninitialized_value_construct_n(extras_begin(x),
                                             extra_fam_count_of(x)))) {
      return std::uninitialized_value_construct_n(extras_begin(x),
                                                  extra_fam_count_of(x));
    }

    constexpr void fill_extras(auto& x, auto const& value) noexcept(
        noexcept(std::uninitialized_fill_n(extras_begin(x),
                                           extra_fam_count_of(x),
                                           value))) {
      return std::uninitialized_fill_n(extras_begin(x),
                                       extra_fam_count_of(x),
                                       value);
    }

    constexpr auto destroy_backwards_n(auto end, auto n) noexcept(
        noexcept(destroy_n(std::reverse_iterator(end), n)))
        -> decltype(destroy_n(std::reverse_iterator(end), n)) {
      return destroy_n(std::reverse_iterator(end), n);
    }
    constexpr void destroy_extras(auto& x) noexcept(
        noexcept(destroy_backwards_n(fam_end(x), extra_fam_count_of(x)))) {
      return destroy_backwards_n(fam_end(x), extra_fam_count_of(x));
    }

    template<class Base>
    struct fam_wrapper : Base {
      using Base::Base;

     private:
      constexpr Base& This() noexcept(noexcept(*static_cast<Base*>(this))) {
        return *static_cast<Base*>(this);
      }
      constexpr static bool base_has_destroy =
          (requires(Base b) { b.destroy(); });

     public:

      constexpr ~fam_wrapper() {
        if constexpr(base_has_destroy) This().destroy();
        destroy_extras(This());
      }
      constexpr ~fam_wrapper() requires std::is_trivially_destructible_v<
          fam_elt_t<Base>> and(!base_has_destroy) = default;
    };

    template<class T>
    struct fam_test_impl {
      std::size_t size;
      flex<T>     ns;

      constexpr static std::size_t
          count_for_args(std::size_t n) noexcept(noexcept(n)) {
        return n;
      }

      constexpr T*          fam() noexcept(noexcept(ns)) { return ns; }
      constexpr std::size_t fam_count() noexcept(noexcept(size)) {
        return size;
      }
    };

    template<class T>
    struct fam_test
        : disable_new_and_delete,
          fam_wrapper<fam_test_impl<T>> {};

    template<class T>
    struct fam_mac_test : public fam_wrapper<decltype([]() {
      struct fam_mac_test {
        std::size_t size;
        flex<T>     ns;
        constexpr static std::size_t
            count_for_args(std::size_t n) noexcept(noexcept(n)) {
          return n;
        }
        constexpr T*          fam() noexcept(noexcept(ns)) { return ns; }
        constexpr std::size_t fam_count() noexcept(noexcept(size)) {
          return size;
        }
      };
      return std::declval<struct fam_mac_test>(); }())>{};

    template<class T>
    struct fam_lam_test : fam_wrapper<decltype([](){
      struct fam_lam_test : disable_new_and_delete {
        std::size_t size;
        flex<T>     ns;

        constexpr static std::size_t
            count_for_args(std::size_t n) noexcept(noexcept(n)) {
          return n;
        }

        constexpr T*          fam() noexcept(noexcept(ns)) { return ns; }
        constexpr std::size_t fam_count() noexcept(noexcept(size)) {
          return size;
        }
      };
      return std::declval<fam_lam_test>();}())> {};

    using fami = fam_lam_test<int>;

    template<class T>
    T* famnew(auto&&... args) {
      auto const famcount = T::count_for_args(args...);
      auto       buf      = std::make_unique<std::byte[]>(famcount);
      T* const   obj =
          new(buf.get()) T((static_cast<decltype(args)&&>(args))...);
      buf.release();
      return obj;
    }

    template<class T>
    void famdel(T* p) noexcept {
      std::destroy_at(p);
      std::byte* buf = reintepret_cast<std::byte*>(p);
      delete[] buf;
    }

    template<class T>
    constexpr auto make_unique_fam(auto&&... args) noexcept(noexcept(
        std::unique_ptr{famnew<T>((static_cast<decltype(args)&&>(args))...),
                        famdel<T>})) -> decltype(std::unique_ptr{
        famnew<T>((static_cast<decltype(args)&&>(args))...),
        famdel<T>}) {
      return std::unique_ptr{
          famnew<T>((static_cast<decltype(args)&&>(args))...),
          famdel<T>};
    }

    template<class T>
    constexpr auto
        make_shared_fam(auto&&... args) noexcept(noexcept(std::shared_ptr{
            make_unique_fam<T>((static_cast<decltype(args)&&>(args))...)}))
            -> decltype(std::shared_ptr{make_unique_fam<T>(
                (static_cast<decltype(args)&&>(args))...)}) {
      return std::shared_ptr{
          make_unique_fam<T>((static_cast<decltype(args)&&>(args))...)};
    }

    using fam_test_test = fam_test<int>;
}}} // namespace functional_data_structures::lowlevel_data::fam

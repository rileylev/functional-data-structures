#ifndef functional_data_structures_leftist_heap_hpp_INCLUDE_GUARD
#define functional_data_structures_leftist_heap_hpp_INCLUDE_GUARD

#include "impl/macs.hpp"
#include "impl/pass.hpp"
#include "memory/concepts.hpp"

#include <cstdint>
#include <functional>

namespace functional_data_structures {

using namespace functional_data_structures::impl::pass;

template<class To, class From>
constexpr To narrow(From const x) ANOEXCEPT() {
  To y = static_cast<To>(x);
  ASSERT(std::cmp_equal(x, y));
  return y;
}

template<class handles, class Elt, class Rank = std::int8_t>
// Rank <= floor(log(size + 1))  (Okasaki, p. 18 exercise 3.1)
// for 64 bit pointers, 64 = 2^6, so sizeof(Rank) <= 6.
// never need more than a byte
struct node_data {
  using handle_t  = typename handles::template handle<node_data>;
  using element_t = Elt;
  using rank_t    = Rank;

  Elt      elt_;
  Rank     rank_{};
  handle_t left_;
  handle_t right_;

  node_data() = default;
  node_data(Elt elt, Rank rank, handle_t left, handle_t right)
      : elt_{elt}, rank_{rank}, left_{left}, right_{right} {}

  ACCESSOR(rank)
  ACCESSOR(elt)
  ACCESSOR(left)
  ACCESSOR(right)
};

template<class NodeData>
struct node_basis {
  using node_data_t = NodeData;
  using element_t   = typename node_data_t::element_t;
  using handle_t    = typename node_data_t::handle_t;
  using rank_t      = typename node_data_t::rank_t;

  template<memory::can_store<node_data_t> Mem>
  struct with_mem_t {
    [[no_unique_address]] Mem mem;

    constexpr rank_t rank(handle_t h)
        NOEX(mem.is_null(h) ? rank_t{} : mem[h].rank())

    constexpr handle_t make() NOEX(mem.null())
    constexpr handle_t make(element_t x)
        NOEX(make(std::move(x), make(), make()))
    constexpr handle_t make(element_t x, handle_t a, handle_t b) NOEX(
        (rank(a) >= rank(b))
            ? mem.template make<node_data_t>(
                narrow<rank_t>(rank(b) + 1),
                std::move(x),
                std::move(a),
                std::move(b))
            : mem.template make<node_data_t>(
                narrow<rank_t>(rank(a) + 1),
                std::move(x),
                std::move(b),
                std::move(a)))

    constexpr ReadOut<element_t> top(ReadIn<handle_t> h) NOEX(mem[h].elt())

    template<class Leq>
    struct with_leq_t : with_mem_t {
      [[no_unique_address]] Leq leq;
      constexpr handle_t merge(ReadIn<handle_t> h0, ReadIn<handle_t> h1) {
        if(mem.is_null(h0)) return h1;
        if(mem.is_null(h1)) return h0;

        // clang-format off
        auto&& x0 = mem[h0].elt()   ; auto&& x1 = mem[h1].elt()   ;
        auto&& a0 = mem[h0].left()  ; auto&& a1 = mem[h1].left()  ;
        auto&& b0 = mem[h0].right() ; auto&& b1 = mem[h1].right() ;
        // clang-format on

        return leq(x0, x1) ? make(x0, a0, merge(b0, h1))
                           : make(x1, a1, merge(h0, b1));
      }

      constexpr handle_t pop(ReadIn<handle_t> h) noexcept(
          merge(leq, mem[h].left(), mem[h].right())) {
        ASSERT(!mem.is_null(h))
        return merge(leq, mem[h].left(), mem[h].right());
      }

      constexpr handle_t insert(ReadIn<handle_t> h, element_t elt)
          NOEX(leq(elt, mem[h].elt())
                   ? make(elt, make(), h)
                   : make(mem[h].elt(),
                          mem[h].left(),
                          insert(elt, mem[h].right())))
    };

    constexpr auto with_leq(auto leq) NOEX_DECL(with_leq_t{mem, leq})
  };

  template<class Mem>
  static constexpr auto with_mem(Mem mem) RET(with_mem_t<Mem>{mem})
};

template<class Elt, class Mem, class Leq = std::less_equal<>>
class leftist_heap {
  using handles   = typename Mem::handles;
  using element_t = Elt;
  using handle_t  = typename handles::template handle<element_t>;
  using node_t    = node_data<handles, element_t>;
  using rank_t    = typename node_t::rank_t;
  using basis_t   = node_basis<node_t>;

  [[no_unique_address]] Mem                    mem_;
  [[no_unique_address]] Leq                    leq_;
  typename handles::template handle<element_t> root_;

 public:
  leftist_heap() = default;
  constexpr leftist_heap(Mem mem, Leq leq = {})
      : leftist_heap(mem, leq, mem.null()) {}
  constexpr leftist_heap(Mem mem, Leq leq, handle_t root)
      : mem_{mem}, leq_{leq}, root_{root} {}

  constexpr bool               empty() NOEX(mem_.is_null(root_))
  constexpr ReadOut<element_t> top() ANOEXCEPT() {
    ASSERT(!empty());
    return root_.elt();
  }

  constexpr leftist_heap insert(element_t elt) NOEX(leftist_heap{
      mem_,
      leq_,
      basis_t::with_mem(mem_).with_leq(leq_).insert(root_, std::move(elt))})

  constexpr leftist_heap pop()
      NOEX(leftist_heap{mem_,
                        leq_,
                        basis_t::with_mem(mem_).with_leq(leq_).pop(root_)})
};

} // namespace functional_data_structures

#endif // FUNCTIONAL_DATA_STRUCTURES_LEFTIST_HEAP_HPP_INCLUDE_GUARD

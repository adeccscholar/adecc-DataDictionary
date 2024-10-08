#pragma once

#include <ranges>
#include <concepts> 
#include <coroutine>
#include <experimental/generator>

inline std::experimental::generator<long long> fibunacci() {
   co_yield 0l;
   long long prepredecessor = 0;
   long long predecessor = 1;
   for (auto step : std::views::iota(0)) {
      long long next = prepredecessor + predecessor;
      prepredecessor = predecessor;
      predecessor = next;
      co_yield next;
      }
   }

namespace own {
   struct filter_odd {
      template <std::ranges::input_range range_ty>
         requires std::integral<std::ranges::range_value_t<range_ty>>
      auto operator()(range_ty&& r) const -> std::experimental::generator<std::ranges::range_value_t<range_ty>> {
         for (const auto& elem : r) {
            if (elem % 2 == 1) {
               co_yield elem;
               }
            }
         }

      template <std::ranges::input_range range_ty>
      friend auto operator | (range_ty&& r, filter_odd const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      };

   namespace views {
      inline constexpr auto odd = filter_odd{};
      }
   }

void fibunacci_test();

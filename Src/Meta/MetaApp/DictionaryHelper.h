#pragma once

#include <ranges>

#include <map>
#include <vector>
#include <set>
#include <array>
#include <span>
#include <utility>

#include <cstddef>  
#include <concepts> 
#include <coroutine>
#include <experimental/generator>

namespace own {

   // --------------------------------------------------------------------------
   template <typename ty, typename = void>
   struct is_map : std::false_type {};

   template <typename key_ty, typename value_ty, typename compare_ty, typename allocator_ty>
   struct is_map<std::map<key_ty, value_ty, compare_ty, allocator_ty>> : std::true_type {};
   // --------------------------------------------------------------------------
   template <typename ty, typename = void>
   struct is_vector : std::false_type {};

   template <typename ty, typename alloc_ty>
   struct is_vector<std::vector<ty, alloc_ty>> : std::true_type {};

   // --------------------------------------------------------------------------
   template <typename ty, typename = void>
   struct is_set : std::false_type {};

   template <typename ty, typename compare_ty, typename alloc_ty>
   struct is_set<std::set<ty, compare_ty, alloc_type>> : std::true_type {};
   
   // --------------------------------------------------------------------------
   template <typename ty, typename = void>
   struct is_span : std::false_type {};

   template <typename ty, std::size_t Extent>
   struct is_span<std::span<ty, Extent>> : std::true_type {};

   // --------------------------------------------------------------------------
   template <typename ty>
   struct is_array : std::false_type {};

   template <typename ty, std::size_t N>
   struct is_array<std::array<ty, N>> : std::true_type {};

   // ----------------------------------------------------------------------------
   template <typename ty>
   struct is_pair : std::false_type {};

   template <typename first_ty, typename second_ty>
   struct is_pair<std::pair<first_ty, second_ty>> : std::true_type {};


   template <typename, typename = void>
   struct has_value_type : std::false_type {};

   template <typename ty>
   struct has_value_type<ty, std::void_t<typename ty::value_type>> : std::true_type {};

 
   template <typename ty, typename = void>
   struct is_vector_pair : std::false_type {};


   template <typename ty> 
   struct is_vector_pair<ty, std::enable_if_t<is_vector<ty>::value && has_value_type<ty>::value &&
                                             is_pair<typename ty::value_type>::value>> : std::true_type {};

   static_assert(is_vector<std::vector<int>>::value, "std::vector<int> ist ein std::vector");
   static_assert(!is_vector_pair<std::vector<int>>::value, "std::vector<int> ist ein std::vector");
   static_assert(is_vector_pair<std::vector<std::pair<int, int>>>::value, "std::vector<int> ist ein std::vector");


   template <typename ty>
   struct is_set_pair {
      static constexpr bool value = is_set<ty>::value && has_value_type<ty>::value &&
                                    is_pair<typename ty::value_type>::value;
      };


   template <typename, typename = void>
   struct has_element_type : std::false_type {};

   template <typename ty>
   struct has_element_type<ty, std::void_t<typename ty::element_type>> : std::true_type {};


   template <typename ty, typename = void>
   struct is_span_pair : std::false_type {};

   template <typename ty>
   struct is_span_pair<ty, std::enable_if_t<is_span<ty>::value && has_element_type<ty>::value && 
                       is_pair<typename ty::element_type>::value>> : std::true_type {};


   template <typename ty, typename = void>
   struct is_array_pair : std::false_type {};

   template <typename ty>
   struct is_array_pair<ty, std::enable_if_t<is_span<ty>::value && has_value_type<ty>::value && 
                        is_pair<typename ty::value_type>::value>> : std::true_type {};


   template <typename ty>
   struct has_pair_value {

      static constexpr bool value = is_map<ty>::value || is_vector_pair<ty>::value ||
                                    is_set_pair<ty>::value || is_span_pair<ty>::value ||
                                    is_array_pair<ty>::value;
      };



   
   //template <typename ty>
   //struct is_container : std::disjunction<is_map<ty>, is_vector<ty>, is_set<ty>, is_span<ty>> {};


   static_assert( is_map<std::map<int, int>>::value, "std::map<int, int>> ist eine std::map");
   static_assert(!is_map<std::vector<int>>::value,   "std::vector<int> ist keine std::map");
   static_assert(!is_map<std::set<int>>::value,      "std::set<int> ist keine std::map");
   static_assert(!is_map<std::span<int>>::value,     "std::span<int> ist keine std::map");
   static_assert(!is_map<std::array<int, 5>>::value, "std::array<int, 5> ist keine std::map");


   static_assert( is_vector<std::vector<int>>::value,   "std::vector<int> ist ein std::vector");
   static_assert(!is_vector<std::map<int, int>>::value, "std::map<int, int>> ist kein std::vector");
   static_assert(!is_vector<std::set<int>>::value,      "std::set<int> ist kein std::vector");
   static_assert(!is_vector<std::span<int>>::value,     "std::span<int> ist kein std::vector");
   static_assert(!is_vector<std::array<int, 5>>::value, "std::array<int, 5> ist kein std::vector");

   static_assert( is_set<std::set<int>>::value,      "std::set<int> ist ein std::set");
   static_assert(!is_set<std::map<int, int>>::value, "std::map<int, int>> ist kein std::set");
   static_assert(!is_set<std::vector<int>>::value,   "std::vector<int> ist kein std::set");
   static_assert(!is_set<std::span<int>>::value,     "std::span<int> ist kein std::set");
   static_assert(!is_set<std::array<int, 5>>::value, "std::array<int, 5> ist kein std::set");

   static_assert( is_span<std::span<int>>::value,     "std::span<int> ist ein std::span");
   static_assert(!is_span<std::vector<int>>::value,   "std::vector<int> ist kein std::span");
   static_assert(!is_span<std::set<int>>::value,      "std::set<int> ist kein std::span");
   static_assert(!is_span<std::map<int, int>>::value, "std::map<int, int> ist kein std::span");
   static_assert(!is_span<std::array<int, 5>>::value, "std::array<int, 5> ist kein std::span");

   static_assert( is_array<std::array<int, 5>>::value, "std::array<int, 5> ist ein std::array");
   static_assert(!is_array<std::span<int>>::value,     "std::span<int> ist kein std::array");
   static_assert(!is_array<std::vector<int>>::value,   "std::vector<int> ist kein std::array");
   static_assert(!is_array<std::set<int>>::value,      "std::set<int> ist kein std::array");
   static_assert(!is_array<std::map<int, int>>::value, "std::map<int, int> ist kein std::array");


   static_assert(is_vector_pair<std::vector<std::pair<int, int>>>::value, "std::vector<std::pair<int, int>> ist ein std::vector<std::pair>");
   static_assert(!is_vector_pair<std::vector<int>>::value, "std::vector<int> ist kein std::vector<std::pair>");
   static_assert(!is_vector_pair<std::set<std::pair<int, int>>>::value, "std::set<std::pair<int, int>> ist kein std::vector<std::pair>");
   static_assert(!is_vector_pair<std::set<int>>::value, "std::set<int> ist kein std::vector<std::pair>");

   static_assert(is_set_pair<std::set<std::pair<int, int>>>::value, "std::set<std::pair<int, int>> ist kein std::set<std::pair>");
   static_assert(!is_set_pair<std::set<int>>::value, "std::set<int> ist kein std::set<std::pair>");
   static_assert(!is_set_pair<std::vector<std::pair<int, int>>>::value, "std::vector<std::pair<int, int>> ist ein std::set<std::pair>");
   static_assert(!is_set_pair<std::vector<int>>::value, "std::vector<int> ist kein std::set<std::pair>");

   static_assert(has_pair_value<std::map<int, std::string>>::value, "std::map<int, std::string>> hat einen value als std::pair");
   static_assert(has_pair_value<std::vector<std::pair<int, int>>>::value, "std::vector<std::pair<int, int>> hat einen value als std::pair");
   static_assert(has_pair_value<std::set<std::pair<int, int>>>::value, "std::set<std::pair<int, int>> hat einen value als std::pair");
   static_assert(!has_pair_value<std::vector<int>>::value, "std::vector<int> hat keinen value als std::pair");
   static_assert(!has_pair_value<std::set<int>>::value, "std::set<int> hat keinen value als std::pair");


   template <typename ty>
   concept map_type = is_map<ty>::value && !is_vector<ty>::value;

   template <typename ty>
   concept vector_type = is_vector<ty>::value && !is_map<ty>::value;

   template <typename ty>
   concept vector_type_of_pairs = is_vector_pair<ty>::value;

   template <typename ty>
   concept set_type = is_set<ty>::value;

   template <typename ty>
   concept set_type_of_pairs = is_set_pair<ty>::value;

   template <typename ty>
   concept span_type = is_span<ty>::value;

   template <typename ty>
   concept span_type_of_pairs = is_span_pair<ty>::value;


   template <typename ty>
   concept array_type = is_array<ty>::value;

   template <typename ty>
   concept with_pairs = has_pair_value<ty>::value;

   template <typename ty>
   concept without_pairs = !has_pair_value<ty>::value;

   //*

   template <typename ty>
   struct pair_type_t;

   template <typename ty> requires map_type<ty>
   struct pair_type_t<ty> {
      using value_type = std::pair<typename ty::key_type, typename ty::mapped_type>;
      };

   template <typename ty> requires vector_type<ty> && with_pairs<ty>
   struct pair_type_t<ty> {
      using value_type = typename ty::value_type;
      };

   template <typename ty> requires set_type<ty> && with_pairs<ty>
   struct pair_type_t<ty> {
      using value_type = typename ty::value_type;
      };

   template <typename ty> requires array_type<ty>&& with_pairs<ty>
   struct pair_type_t<ty> {
      using value_type = typename ty::value_type;
      };


   template <typename ty> requires span_type<ty> && with_pairs<ty>
   struct pair_type_t<ty> {
      using value_type = typename ty::element_type;
      };


   template <typename ty>
   struct withoutpair_type_t;

   template <typename ty> requires map_type<ty>
   struct withoutpair_type_t<ty> {
      using value_type = std::pair<typename ty::key_type, typename ty::mapped_type>;
   };

   template <typename ty> requires vector_type<ty> && without_pairs<ty>
   struct withoutpair_type_t<ty> {
      using value_type = typename ty::value_type;
   };

   template <typename ty> requires set_type<ty> && without_pairs<ty>
   struct withoutpair_type_t<ty> {
      using value_type = typename ty::value_type;
   };

   template <typename ty> requires array_type<ty> && without_pairs<ty>
   struct withoutpair_type_t<ty> {
      using value_type = typename ty::value_type;
   };


   template <typename ty> requires span_type<ty> && without_pairs<ty>
   struct withoutpair_type_t<ty> {
      using value_type = typename ty::element_type;
   };


   template <with_pairs ty> 
   std::experimental::generator<typename pair_type_t<ty>::value_type> serialize(ty const& container) {
      for (const auto& value : container) {
         co_yield value;   
         }
      co_return;
      }
   
   template <without_pairs ty>
   std::experimental::generator<typename withoutpair_type_t<ty>::value_type> serialize(ty const& container) {
      for (int i = 1;  const auto & value : container) {
         co_yield value;
      }
      co_return;
   }


   template <typename ty>
   concept own_has_size = requires(ty const t) {
         { t.size() } -> std::convertible_to<size_t>;
      };

   template <typename ty>
   concept own_has_primary = requires(ty const t) {
         { t.Primary() } -> std::convertible_to<bool>;
         { t.ID() } -> std::convertible_to<size_t>;
      };



   template <class ty> 
   using first_view = std::ranges::elements_view<ty, 0>;
   
   template <class ty>
   using second_view = std::ranges::elements_view<ty, 1>;

   namespace views {
      inline constexpr auto first  = std::ranges::views::elements<0>;
      inline constexpr auto second = std::ranges::views::elements<1>;
      }


   struct primary_view {
      template <std::ranges::input_range range_ty>
         requires own_has_primary<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty && r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& e) { return e.Primary(); });
          }
   
      template <std::ranges::input_range range_ty>
         requires own_has_primary<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, primary_view const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      }; 

   struct size_view {
      template <std::ranges::input_range range_ty>
         requires own_has_size<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::transform([](auto const& e) -> std::size_t { return e.size(); });
         }

      template <std::ranges::input_range range_ty>
         requires own_has_size<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, size_view const& view) {
         return view(std::forward<range_ty>(r));
         }

      };

   namespace views {
      inline constexpr auto primary = primary_view{};
      inline constexpr auto size    = size_view{};
      }




   struct first_view_co {
      template <std::ranges::input_range range_ty>
      auto operator()(range_ty&& r) const -> std::experimental::generator<typename std::tuple_element<0, std::ranges::range_value_t<range_ty>>::type> {
         for (auto const& elem : r) {
            co_yield std::get<0>(elem);
            }
         }

      template <std::ranges::input_range range_ty>
      friend auto operator | (range_ty&& r, first_view_co const& view) {
         return view(std::forward<range_ty>(r));
         }

      };


   struct size_view_co {
      template <std::ranges::input_range range_ty>
         requires own_has_size<std::ranges::range_value_t<range_ty>>
      auto operator()(range_ty&& r) const -> std::experimental::generator<size_t> {
         for (const auto& elem : r) {
            co_yield static_cast<size_t>(elem.size());  
            }
         }
      
      template <std::ranges::input_range range_ty>
         requires own_has_size<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, size_view_co const& view) {
         return view(std::forward<range_ty>(r));  
         }

      };
   namespace views {
      inline constexpr auto first_co = first_view_co{};
      inline constexpr auto size_co  = size_view_co{};
      }
   
   }

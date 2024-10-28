#pragma once

#include "RangesHelper.h"

namespace own {

   template <typename ty>
   concept own_has_primary = requires(ty const t) {
         { t.Primary() } -> std::convertible_to<bool>;
         { t.ID() } -> std::convertible_to<size_t>;
      };


   template<typename ty>
   concept own_has_indices = std::is_enum_v<decltype(std::declval<ty>().IndexType())> &&
                             std::is_same_v<decltype(std::declval<ty>().IndexType()), EMyIndexType>;

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

   struct non_primary_view {
      template <std::ranges::input_range range_ty>
         requires own_has_primary<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& e) { return !e.Primary(); });
         }

      template <std::ranges::input_range range_ty>
         requires own_has_primary<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, non_primary_view const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      };


   struct is_table_view {
      template <std::ranges::input_range range_ty>
         //requires own_has_primary<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& tbl) { return std::get<1>(tbl).EntityType() != EMyEntityType::view; });
         }

      template <std::ranges::input_range range_ty>
         //requires own_has_primary<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, is_table_view const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      };

   struct is_view_view {
      template <std::ranges::input_range range_ty>
      //requires own_has_primary<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& tbl) { return std::get<1>(tbl).EntityType() == EMyEntityType::view; });
         }

      template <std::ranges::input_range range_ty>
      //requires own_has_primary<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, is_view_view const& filter) {
         return filter(std::forward<range_ty>(r));
      }
   };

   struct is_index_view {
      template <std::ranges::input_range range_ty>
        requires own_has_indices<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& idx) { return idx.IndexType() != EMyIndexType::key; });
         }

      template <std::ranges::input_range range_ty>
        requires own_has_indices<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, is_index_view const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      };

   struct is_unique_key_view {
      template <std::ranges::input_range range_ty>
        requires own_has_indices<std::ranges::range_value_t<range_ty>>
      auto operator () (range_ty&& r) const {
         return std::forward<range_ty>(r) | std::views::filter([](auto const& idx) { return idx.IndexType() == EMyIndexType::key; });
         }

      template <std::ranges::input_range range_ty>
        requires own_has_indices<std::ranges::range_value_t<range_ty>>
      friend auto operator | (range_ty&& r, is_unique_key_view const& filter) {
         return filter(std::forward<range_ty>(r));
         }
      };

  
   namespace views {
      inline constexpr auto primary       = primary_view{};
      inline constexpr auto non_primary   = non_primary_view{};
      inline constexpr auto is_table      = is_table_view{};
      inline constexpr auto is_view       = is_view_view{};
      inline constexpr auto is_unique_key = is_unique_key_view{};
      inline constexpr auto is_index      = is_index_view{};
      }


// =================================================================================================

   
   }

#pragma once

#include <vector>
#include <string>
#include <ranges>
#include <stdexcept>
#include <algorithm>

template <typename ty>
concept MySize_T = std::same_as<ty, unsigned int> || std::is_convertible_v<ty, std::string>;

template <MySize_T... Args>
std::vector<std::pair<size_t, size_t>> Funktion(std::string strName, Args&&... args)  {
   std::vector<std::pair<size_t, size_t>> result;

   std::vector<size_t> values1;
   std::vector<size_t> values2;
   std::string separator;

   bool foundSeparator = false;

   (([&](const auto& arg) {
      if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) [[unlikely]] {
         separator = arg;
         foundSeparator = true;
         }
      else if (foundSeparator) {
         values2.emplace_back(static_cast<size_t>(arg));
         }
      else {
         values1.emplace_back(static_cast<size_t>(arg));
         }
      }(args)), ...);

   // Überprüfe, ob die Gruppen die gleiche Länge haben
   if (values1.size() != values2.size()) {
      throw std::invalid_argument("Ungleich lange Gruppen von size_t-Werten.");
      }

   auto zipped = std::views::zip(values1, values2);
   std::copy(zipped.begin(), zipped.end(), std::back_inserter(result));

   return result;
   }
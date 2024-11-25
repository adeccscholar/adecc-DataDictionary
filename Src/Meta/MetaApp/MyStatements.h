#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <format>

using namespace std::string_literals;

using myStatements = std::vector<std::string>;

inline myStatements& operator += (myStatements& stmts1, myStatements const& stmts2) {
   stmts1.reserve(stmts1.size() + stmts2.size());
   std::for_each(stmts2.begin(), stmts2.end(), [&stmts1](auto const& val) { stmts1.emplace_back(val);  });
   return stmts1;
   }

inline myStatements& operator += (myStatements& stmts, std::string const& val) {
   stmts.emplace_back(val);
   return stmts;
   }

inline myStatements operator + (myStatements const& stmts1, myStatements const& stmts2) {
   myStatements stmts;
   stmts.reserve(stmts1.size() + stmts2.size());
   std::for_each(stmts1.begin(), stmts1.end(), [&stmts](auto const& val) { stmts.emplace_back(val);  });
   std::for_each(stmts2.begin(), stmts2.end(), [&stmts](auto const& val) { stmts.emplace_back(val);  });
   return stmts;
   }

inline std::string toString(myStatements const& stmts) {
   return std::ranges::fold_left(stmts, std::string{}, [](std::string s, std::string const& v) { return s += "\n"s + v;  });
   }

inline void WriteSource(myStatements const& stmts, std::ostream& os) {
   switch(stmts.size()) {
      case 0:
         os << "     \"\";"s;
         break;
      default: [[likely]] //[[fallthrough]]
         for (auto const& row : stmts | std::views::take(stmts.size() - 1))
            os << std::format("     \"{}\\n\"\n", row);
         [[fallthrough]];
      case 1: 
         os << std::format("     \"{}\";", stmts.back());
         break;
      }
   }


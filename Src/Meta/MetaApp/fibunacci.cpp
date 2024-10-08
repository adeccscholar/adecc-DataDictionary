
#include "fibunacci.h"
#include "RangesHelper.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <algorithm>

using namespace std::string_literals;

using test_type = std::pair<int, std::string>;
std::experimental::generator<test_type> test_data() {

   for(test_type element : { 
      test_type{1, "banana"s}, test_type{2, "apple"s}, test_type{3, "pear"s}, test_type{4, "pineapple"s}, 
      test_type{5, "peach"s}, test_type{6, "mango"s}, test_type{7, "strawberry"s}, test_type{8, "orange"s}, 
      test_type{9, "watermelon"s}, test_type{10, "grape"s}, test_type{11, "cherry"s}, test_type{12, "blueberry"s},
      test_type{13, "raspberry"s}, test_type{14, "papaya"s}, test_type{15, "kiwi"s}, test_type{16, "pomegranate"s}, 
      test_type{17, "plum"s}, test_type{18, "avocado"s}, test_type{19, "coconut"s}, test_type{20, "fig"s}, 
      test_type{21, "lemon"s}, test_type{22, "lime"s}, test_type{23, "grapefruit"s}, test_type{24, "passionfruit"s},
      test_type{25, "dragonfruit"s}, test_type{26, "guava"s}, test_type{27, "apricot"s}, test_type{28, "lychee"s}, 
      test_type{29, "cantaloupe"s}, test_type{30, "blackberry"s}, test_type{31, "persimmon"s}, 
      test_type{32, "nectarine"s}, test_type{33, "cranberry"s}, test_type{34, "mulberry"s}, test_type{35, "gooseberry"s},
      test_type{36, "tangerine"s}, test_type{37, "durian"s}, test_type{38, "jackfruit"s}, test_type{39, "starfruit"s},
      test_type{40, "custard apple"s}, test_type{41, "acerola"s}, test_type{42, "rosehip"s}, test_type{43, "elderberry"s}, 
      test_type{44, "boysenberry"s}, test_type{45, "blackcurrant"s}, test_type{46, "redcurrant"s}, test_type{47, "blackthorn"s},
      test_type{48, "mangosteen"s}, test_type{49, "chokeberry"s}, test_type{ 50, "cloudberry"s }, test_type{ 51, "jujube"s },
      test_type{52, "physalis"s}, test_type{53, "tamarillo"s} 
       }) {
      co_yield element;
      }
   co_return;
   }


std::map<int, std::string> CreateTestData_Map() {
   return test_data() | std::ranges::to<std::map<int, std::string>>();
   }

std::vector<test_type> CreateTestData_Vec() {
   return test_data() | std::ranges::to<std::vector<std::pair<int, std::string>>>();
   }

void fibunacci_test() {

   

   test_type arrayTest[] = { {  1, "banana"s },       {  2, "apple"s },
                             {  3, "pear"s },         {  4, "pineapple"s },
                             {  5, "peach"s },        {  6, "mango"s },
                             {  7, "strawberry"s },   {  8, "orange"s },
                             {  9, "watermelon"s },   { 10, "grape"s },
                             { 11, "cherry"s },       { 12, "blueberry"s },
                             { 13, "raspberry"s },    { 14, "papaya"s },
                             { 15, "kiwi"s },         { 16, "pomegranate"s },
                             { 17, "plum"s },         { 18, "avocado"s },
                             { 19, "coconut"s },      { 20, "fig"s },
                             { 21, "lemon"s },        { 22, "lime"s },
                             { 23, "grapefruit"s },   { 24, "passionfruit"s },
                             { 25, "dragonfruit"s },  { 26, "guava"s },
                             { 27, "apricot"s },      { 28, "lychee"s },
                             { 29, "cantaloupe"s },   { 30, "blackberry"s },
                             { 31, "persimmon"s },    { 32, "nectarine"s },
                             { 33, "cranberry"s },    { 34, "mulberry"s },
                             { 35, "gooseberry"s },   { 36, "tangerine"s },
                             { 37, "durian"s },       { 38, "jackfruit"s },
                             { 39, "starfruit"s },    { 40, "custard apple"s },
                             { 41, "acerola"s },      { 42, "rosehip"s },
                             { 43, "elderberry"s },   { 44, "boysenberry"s },
                             { 45, "blackcurrant"s }, { 46, "redcurrant"s },
                             { 47, "blackthorn"s },   { 48, "mangosteen"s },
                             { 49, "chokeberry"s },   { 50, "cloudberry"s },
                             { 51, "jujube"s },       { 52, "physalis"s },
                             { 53, "tamarillo"s }
                           };

   std::map<int, std::string> mapTest = CreateTestData_Map();
   std::vector<test_type> vecTest     = CreateTestData_Vec();

   std::ranges::sort(vecTest, [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

   //for (auto const& val : own::serialize(std::span(arrayTest)) | own::views::second) std::cout << val << '\n';
   //for (auto const& val : own::serialize(mapTest) | own::views::second) std::cout << val << '\n';
   for (auto const& val : test_data() | own::views::second) std::cout << val << '\n';

   std::vector<long long> test;
   std::ranges::copy(fibunacci() | std::views::take(30) | own::views::odd, std::back_inserter(test));
   std::ranges::sort(test, [](auto const& lhs, auto const& rhs) { return rhs < lhs; });
   for (auto f : test) {
      std::cout << f << '\n';
      }

   }

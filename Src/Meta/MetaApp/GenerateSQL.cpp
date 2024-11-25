/** \file
 * \brief implementation of the generator of sql code from adecc Scholar metadata management
 * \details this file is part of the implementations of the metadata management in the project
            adecc Scholar and containes the functions to generate the sql code with the dictionary
 * \details this is a spin-off from the previous DataDictionary.cpp file and implement parts of
            the classes TMyTable and TMyDictionary
   \version 1.0
   \since Version 1.0
   \authors Volker Hillmann (VH)
   \date 10.02.2024 VH extracted from the previous file DataDictionary.cpp
   \copyright copyright &copy; 2024. All rights reserved.
   This project is released under the MIT License.
*/


#include "GenerateSQL.h"

#include "DataDictionary.h"
#include "DictionaryHelper.h"
#include "MyProperty.h"

#include <fstream>
#include <sstream>
#include <set>
#include <array>
#include <algorithm>
#include <exception>
#include <functional>
#include <filesystem>
#include <variant>
#include <format>
#include <ranges>

namespace fs = std::filesystem;   
using namespace std::string_literals;



// -------------------------------------------------------------------------------------------------
// sql statements
// -------------------------------------------------------------------------------------------------

constexpr std::string Generator_SQL::get_name_for_table(TMyTable const& table) const {
   return table.Name();
   }


Generator_SQL const& Generator_SQL::WriteCreateTable(TMyTable const& table, fs::path file) const {
   std::ofstream ofs(file);
   if (!ofs) throw std::runtime_error(std::format("error while writing create statements for table {} in file {}.",
                                      table.FullyQualifiedSQLName(), file.string()));
   else [[likely]] return WriteCreateTable(table, ofs);
   }


Generator_SQL const& Generator_SQL::WriteCreateTable(TMyTable const& table, std::ostream &os) const {
   if (auto statements = CreateTable_Statements(table); statements.size() > 0) [[likely]] {
      os << "-- statement to create the table " << table.FullyQualifiedSQLName() << '\n';
      std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
      os << '\n';
      return *this;
      }
   else throw std::runtime_error(std::format("error while writing for table {}, no statements", 
                                 table.FullyQualifiedSQLName()));
   }


Generator_SQL const& Generator_SQL::WriteCreateView(TMyTable const& table, std::ostream& os) const {
   os << "-- statement to create the view " << table.FullyQualifiedSQLName() << '\n';
   auto statements = CreateView_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteAlterTable(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- alter table for table ... 
   auto statements = AlterTable_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if(statements.size() > 0) os << '\n';
   return *this;
   }


Generator_SQL const& Generator_SQL::WritePrimaryKey(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- create primary key for table ... 
   auto statements = PrimaryKey_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteForeignKeys(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- create foreign keys for table ... 
   auto statements = ForeignKeys_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteUniqueKeys(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- create unique keys for table ... 
   auto statements = UniqueKeys_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteCreateIndices(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- create indices for table ... 
   auto statements = Indices_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteCreateCheckConditions(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- create check conditions for table ...
   auto statements = CheckConditions_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteRangeValues(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- insert range values for table ...
   auto statements = RangeValues_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

/// \notes post conditions are complete and when needed is a finishing ";" in the statement
/// \notes there can be a go before needed
Generator_SQL const& Generator_SQL::WriteCreatePostConditions(TMyTable const& table, std::ostream& os) const {
   // possibly add a comment -- insert range values for table ...
   auto statements = PostConditions_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << "\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteCreateCleaning(TMyTable const& table, std::ostream& os) const {
   // possible comment 
   auto statements = Cleaning_Statements(table);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   if (statements.size() > 0) os << '\n';
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteDescriptions(TMyTable const& table, std::ostream& os) const {
   return *this;
   }






std::string Generator_SQL::CreateTable_SQLRow(TMyTable const& table, TMyAttribute const& attr, size_t len) const {
   auto const& datatype = table.Dictionary().FindDataType(attr.DataType());
   std::ostringstream os;
   if (!attr.IsComputed()) [[likely]] {
      os << std::format("   {:<{}} {}", attr.DBName(), len, datatype.DatabaseType());
      if (datatype.UseLen()) {
         os << "(" << attr.Len();
         if (datatype.UseScale()) os << ", " << attr.Scale() << ")";
         else os << ")";
      }
      if (attr.NotNull()) os << " NOT NULL";
      if (datatype.CheckSeq().size() > 0) {
         os << " CHECK (" << attr.DBName() << " " << datatype.CheckSeq();
         if (attr.CheckSeq().size() > 0 && attr.CheckAtTable() != EMyCheckKinds::table)
            os << " AND " << attr.DBName() << " " << attr.CheckSeq() << ")";
         else os << ")";
      }
      else {
         if (attr.CheckSeq().size() > 0 && attr.CheckAtTable() != EMyCheckKinds::table)
            os << " CHECK (" << attr.DBName() << " " << attr.CheckSeq() << ")";
      }
   }
   else {
      // only for EMyCalculationKinds::attribute and EMyCalculationKinds::attribute_permanent
      if (attr.KindOfCalulate() == EMyCalculationKinds::attribute || attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent) {
         os << std::format("   {:<{}}", attr.DBName(), len) << " AS " << attr.Computed();
         if (attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent) os << "  PERSISTED";
      }
   }
   return os.str();
   }


std::string  Generator_SQL::SQLRow(TMyTable const& table, TMyReferences const& reference) const {
   std::string strSQL;
   std::ostringstream sList1, sList2;

   TMyTable const& ref = reference.Table().Dictionary().FindTable(reference.RefTable());

   sList1 << "(";
   sList2 << "(";
   int i = 0;
   std::ranges::for_each(reference.Values(), [&reference, &ref, &sList1, &sList2, &i](auto const& row) {
                       sList1 << (i > 0 ? ", " : "") << reference.Table().FindAttribute(row.first).DBName();
                       sList2 << (i++ > 0 ? ", " : "") << ref.FindAttribute(row.second).DBName();
                       });
   sList1 << ")";
   sList2 << ")";

   strSQL = "ALTER TABLE "s + reference.Table().FullyQualifiedSQLName() + " ADD CONSTRAINT ref"s + reference.Name() +
            " FOREIGN KEY "s + sList1.str() +
            " REFERENCES "s + ref.FullyQualifiedSQLName() + " "s + sList2.str();
   return strSQL;
   }


std::string Generator_SQL::SQLRow(TMyTable const& table, TMyIndices const& idx) const {
   std::ostringstream sSQL;
   if (idx.IndexType() == EMyIndexType::key) {
      sSQL << "ALTER TABLE " << idx.Table().FullyQualifiedSQLName() << " ADD CONSTRAINT uk" << idx.Name()
         << " UNIQUE (";
      unsigned int i = 0;
      std::ranges::for_each(idx.Values(), [&table, &sSQL, &i](auto const& row) {
         sSQL << (i++ > 0 ? ", " : "") << table.FindAttribute(row.first).DBName(); });
      sSQL << ")";
      }
   else {
      sSQL << "CREATE";
      switch (idx.IndexType()) {
         case EMyIndexType::unique:       sSQL << " UNIQUE";       break;
         case EMyIndexType::clustered:    sSQL << " CLUSTERED";    break;
         case EMyIndexType::nonclustered: sSQL << " NONCLUSTERED"; break;
          }
      sSQL << " INDEX idx"s + idx.Name() + " ON " << table.FullyQualifiedSQLName() << " (";
      unsigned int i = 0;
      std::ranges::for_each(idx.Values(), [&idx, this, &sSQL, &i](auto const& row) {
         sSQL << (i++ > 0 ? ", " : "") << idx.Table().FindAttribute(row.first).DBName(); });
      sSQL << ")";
      }
   return sSQL.str();
   }




myStatements Generator_SQL::CreateTable_Statements(TMyTable const& table) const {

   myStatements ret;
   std::ostringstream os;

   if(table.EntityType() != EMyEntityType::view) {
      os << "CREATE TABLE " << table.FullyQualifiedSQLName() << " (\n";

      auto attributes = table.Attributes() | std::views::filter([](auto const& attr) {
                              return !attr.IsComputed() || attr.KindOfCalulate() == EMyCalculationKinds::attribute ||
                                      attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent; } );
      auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != attributes.end()  ) {
         auto len = maxLength->DBName().size();
         for (auto const& attribute : attributes | std::ranges::views::take(table.Attributes().size() - 1)) {
            os << CreateTable_SQLRow(table, attribute, len) << ",\n";
            }
         os << CreateTable_SQLRow(table, table.Attributes().back(), len) << '\n';
         }
      os << "   )";
      }
   ret.emplace_back(os.str());
   return ret;
   }

myStatements Generator_SQL::AlterTable_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else {
      auto attributes = table.Attributes() | std::views::filter([](auto const& attr) {
         return attr.IsComputed() && (attr.KindOfCalulate() == EMyCalculationKinds::table || attr.KindOfCalulate() == EMyCalculationKinds::table_permanent);
         });
      auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != attributes.end()) {
         auto len = maxLength->DBName().size();

         return attributes | std::views::transform([&table](auto const& attr) {
            return std::format("ALTER TABLE {} ADD {} AS {}{}", table.FullyQualifiedSQLName(), attr.DBName(), attr.Computed(),
               (attr.KindOfCalulate() == EMyCalculationKinds::table_permanent ? " PERSISTED" : "")); })
               | std::ranges::to<std::vector<std::string>>();
         }
      else return { };
      }
   }


myStatements Generator_SQL::PrimaryKey_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else {
      myStatements ret;
      std::ostringstream os;
      os << "ALTER TABLE " << table.FullyQualifiedSQLName() << " ADD CONSTRAINT pk" << table.SQLName() << " PRIMARY KEY (";
      size_t i = 0;
      auto key_attr = table.Attributes() | std::views::filter([](auto const& a) { return a.Primary() == true; });
      std::ranges::for_each(key_attr, [&os, &i](auto const& a) { os << (i++ > 0 ? ", " : "") << a.DBName();  });
      if (i > 0) {
         os << ")";
         ret.emplace_back(os.str());
         }
      return ret;
      }
   }

myStatements Generator_SQL::ForeignKeys_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else return table.References() | std::views::transform([&table, this](auto const& ref) { return SQLRow(table, ref); }) 
                                  | std::ranges::to<std::vector<std::string>>();
   }

myStatements Generator_SQL::UniqueKeys_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else return table.Indices() | own::views::is_unique_key // std::views::filter([](auto const& i) { return i.IndexType() == EMyIndexType::key; })
                               | std::views::transform([&table, this](auto const& i) { return SQLRow(table, i); })
                               | std::ranges::to<std::vector<std::string>>();
   }

myStatements Generator_SQL::CheckConditions_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else return table.Attributes() | std::views::transform([&table](auto const& attr) {
                                           return std::make_pair(attr, table.Dictionary().FindDataType(attr.DataType())); })
                                  | std::views::filter([](auto const& data) {
                                           return (data.first.CheckSeq().size() > 0 && 
                                                   data.first.CheckAtTable() == EMyCheckKinds::table); })
                                  | std::views::transform([&table](auto const& data) {
                                          auto Exists = [](auto const& val) { return val.CheckSeq().size() > 0; };
                                          std::ostringstream os;
                                          os << std::format("ALTER TABLE {} ADD CONSTRAINT ck_{}_{} CHECK ({})", 
                                                            table.FullyQualifiedSQLName(), table.SQLName(), data.first.Name(), 
                                                            data.first.CheckSeq());
                                          return os.str();
                                          })
                                  | std::ranges::to<std::vector<std::string>>();
   }

myStatements Generator_SQL::Indices_Statements(TMyTable const& table) const {
   if (table.EntityType() == EMyEntityType::view) return { };
   else return table.Indices() | own::views::is_index //  | std::views::filter([](auto const& i) { return i.IndexType() != EMyIndexType::key; })
                               | std::views::transform([&table, this](auto const& i) { return SQLRow(table, i); })
                               | std::ranges::to<std::vector<std::string>>();
   }

myStatements Generator_SQL::RangeValues_Statements(TMyTable const& table) const {
   if (table.EntityType() != EMyEntityType::view) [[likely]] return table.RangeValues();
   else return { };
   }

myStatements Generator_SQL::PostConditions_Statements(TMyTable const& table) const {
   // in first step the Postconditions are the same as the create view, for views don't exists Postconditions
   if (table.EntityType() != EMyEntityType::view) return table.PostConditions();
   else return { };
   }

myStatements Generator_SQL::Cleaning_Statements(TMyTable const& table) const {
   return table.Cleanings();
   }


myStatements Generator_SQL::CreateView_Statements(TMyTable const& table) const {
   // in first step the Postconditions are the same as the create view
   if (table.EntityType() != EMyEntityType::view) return { };
   else return table.PostConditions();
   }


// =====================================================================================================================
//  NEW DEFINITION FOR FUNCTIONS WHICH CREATE SQL STATEMENTS
// =====================================================================================================================

using attr_filter_func = std::function<bool(TMyAttribute const&)>;

enum class Attr_Mode : uint32_t { undefined, none, assign, like };

//myStatements, stream, prefix, intro, first, follow, separator, Abschluss, länge, zuweisung, function
//                                0                1                 2            3            4            5        
using part_data = std::tuple<myStatements, std::ostringstream, std::string, std::string, std::string, std::string, 
//                                6           7         8        9            10
                             std::string, std::string, size_t, Attr_Mode, attr_filter_func>;

auto part_detail = [](attr_filter_func f, int l, Attr_Mode m, std::string&& p1, std::string&& p2, std::string&& p3, std::string&& p4, std::string&& p5, std::string&& p6) {
   return part_data{ myStatements {}, std::ostringstream {}, p1, p2, p3, p4, p5, p6, (l < 0 ? 0u : static_cast<size_t>(l)), m, f };
   };


using myAttrWithParam = std::vector<std::tuple<TMyAttribute, std::string>>;

template <typename ty>
concept AttributeValue = requires(ty t) {
   requires std::is_same_v<ty, TMyAttribute> ||
            std::is_same_v<ty, std::tuple<TMyAttribute, std::string>>;
   };

template<typename ty>
concept AttributeOnly = requires(ty t) {
   typename ty::value_type;
   requires  std::is_same_v<typename ty::value_type, TMyAttribute>;
   };

template<typename ty>
concept AttributeContainer = requires(ty t) {
   typename ty::value_type;  
   requires  std::is_same_v<typename ty::value_type, TMyAttribute> ||
            (std::is_same_v<std::tuple_element_t<0, typename ty::value_type>, TMyAttribute> &&
             std::is_convertible_v<std::tuple_element_t<1, typename ty::value_type>, std::string>);
   };

template <size_t SIZE>
myStatements CreateStatement(std::array<part_data, SIZE>& parts, AttributeContainer auto const& all_attributes) {
   static_assert(SIZE != 0, "invalid specification of SIZE");

   using attr_only_func = std::function<std::string(std::string const&/*prefix*/, std::string const& /*attribute*/, size_t /*width*/)>;
   using attr_para_func = std::function<std::string(std::string const&/*prefix*/, std::string const& /*attribute*/, std::string const& /*paramter*/, size_t /*width*/)>;
   using attr_func = attr_para_func; // std::variant<attr_only_func, attr_para_func>;
   
   static std::map<Attr_Mode, attr_func> attr_rules = {
         { Attr_Mode::undefined, [](std::string const&, std::string const&, std::string const& , size_t) -> std::string {
                                                 throw std::runtime_error("undefined rule for attribute line");
                                                 }
         },
         { Attr_Mode::none,      [](std::string const& prefix, std::string const& attribute, std::string const& parameter, size_t width) -> std::string {
                                                 return std::format("{0:}{1:<{2}}", prefix, attribute, width);
                                                 } 
         },
         { Attr_Mode::assign,    [](std::string const& prefix, std::string const& attribute, std::string const& parameter, size_t width) -> std::string {
                                                 return std::format("{1:<{3}} = {0:}{2:}", prefix, attribute, parameter, width);
                                                 } 
         },
         { Attr_Mode::like,      [](std::string const& prefix, std::string const& attribute, std::string const& parameter, size_t width) -> std::string {
                                                 return std::format("{1:<{3}} LIKE {0:}{2:}", prefix, attribute, parameter, width);
                                                 } 
         }
      };

   if (all_attributes.size() == 0) [[unlikely]] return { };
   else {
      for (auto& part : parts) {
         auto attributes = [&all_attributes, &part]() {
               using used_type = std::remove_cvref_t<decltype(all_attributes)>;
               if constexpr (AttributeOnly<used_type>) {
                  return all_attributes | std::views::filter([&part](auto const& a) { return std::get<10>(part)(a); })
                                        | std::ranges::to<std::vector>();
                  }
               else {
                  return all_attributes | std::views::filter([&part](auto const& a) { return std::get<10>(part)(std::get<0>(a)); })
                                        | std::ranges::to<std::vector>();
                  }
               }();

         auto stmts  = Property([&part]() -> myStatements&       { return std::get<0>(part);  });
         auto stream = Property([&part]() -> std::ostringstream& { return std::get<1>(part);  });
         auto empty  = Property([&part]() -> bool                { return std::get<1>(part).str().size() == 0;  });
         auto get    = Property([&part]() -> std::string         { std::string result = std::move(std::get<1>(part).str());
                                                                   std::get<1>(part).str("");
                                                                   return result; });
         auto intro  = Property([&part]() -> std::string const&  { return std::get<3>(part);  });

         size_t test = intro().size();
         if (intro().size() > 0) stmts += std::string(intro);
      
         if (attributes.size() > 0) {
            auto prefix    = [&part]() -> std::string const& { return std::get<2>(part);  };
            auto first     = [&part]() -> std::string const& { return std::get<4>(part);  };
            auto follow    = [&part]() -> std::string const& { return std::get<5>(part);  };
            auto sepa      = [&part]() -> std::string const& { return std::get<6>(part);  };
            auto closer    = [&part]() -> std::string const& { return std::get<7>(part);  };
            auto to_long   = [&part]() -> bool               { return std::get<1>(part).str().size() > std::get<8>(part);  };
            auto assign    = [&part]() -> Attr_Mode          { return std::get<9>(part);  };

            auto rule_type = [assign](AttributeValue auto const& attr) -> Attr_Mode {
               using used_type = std::remove_cvref_t<decltype(attr)>;
               if constexpr (std::is_same_v<used_type, TMyAttribute>)
                  return assign() != Attr_Mode::like ? assign() : (attr.GetDataType().WithLike() ? Attr_Mode::like : Attr_Mode::assign);
               else
                  return assign() != Attr_Mode::like ? assign() : (std::get<0>(attr).GetDataType().WithLike() ? Attr_Mode::like : Attr_Mode::assign);
               };

            static constexpr auto attr_param_name_impl = [](auto const& para, bool return_dbname) {
               using value_type = std::remove_cvref_t<decltype(para)>;

               if constexpr (std::is_same_v<value_type, TMyAttribute>) {
                  return para.DBName();
                  }
               else if constexpr (std::is_same_v<value_type, std::tuple<TMyAttribute, std::string>>) {
                  return return_dbname ? std::get<0>(para).DBName() : std::get<1>(para);
                  } 
               else static_assert(always_false<std::decay_t<decltype(para)>>, "unexpected variant for method");
               };

            static constexpr auto attr_name = [](auto const& para) {
               return attr_param_name_impl(para, true);
               };

            static constexpr auto param_name = [](auto const& para) {
               return attr_param_name_impl(para, false);
               };

            auto maxLengthElement = [&attributes, &to_long, &assign]() {
                          if (to_long() == 0 && assign() != Attr_Mode::none)
                             return std::ranges::max_element(attributes, [](auto const& a, auto const& b) {
                                                                              return attr_name(a).size() < attr_name(b).size();
                                                                              });
                          else return attributes.end();
                          }();

            auto maxLengthAttr = [&attributes, &maxLengthElement]() -> size_t {
                          if (maxLengthElement != attributes.end()) return attr_name(*maxLengthElement).size();
                          else                        return 0u;      
                          }();

            // reducing the scope for first attribute
            {
               auto attr = *std::begin(attributes);
               stream << std::format("{}{}", first(), attr_rules[rule_type(attr)](prefix(), attr_name(attr), param_name(attr), maxLengthAttr));
            }

            if (attributes.size() > 1) [[likely]] {
               for (auto const& attr : attributes | std::views::drop(1) | std::views::take(attributes.size() - 2)) {
                  if (to_long()) {
                     stream << sepa();
                     stmts += get;
                     stream << std::format("{}{}", follow(), attr_rules[rule_type(attr)](prefix(), attr_name(attr), param_name(attr), maxLengthAttr));
                     }
                  else [[likely]] {
                     stream << std::format("{}{}", sepa(), attr_rules[rule_type(attr)](prefix(), attr_name(attr), param_name(attr), maxLengthAttr));
                     }
                  }

               auto attr = attributes.back();
               if (to_long()) {
                  stream << sepa();
                  stmts += get;
                  stream << std::format("{}{}{}", follow(), attr_rules[rule_type(attr)](prefix(), attr_name(attr), param_name(attr), maxLengthAttr), closer());
                  }
               else {
                  stream << std::format("{}{}{}", sepa(), attr_rules[rule_type(attr)](prefix(), attr_name(attributes.back()), param_name(attributes.back()), maxLengthAttr), closer());
                  }
               }
            }
         if(!empty) stmts += get;
         }   
      if constexpr (SIZE > 1) {
         for(size_t step = 1; step < SIZE; ++step)  std::get<0>(parts[0]) += std::get<0>(parts[step]);
         }
      return std::get<0>(parts[0]);
      }
   }


myStatements Generator_SQL::CreateSelectAll_Statement(TMyTable const& table) const {
   auto GetAll  = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };

   std::array<part_data, 2> parts{
          part_detail(GetAll,  60, Attr_Mode::none, ""s,  ""s, "SELECT "s,  "       "s, ","s, ""s),
          part_detail(GetNone, 60, Attr_Mode::none, ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ","s, ""s)
      };

   return CreateStatement(parts, table.Attributes());
   }

myStatements Generator_SQL::CreateSelectPrim_Statement(TMyTable const& table) const {
   auto GetAll  = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetPrim = [](TMyAttribute const& a) -> bool { return a.Primary();  };

   std::array<part_data, 3> parts{
        part_detail(GetAll,  60, Attr_Mode::none,   ""s,  ""s, "SELECT "s,  "       "s, ","s, ""s),
        part_detail(GetNone, 60, Attr_Mode::none,   ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetPrim,  0, Attr_Mode::assign, ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
      };

   return CreateStatement(parts, table.Attributes());
   }

myStatements Generator_SQL::CreateSelectUniqueKey_Statement(TMyTable const& table, TMyIndices const& idx) const {
   auto vals = idx.Values() | own::views::first | std::ranges::to<std::vector>();
   auto idx_lst = std::views::zip(std::views::iota(size_t { 1 }), table.Attributes()) 
                                  | std::views::filter([&vals](auto const& p) {
                                         return std::ranges::find(vals, std::get<0>(p)) != vals.end(); 
                                         })
                                  | own::views::second 
                                  | std::ranges::to<std::vector>();

   auto GetAll  = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetIdx  = [&idx_lst](TMyAttribute const& a) -> bool { return std::ranges::find_if(idx_lst, [&a](auto const& c) { 
                                                                             return a.Name() == c.Name();
                                                                             }) != idx_lst.end();  };

   std::array<part_data, 3> parts{
        part_detail(GetAll,  60, Attr_Mode::none,    ""s,  ""s, "SELECT "s,  "       "s, ","s, ""s),
        part_detail(GetNone, 60, Attr_Mode::none,    ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetIdx,   0, Attr_Mode::assign,  ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
   };

   return CreateStatement(parts, table.Attributes());
}


myStatements Generator_SQL::CreateSelectIndex_Statement(TMyTable const& table, TMyIndices const& idx) const {
   auto vals = idx.Values() | own::views::first | std::ranges::to<std::vector>();
   auto idx_lst = //std::views::zip(std::views::iota(size_t{ 1 }), table.Attributes())
                   table.Attributes() | std::views::transform([](auto const& p) {
                                  return std::make_tuple(p.ID(), p); })
                   | std::views::filter([&vals](auto const& p) {
                             return std::ranges::find(vals, std::get<0>(p)) != vals.end();
                             })
                   | own::views::second
                   | std::ranges::to<std::vector>();

   auto GetAll = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetIdx = [&idx_lst](TMyAttribute const& a) -> bool { return std::ranges::find_if(idx_lst, [&a](auto const& c) {
      return a.Name() == c.Name();
      }) != idx_lst.end();  };

   std::array<part_data, 3> parts{
        part_detail(GetAll,  60, Attr_Mode::none,    ""s,  ""s, "SELECT "s,  "       "s, ","s, ""s),
        part_detail(GetNone, 60, Attr_Mode::none,    ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetIdx,   0, Attr_Mode::like,   ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
      };

   return CreateStatement(parts, table.Attributes());
   }

myStatements Generator_SQL::CreateSelectReference_Statement(TMyTable const& table, TMyReferences const& ref) const {
   auto const& refVals = ref.Values();
   auto const& refAttr = table.Dictionary().FindTable(ref.RefTable()).Attributes();
   
   auto used_attributes = table.Attributes() 
                          | std::views::transform([&ref, &refVals, refAttr](auto a) -> std::tuple<TMyAttribute, std::string> {
                               if(auto it = std::ranges::find_if(refVals, [a](auto p) { return std::get<0>(p) == a.ID(); }); it != refVals.end()) {
                                  if (auto it1 = std::ranges::find_if(refAttr, [&ref, &a, &it](auto ra) { return ra.ID() == std::get<1>(*it); }); it1 != refAttr.end()) {
                                     return std::make_tuple(a, it1->DBName());
                                     }
                                   else {
                                      throw std::runtime_error("unexpected combination in reference in ref "s + ref.Name());
                                      }
                                   }
                               else {
                                  return std::make_tuple(a, ""s);
                                  }
                               })
                          | std::ranges::to<std::vector>();

   auto vals = refVals | own::views::first | std::ranges::to<std::vector>();

   auto GetAll = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };

   auto GetRef = [&vals](TMyAttribute const& a) -> bool { return std::ranges::find_if(vals, [&a](auto const& c) {
      return a.ID() == c;
      }) != vals.end();  };



   std::array<part_data, 3> parts{
        part_detail(GetAll,  60, Attr_Mode::none,    ""s,  ""s, "SELECT "s,  "       "s, ", "s, ""s),
        part_detail(GetNone, 60, Attr_Mode::none,    ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetRef,   0, Attr_Mode::assign,   ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
      };

   return CreateStatement(parts, used_attributes); 
   }

myStatements Generator_SQL::CreateSelectRevReference_Statement(TMyTable const& table, TMyReferences const& ref) const {
   auto refVals = ref.Values() | std::views::transform([](auto p) {
           return std::make_pair(std::get<1>(p), std::get<0>(p));
           });
   auto const& refAttr = ref.Table().Attributes();

   auto used_attributes = table.Attributes()
                          | std::views::transform([&ref, &refVals, refAttr](auto a) -> std::tuple<TMyAttribute, std::string> {
                             if(auto it = std::ranges::find_if(refVals, [a](auto p) { return std::get<0>(p) == a.ID(); }); it != refVals.end()) {
                                if (auto it1 = std::ranges::find_if(refAttr, [&ref, &a, &it](auto ra) { return ra.ID() == std::get<1>(*it); }); it1 != refAttr.end()) {
                                   return std::make_tuple(a, it1->DBName());
                                   }
                                else {
                                   throw std::runtime_error("unexpected combination in reference in ref "s + ref.Name());
                                   }
                                }
                             else {
                                return std::make_tuple(a, ""s);
                                }
                             })
                          | std::ranges::to<std::vector>();

   auto vals = refVals | own::views::first | std::ranges::to<std::vector>();


   auto GetAll = [](TMyAttribute const& a) -> bool { return true;  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetRef = [&vals](TMyAttribute const& a) -> bool { return std::ranges::find_if(vals, [&a](auto const& c) {
         return a.ID() == c;
         }) != vals.end();  };


   std::array<part_data, 3> parts {
        part_detail(GetAll,  60, Attr_Mode::none,    ""s,  ""s, "SELECT "s,  "       "s, ", "s, ""s),
        part_detail(GetNone, 60, Attr_Mode::none,    ""s,  std::format("FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetRef,   0, Attr_Mode::like,   ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
      };

   return CreateStatement(parts, used_attributes);
}


// --------------------------------------------------------------------------------------------------------------------
// create the insert statement for a table 
// --------------------------------------------------------------------------------------------------------------------

myStatements Generator_SQL::CreateInsert_Statement(TMyTable const& table) const {
   auto GetAll = [](TMyAttribute const& a) -> bool { return !a.IsComputed();  };
   std::array<part_data, 2> parts {
          part_detail (GetAll, 60, Attr_Mode::none, ""s,  std::format("INSERT INTO {} ", table.FullyQualifiedSQLName()), "       ("s,  "        "s, ","s, ")"s ),
          part_detail (GetAll, 60, Attr_Mode::none, ":"s, ""s,                                                           "VALUES ("s,  "        "s, ","s, ")"s )
          };

   return CreateStatement(parts, table.Attributes());
   }


myStatements Generator_SQL::CreateUpdateAll_Statement(TMyTable const& table) const {
   auto GetAll = [](TMyAttribute const& a) -> bool { return !a.IsComputed();  };
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetPrim = [](TMyAttribute const& a) -> bool { return a.Primary();  };

   std::array<part_data, 3> parts{
        part_detail(GetNone,  0, Attr_Mode::none,    ""s,  std::format("UPDATE {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetAll,   0, Attr_Mode::assign,  ":"s,  ""s, "SET   "s, "      "s,  ","s, ""s),
        part_detail(GetPrim,  0, Attr_Mode::assign,  ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ")"s)
        };

   return CreateStatement(parts, table.Attributes());
   }

myStatements Generator_SQL::CreateUpdateWithoutPrim_Statement(TMyTable const& table) const {
   auto GetNoPrim = [](TMyAttribute const& a) -> bool { return !a.Primary() && !a.IsComputed();  };
   auto GetNone   = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetPrim   = [](TMyAttribute const& a) -> bool { return a.Primary();  };

   std::array<part_data, 3> parts{
        part_detail(GetNone,   0, Attr_Mode::none,    ""s,  std::format("UPDATE {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
        part_detail(GetNoPrim, 0, Attr_Mode::assign,  ":"s,  ""s, "SET   "s, "      "s,  ","s, ""s),
        part_detail(GetPrim,   0, Attr_Mode::assign,  ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ")"s)
        };

   return CreateStatement(parts, table.Attributes());
   }


myStatements Generator_SQL::CreateDeleteAll_Statement(TMyTable const& table) const {
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   std::array<part_data, 1> parts{
        part_detail(GetNone, 60, Attr_Mode::none, ""s,  std::format("DELETE FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s)
        };

   return CreateStatement(parts, table.Attributes());
   }


myStatements Generator_SQL::CreateDeletePrim_Statement(TMyTable const& table) const {
   auto GetNone = [](TMyAttribute const& a) -> bool { return false;  };
   auto GetPrim = [](TMyAttribute const& a) -> bool { return a.Primary();  };

   std::array<part_data, 2> parts{
       part_detail(GetNone, 60, Attr_Mode::none,    ""s,  std::format("DELETE FROM {}", table.FullyQualifiedSQLName()), ""s, ""s,  ""s, ""s),
       part_detail(GetPrim,  0, Attr_Mode::assign,  ":key"s,  ""s, "WHERE "s, "      "s,  " AND"s, ""s)
      };

   return CreateStatement(parts, table.Attributes());
   }



// =============================================================================================================
 
Generator_SQL const& Generator_SQL::WriteSQLDocumentation(std::ostream& os) const {
   static auto constexpr Clean = [](std::string const& strValue) {
      std::string strRetVal = "";
      std::ranges::copy_if(strValue, std::back_inserter(strRetVal), [](char letter) { return letter != '\''; });
      return strRetVal;
      };

   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to add descriptions for the project " << Dictionary().Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Dictionary().Author() << '\n'
      << " * copyright © "   << Dictionary().Copyright() << '\n'
      << " * ------------------------------------------------------------------------------------ */\n\n";

   // check the table, drop descriptions when they already exist to override them
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) {
      auto const& schema_name = table.SQLSchema();
      auto const& table_name = table.SQLName();
          
      os << "IF EXISTS (SELECT 1 FROM fn_listextendedproperty(N'MS_Description', N'schema', N'" << schema_name << "', "
         <<                                                  "N'table', N'" << table_name << "', NULL, NULL))\n"
         << "BEGIN\n";
      os << "   EXEC sys.sp_dropextendedproperty @name = N'MS_Description', \n"
         << "                                    @level0type=N'SCHEMA', @level0name = N'" << schema_name << "', \n"
         << "                                    @level1type = N'TABLE', @level1name = '" << table_name << "'\n"
         << "END\n";
      os << '\n';
 
      os << "EXEC sys.sp_addextendedproperty @name = N'MS_Description', \n"
         << "              @value = N'" << Clean(table.Denotation()) << "', \n" 
         << "              @level0type=N'SCHEMA', @level0name = N'" << schema_name <<  "', \n" 
         << "              @level1type = N'TABLE', @level1name = '" << table_name << "'\n";
       os << "\n";

       for(auto const& attr : table.Attributes()) {
           auto const& column_name = attr.DBName();

          os << "IF EXISTS(SELECT 1 FROM fn_listextendedproperty(N'MS_Description', N'schema', '" << schema_name << "',\n"
             << "                                                N'table', '" << table_name << "',\n"
             << "                                                N'column', '" << column_name << "'))\n"
             << "BEGIN\n"
             << "   EXEC sys.sp_dropextendedproperty @name = N'MS_Description',\n"
             << "                                   @level0type=N'SCHEMA', @level0name = N'" << schema_name << "',\n"
             << "                                   @level1type = N'TABLE', @level1name = '" << table_name << "',\n"
             << "                                   @level2type = N'COLUMN', @level2name = N'"  << column_name << "'\n"
             << "END\n"
             << "\n";

          os << "EXEC sys.sp_addextendedproperty @name = N'MS_Description', "
             << "@value = N'" << Clean(attr.Denotation()) << "', "
             << "@level0type=N'SCHEMA', @level0name = N'" << table.SQLSchema() << "', "
             << "@level1type = N'TABLE', @level1name = '" << table.SQLName() << "', "
             << "@level2type = N'COLUMN', @level2name = N'" << attr.DBName() << "'";
          os << '\n';
          }
       os << '\n';

      }

   return *this;
   }

Generator_SQL const& Generator_SQL::WriteSQLTables(std::ostream& os) const {

   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to create tables / views for the project " << Dictionary().Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Dictionary().Author() << '\n'
      << " * copyright © "   << Dictionary().Copyright() << '\n'
      << " * ------------------------------------------------------------------------------------ */\n\n"
      << "-- create tables for the project\n\n";

   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) {
      Dictionary().sql_builder()
         .WriteCreateTable(table, os)
         .WriteCreatePostConditions(table, os);
      }

   os << "-- create views for the project\n\n";
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_view) {
      Dictionary().sql_builder().WriteCreateView(table, os);
      os << '\n';
      }

   return *this;
   }


Generator_SQL const& Generator_SQL::WriteSQLAdditionals(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with keys, references and indices for the project " << Dictionary().Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Dictionary().Author() << '\n'
      << " * copyright © "   << Dictionary().Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n";

   os << "\n-- alter tables with calculated columns " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteAlterTable(table, os);

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create primary keys for tables of the dictionary " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WritePrimaryKey(table, os);
 
   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create unique keys for tables of the dictionary " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteUniqueKeys(table, os);
   
   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create foreign keys for tables of the dictionary " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteForeignKeys(table, os);

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create check conditions for tables of the dictionary " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteCreateCheckConditions(table, os);

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create indices for tables of the dictionary " << Dictionary().Name() << '\n';
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteCreateIndices(table, os);

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   return *this;
   }


Generator_SQL const& Generator_SQL::WriteSQLRangeValues(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to prefill the rangevalues for the project " << Dictionary().Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Dictionary().Author() << '\n'
      << " * copyright © "   << Dictionary().Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n";

   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      Dictionary().sql_builder().WriteRangeValues(table, os);
   
   return *this;
   }

Generator_SQL const& Generator_SQL::WriteSQLDropTables(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script to drop relationships and tables for the project " << Dictionary().Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Dictionary().Author() << '\n'
      << " * copyright © "   << Dictionary().Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n\n"
      << " -- drop foreign keys to erase dependencies\n";

   for (auto const& [_, table] : Dictionary().Tables()) {
      std::ranges::for_each(table.References(), [&os, &table](auto ref) {
            os << "ALTER TABLE "s + table.FullyQualifiedSQLName() << " DROP CONSTRAINT ref"s << ref.Name() << ";\n"s; });
      if(!table.References().empty()) os << '\n';
      }

   os << "\n-- drop all tables\n";
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_table) 
      os << std::format("DROP TABLE {};\n", table.FullyQualifiedSQLName());

   os << "\n-- drop all tables\n";
   for (auto const& [_, table] : Dictionary().Tables() | own::views::is_view)
      os << std::format("DROP VIEW {};\n", table.FullyQualifiedSQLName());

   os << "\n-- run cleanings for added informations\n";
   for (auto const& [_, table] : Dictionary().Tables()) 
      Dictionary().sql_builder().WriteCreateCleaning(table, os);

   return *this;
   }




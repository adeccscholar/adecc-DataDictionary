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


myStatements Generator_SQL::CreateSelectAll_Statement(TMyTable const& table) const {
   myStatements stmts;
   std::ostringstream os;
   auto const& firstAttr = std::begin(table.Attributes())->DBName();
   os << std::format("SELECT {0}", firstAttr);
   
   for (size_t len = firstAttr.size(); auto const& attrib : table.Attributes() | std::views::drop(1)) {
      if (len > 60) {
         os << ",";
         stmts.emplace_back(os.str());
         os.str("");
         os << "       ";
         len = attrib.DBName().size();
         }
      else {
         os << ", ";
         len += attrib.DBName().size();
         }
      os << std::format("{0}", attrib.DBName());
      }
   stmts.emplace_back(os.str());
   os.str("");
   os << "FROM " << table.FullyQualifiedSQLName();
   stmts.emplace_back(os.str());
   return stmts;
   }

myStatements Generator_SQL::CreateWherePrim_Statement(TMyTable const& table) const {
   myStatements stmts;

   auto prim_keys = table.Attributes() | own::views::primary | std::ranges::to<std::vector>();
   auto maxPrimElement = std::ranges::max_element(prim_keys, [](auto const& a, auto const& b) {
      return a.DBName().size() < b.DBName().size();
      });

   size_t maxLengthPrimAttr = 0;
   if (maxPrimElement != prim_keys.end()) {
      maxLengthPrimAttr = maxPrimElement->DBName().size();
      }

   std::ostringstream os;
   if (prim_keys.size() > 0) {
      os << std::format("WHERE {0:<{1}} = :key{0}", std::begin(prim_keys)->DBName(), maxLengthPrimAttr);
      if (prim_keys.size() == 1) {
         stmts.emplace_back(os.str());
         }
      else {
         for (auto const& attrib : prim_keys | std::views::drop(1)) {
            os << " AND";
            stmts.emplace_back(os.str());
            os.str("");
            os << std::format("      {0:<{1}} = :key{0}", attrib.DBName(), maxLengthPrimAttr);
            }
         stmts.emplace_back(os.str());
         }
      }
   return stmts;
   }

myStatements Generator_SQL::CreateSelectPrim_Statement(TMyTable const& table) const {
   auto stmts = CreateSelectAll_Statement(table); 
   stmts += CreateWherePrim_Statement(table);
   return stmts;
   }


template <bool WithPrim = true>
myStatements CreateUpdate_Statement(TMyTable const& table) {
   myStatements stmts;

   auto attributes = [&table]() {
      if constexpr (WithPrim == true) return table.Attributes() | std::ranges::to<std::vector>();
      else return table.Attributes() | own::views::non_primary | std::ranges::to<std::vector>();
         }();

   auto maxLengthElement = std::ranges::max_element(attributes, [](auto const& a, auto const& b) {
      return a.DBName().size() < b.DBName().size();
      });

   size_t maxLengthAttr = 0;
   if (maxLengthElement != attributes.end()) {
      maxLengthAttr = maxLengthElement->DBName().size();
      }

   if (attributes.size() == 0) return {};
   //stmts.emplace_back(std::format("UPDATE {}", table.FullyQualifiedSQLName()));
   stmts += std::format("UPDATE {}", table.FullyQualifiedSQLName());
   if (attributes.size() == 1) {
      stmts += std::format("SET {0:<{1}} = :{0}", std::begin(attributes)->DBName(), maxLengthAttr);
      }
   else {
      stmts += std::format("SET {0:<{1}} = :{0},", std::begin(attributes)->DBName(), maxLengthAttr);
      if (int to_take = static_cast<int>(attributes.size()) - 2; to_take > 0) {
         for (auto const& attrib : attributes | std::views::drop(1) | std::views::take(to_take)) {
            stmts += std::format("    {0:<{1}} = :{0},", attrib.DBName(), maxLengthAttr);
            }
         }
      stmts += std::format("    {0:<{1}} = :{0}", attributes.back().DBName(), maxLengthAttr);
      }
   return stmts;
   }


myStatements Generator_SQL::CreateUpdateAll_Statement(TMyTable const& table) const {
   myStatements stmts = CreateUpdate_Statement<true>(table);
   stmts += CreateWherePrim_Statement(table);
   return stmts;
   }

myStatements Generator_SQL::CreateUpdateWithoutPrim_Statement(TMyTable const& table) const {
   myStatements stmts = CreateUpdate_Statement<false>(table);
   stmts += CreateWherePrim_Statement(table);
   return stmts;
   }





myStatements Generator_SQL::CreateInsert_Statement(TMyTable const& table) const {
   if (auto const& attributes = table.Attributes(); attributes.size() == 0) [[unlikely]] return { };
   else {
      //                                0                1                 2            3            4            5        6
      using part_data = std::tuple<myStatements, std::ostringstream, std::string, std::string, std::string, std::string, size_t>;
      static auto part_detail = [](int l, std::string&& p1, std::string&& p2, std::string&& p3, std::string&& p4) {
            return part_data{ myStatements {}, std::ostringstream {}, p1, p2, p3, p4, (l < 0 ? 0u : static_cast<size_t>(l)) };
            };
      
      std::array<part_data, 2> parts {
          part_detail (60, ""s,  std::format("INSERT INTO {} (", table.FullyQualifiedSQLName()), "   "s,  "   "s ),
          part_detail (65, ":"s, "VALUES ("s,                                                    "   "s,  "   "s)
          };

      for(auto& part : parts) {
         auto stmts   = Property([&part]() -> myStatements&        { return std::get<0>(part);  });
         auto stream  = Property([&part]() -> std::ostringstream&  { return std::get<1>(part);  });
         auto get     = Property([&part]() -> std::string          { std::string result = std::move(std::get<1>(part).str());
                                                                     std::get<1>(part).str("");
                                                                     return result; });
         auto prefix  = [&part]() -> std::string const&   { return std::get<2>(part);  };
         auto intro   = [&part]() -> std::string const&   { return std::get<3>(part);  };
         auto first   = [&part]() -> std::string const&   { return std::get<4>(part);  };
         auto follow  = [&part]() -> std::string const&   { return std::get<5>(part);  };
         auto to_long = [&part]() -> bool                 { return std::get<1>(part).str().size() > std::get<6>(part);  };

         if (intro().size() > 0) stmts += intro();

         stream << std::format("{}{}{}", first(), prefix(), std::begin(attributes)->DBName());
         if(attributes.size() > 1) [[likely]] {
            for(auto const& attr : attributes | std::views::drop(1) | std::views::take(attributes.size() - 2)) {
               if(to_long()) {
                  stream << ", ";
                  stmts += get;
                  stream << std::format("{}{}{}", follow(), prefix(), attr.DBName());
                  }
               else [[likely]] {
                  stream << std::format(", {}{}", prefix(), attr.DBName());
                  }
               }
            stream << std::format(", {}{})", prefix(), attributes.back().DBName());
            }
         else {
            stream << std::format("{}{})", prefix(), attributes.back().DBName());
            }
         stmts += get;
         }

      std::get<0>(parts[0]) += std::get<0>(parts[1]);
      return std::get<0>(parts[0]);
      }
   }

myStatements Generator_SQL::CreateDeleteAll_Statement(TMyTable const& table) const {
   myStatements stmts;
   stmts.emplace_back(std::format("DELETE FROM {}", table.FullyQualifiedSQLName()));
   return stmts;
   }

myStatements Generator_SQL::CreateDeletePrim_Statement(TMyTable const& table) const {
   myStatements stmts = CreateDeleteAll_Statement(table);
   stmts += CreateWherePrim_Statement(table);
   return stmts;
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




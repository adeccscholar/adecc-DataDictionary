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


#include "DataDictionary.h"

#include <sstream>
#include <set>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;

// -------------------------------------------------------------------------------------------------
// sql statements
// -------------------------------------------------------------------------------------------------

myStatements TMyTable::Create_Table_Statements(bool boAll) const {
   auto SQLRow = [this](TMyAttribute const& attr, size_t len) {
      auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
      if (!attr.IsComputed()) [[unlikely]]
         return std::format("   {:<{}} {}{}{}", attr.DBName(), len, datatype.DatabaseType(),
            (datatype.UseLen() && datatype.UseScale() ? std::format("({}, {})", attr.Len(), attr.Scale()) :
               (datatype.UseLen() ? std::format("({})", attr.Len()) : ""s)),
            (attr.NotNull() ? " NOT NULL"s : ""s));
      else
         return std::format("   {:<{}} {}", attr.DBName(), len, " AS ("s + attr.Computed() + ")"s);
      };

   myStatements ret;
   std::ostringstream os;

   os << "CREATE TABLE " << FullyQualifiedSQLName() << " (\n";

   auto attributes = Attributes() | std::views::filter([boAll](auto const& attr) { return boAll ? true : !attr.IsComputed(); });
   auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
   if (maxLength != attributes.end()  ) {
      auto len = maxLength->DBName().size();
      for (auto const& attribute : attributes | std::ranges::views::take(Attributes().size() - 1)) {
         os << SQLRow(attribute, len) << ",\n";
         }
      os << SQLRow(Attributes().back(), len) << '\n';
      }
   os << "   )";
   ret.emplace_back(os.str());
   return ret;
   }

myStatements TMyTable::Create_Alter_Table_Statements() const {
   auto attributes = Attributes() | std::views::filter([](auto const& attr) { return attr.IsComputed(); });
   auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
   if (maxLength != attributes.end()) {
      auto len = maxLength->DBName().size();
      return attributes | std::views::transform([this](auto const& attr) {
         return std::format("ALTER TABLE {} ADD {} AS ({})", FullyQualifiedSQLName(), attr.DBName(), attr.Computed()); })
         | std::ranges::to<std::vector<std::string>>();
      }
   else return { };
   }


myStatements TMyTable::Create_Primary_Key_Statements() const {
   myStatements ret;
   std::ostringstream os;
   os << "ALTER TABLE " << FullyQualifiedSQLName() << " ADD CONSTRAINT pk_" << SQLName() << " PRIMARY KEY (";
   size_t i = 0;
   auto key_attr = Attributes() | std::views::filter([](auto const& a) { return a.Primary() == true; });
   std::ranges::for_each(key_attr, [&os, &i](auto const& a) { os << (i++ > 0 ? ", " : "") << a.DBName();  });
   if (i > 0) {
      os << ")";
      ret.emplace_back(os.str());
      }
   return ret;
   }

myStatements TMyTable::Create_Foreign_Keys_Statements() const {
   return References() | std::views::transform([](auto const& ref) { return ref.SQLRow(); }) | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Create_Unique_Keys_Statements() const {
   return Indices() | std::views::filter([](auto const& i) { return i.IndexType() == EMyIndexType::key; })
                    | std::views::transform([](auto const& i) { return i.SQLRow(); }) 
                    | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Check_Conditions_Statements() const {
   return Attributes() | std::views::transform([this](auto const& attr) {
                           return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); })
                       | std::views::filter([](auto const& data) {
                           return data.first.CheckSeq().size() > 0 || data.second.CheckSeq().size() > 0; })
                       | std::views::transform([this](auto const& data) {
                           auto Exists = [](auto const& val) { return val.CheckSeq().size() > 0; };
                           std::ostringstream os;
                           os << std::format("ALTER TABLE {} ADD CONSTRAINT ck_{}_{} CHECK (", FullyQualifiedSQLName(), SQLName(), data.first.DBName());
                           if (Exists(data.first) && Exists(data.second)) os << std::format("{0} {1} AND {0} {2}", data.first.DBName(), data.second.CheckSeq(), data.first.CheckSeq());
                           else if (Exists(data.first))  os << std::format("{0} {1}", data.first.DBName(), data.first.CheckSeq());
                           else if (Exists(data.second)) os << std::format("{0} {1}", data.first.DBName(), data.second.CheckSeq());
                           os << ")";
                           return os.str();
                           })
                       | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Create_Indices_Statements() const {
   return Indices() | std::views::filter([](auto const& i) { return i.IndexType() != EMyIndexType::key; })
                    | std::views::transform([](auto const& i) { return i.SQLRow(); }) 
                    | std::ranges::to<std::vector<std::string>>();
}

myStatements TMyTable::Create_RangeValues_Statements() const {
   return RangeValues();
   }

myStatements TMyTable::Create_PostConditions_Statements() const {
   return PostConditions();
   }

myStatements TMyTable::Create_Cleaning_Statements() const {
   return Cleanings();
   }

bool TMyTable::SQL_Create_Table(std::ostream& os, bool boAll) const {
   // possibly add a comment -- create table ... 
   auto statements = Create_Table_Statements(boAll);
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Alter_Table(std::ostream& os) const {
   auto statements = Create_Alter_Table_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
}


bool TMyTable::SQL_Create_Primary_Key(std::ostream& os) const {
   // possibly add a comment -- create primary key for table ... 
   auto statements = Create_Primary_Key_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Foreign_Keys(std::ostream& os) const {
   // possibly add a comment -- create foreign keys for table ... 
   auto statements = Create_Foreign_Keys_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Unique_Keys(std::ostream& os) const {
   // possibly add a comment -- create unique keys for table ... 
   auto statements = Create_Unique_Keys_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Indices(std::ostream& os) const {
   // possibly add a comment -- create indices for table ... 
   auto statements = Create_Indices_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Check_Conditions(std::ostream& os) const {
   // possibly add a comment -- create check conditions for table ...
   auto statements = Check_Conditions_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_RangeValues(std::ostream& os) const {
   auto statements = Create_RangeValues_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_PostConditions(std::ostream& os) const {
   auto statements = Create_PostConditions_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Cleaning(std::ostream& os) const {
   auto statements = Create_Cleaning_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
   return statements.size() > 0;
   }

bool TMyTable::SQL_Create_Descriptions(std::ostream& os) const {
   return true;
   }

/*
DECLARE @TableName NVARCHAR(255) = 'IhreTabelle'; -- Geben Sie den Tabellennamen hier ein

-- Beschreibung für die Tabelle hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für die Tabelle.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName;

-- Beschreibungen für die Spalten hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für die Spalte.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'COLUMN', @level2name=N'IhreSpalte';

-- Beschreibungen für die Primärschlüssel hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Primärschlüssel.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrPrimaryKey';

-- Beschreibungen für die Fremdschlüssel hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Fremdschlüssel.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrForeignKey';

-- Beschreibungen für Unique Keys hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Unique Key.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrUniqueKey';

-- Beschreibungen für Indizes hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Index.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'INDEX', @level2name=N'IhrIndex';

*/
 
bool TMyDictionary::Create_SQL_Tables(std::ostream& os) const {

   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to create tables for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ------------------------------------------------------------------------------------ */\n\n";

   for (auto const& [_, table] : Tables()) {
      table.SQL_Create_Table(os, false);
      os << '\n';
      if (table.SQL_Create_PostConditions(os)) os << '\n';
      }

   return true;
   }

bool TMyDictionary::Create_SQL_Additionals(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with keys, references and indices for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n";

   os << "\n-- alter tables with calculated columns " << Name() << '\n';
   for (auto const& [_, table] : Tables())  if (table.SQL_Create_Alter_Table(os)) os << '\n';

   os << "\n-- create primary keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables())  if(table.SQL_Create_Primary_Key(os)) os << '\n';
 
   os << "\n-- create unique keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Unique_Keys(os)) os << '\n';
    
   os << "\n-- create foreign keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Foreign_Keys(os)) os << '\n'; 

   os << "\n-- create check conditions for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Check_Conditions(os)) os << '\n';
   
   os << "\n-- create indices for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Indices(os)) os << '\n';

   return true;
   }


bool TMyDictionary::Create_SQL_RangeValues(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to prefill the rangevalues for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n";

   for (auto const& [_, table] : Tables()) if (table.SQL_Create_RangeValues(os)) os << '\n';
   
   return true;
   }

bool TMyDictionary::SQL_Drop_Tables(std::ostream& os) const {
   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script to drop relationships and tables for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ----------------------------------------------------------------------------------- */\n\n"
      << " -- drop foreign keys to erase dependencies\n";

   for (auto const& [_, table] : Tables()) {
      std::ranges::for_each(table.References(), [&os, &table](auto ref) {
            os << "ALTER TABLE "s + table.FullyQualifiedSQLName() << " DROP CONSTRAINT "s << ref.Name() << ";\n"s; });
      if(!table.References().empty()) os << '\n';
      }

   os << "\n-- drop all tables\n";
   for (auto const& [_, table] : Tables()) os << std::format("DROP TABLE {};\n", table.FullyQualifiedSQLName());

   os << "\n-- run cleanings for added informations\n";
   for (auto const& [_, table] : Tables()) if (table.SQL_Create_Cleaning(os)) os << '\n';

   return true;
   }




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

myStatements TMyTable::Create_Table_Statements() const {
   auto SQLRow = [this](TMyAttribute const& attr, size_t len) {
      auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
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
         if(attr.KindOfCalulate() == EMyCalculationKinds::attribute || attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent) {
            os << std::format("   {:<{}}", attr.DBName(), len) << " AS " << attr.Computed();
            if (attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent) os << "  PERSISTED";
            }
         }
       return os.str();
      };

   myStatements ret;
   std::ostringstream os;

   if(EntityType() != EMyEntityType::view) {
      os << "CREATE TABLE " << FullyQualifiedSQLName() << " (\n";

      auto attributes = Attributes() | std::views::filter([](auto const& attr) {
                              return !attr.IsComputed() || attr.KindOfCalulate() == EMyCalculationKinds::attribute ||
                                      attr.KindOfCalulate() == EMyCalculationKinds::attribute_permanent; } );
      auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != attributes.end()  ) {
         auto len = maxLength->DBName().size();
         for (auto const& attribute : attributes | std::ranges::views::take(Attributes().size() - 1)) {
            os << SQLRow(attribute, len) << ",\n";
            }
         os << SQLRow(Attributes().back(), len) << '\n';
         }
      os << "   )";
      }
   ret.emplace_back(os.str());
   return ret;
   }

myStatements TMyTable::Create_View_Statements() const {
   // in first step the Postconditions are the same as the create view
   if (EntityType() != EMyEntityType::view) return { };
   else return PostConditions();
   }


myStatements TMyTable::Create_Alter_Table_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else {
      auto attributes = Attributes() | std::views::filter([](auto const& attr) { 
            return attr.IsComputed() && (attr.KindOfCalulate() == EMyCalculationKinds::table || attr.KindOfCalulate() == EMyCalculationKinds::table_permanent); 
            });
      auto maxLength = std::ranges::max_element(attributes, [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != attributes.end()) {
         auto len = maxLength->DBName().size();

         return attributes | std::views::transform([this](auto const& attr) {
                               return std::format("ALTER TABLE {} ADD {} AS {}{}", FullyQualifiedSQLName(), attr.DBName(), attr.Computed(),
                                                                  (attr.KindOfCalulate() == EMyCalculationKinds::table_permanent ? " PERSISTED" : "")); })
                           | std::ranges::to<std::vector<std::string>>();
         }
      else return { };
      }
   }


myStatements TMyTable::Create_Primary_Key_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else {
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
   }

myStatements TMyTable::Create_Foreign_Keys_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else return References() | std::views::transform([](auto const& ref) { return ref.SQLRow(); }) | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Create_Unique_Keys_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else return Indices() | std::views::filter([](auto const& i) { return i.IndexType() == EMyIndexType::key; })
                         | std::views::transform([](auto const& i) { return i.SQLRow(); }) 
                         | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Check_Conditions_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else return Attributes() | std::views::transform([this](auto const& attr) {
                                     return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); })
                            | std::views::filter([](auto const& data) {
                                     return (data.first.CheckSeq().size() > 0 && data.first.CheckAtTable() == EMyCheckKinds::table); })
                            | std::views::transform([this](auto const& data) {
                                     auto Exists = [](auto const& val) { return val.CheckSeq().size() > 0; };
                                     std::ostringstream os;
                                     os << std::format("ALTER TABLE {} ADD CONSTRAINT ck_{}_{} CHECK ({})", FullyQualifiedSQLName(), SQLName(), data.first.Name(), data.first.CheckSeq());
                                     return os.str();
                                     })
                            | std::ranges::to<std::vector<std::string>>();
   }

myStatements TMyTable::Create_Indices_Statements() const {
   if (EntityType() == EMyEntityType::view) return { };
   else return Indices() | std::views::filter([](auto const& i) { return i.IndexType() != EMyIndexType::key; })
                         | std::views::transform([](auto const& i) { return i.SQLRow(); }) 
                         | std::ranges::to<std::vector<std::string>>();
}

myStatements TMyTable::Create_RangeValues_Statements() const {
   return RangeValues();
   }

myStatements TMyTable::Create_PostConditions_Statements() const {
   // in first step the Postconditions are the same as the create view, for views don't exists Postconditions
   if (EntityType() != EMyEntityType::view) return PostConditions();
   else return { };
   }

myStatements TMyTable::Create_Cleaning_Statements() const {
   return Cleanings();
   }

bool TMyTable::SQL_Create_Table(std::ostream& os) const {
   auto statements = Create_Table_Statements();
   if (statements.size() > 0) {
      os << "-- statement to create the table " << FullyQualifiedSQLName() << '\n';
      std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
      return true;
   }
   else return false;
   }

bool TMyTable::SQL_Create_View(std::ostream& os) const {
   auto statements = Create_View_Statements();
   if (statements.size() > 0) {
      os << "-- statement to create the view " << FullyQualifiedSQLName() << '\n';
      std::ranges::for_each(statements, [&os](auto const& p) { os << p << ";\n"; });
      return true;
      }
   else return false;
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

/// \notes post conditions are complete and when needed is a finishing ";" in the statement
/// \notes there can be a go before needed
bool TMyTable::SQL_Create_PostConditions(std::ostream& os) const {
   auto statements = Create_PostConditions_Statements();
   std::ranges::for_each(statements, [&os](auto const& p) { os << p << "\n"; });
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

-- Beschreibungen für die Primärschlüssel hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Primärschlüssel.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrPrimaryKey';

-- Beschreibungen für die Fremdschlüssel hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Fremdschlüssel.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrForeignKey';

-- Beschreibungen für Unique Keys hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Unique Key.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'CONSTRAINT', @level2name=N'IhrUniqueKey';

-- Beschreibungen für Indizes hinzufügen
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Dies ist eine Beschreibung für den Index.', @level0type=N'SCHEMA', @level0name=N'dbo', @level1type=N'TABLE', @level1name=@TableName, @level2type=N'INDEX', @level2name=N'IhrIndex';

*/
 
bool TMyDictionary::Create_SQL_Documentation(std::ostream& os) const {
   static auto constexpr Clean = [](std::string const& strValue) {
      std::string strRetVal = "";
      std::ranges::copy_if(strValue, std::back_inserter(strRetVal), [](char letter) { return letter != '\''; });
      return strRetVal;
      };

   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to add descriptions for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ------------------------------------------------------------------------------------ */\n\n";

   // check the table, drop descriptions when they already exist to override them
   for (auto const& [_, table] : Tables() | std::views::filter([](auto const& tab) { return std::get<1>(tab).EntityType() != EMyEntityType::view; })) {
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

   return true;
   }

bool TMyDictionary::Create_SQL_Tables(std::ostream& os) const {

   os << "/* ------------------------------------------------------------------------------------\n"
      << " * script with statements to create tables / views for the project " << Name() << '\n'
      << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
      << " * author:       " << Author() << '\n'
      << " * copyright © " << Copyright() << '\n'
      << " * ------------------------------------------------------------------------------------ */\n\n"
      << "-- create tables for the project\n\n";

   for (auto const& [_, table] : Tables() | std::views::filter([](auto const& tab) { return std::get<1>(tab).EntityType() != EMyEntityType::view; })) {
      table.SQL_Create_Table(os);
      os << '\n';
      if (table.SQL_Create_PostConditions(os)) os << '\n';
      }

   os << "-- create views for the project\n\n";
   for (auto const& [_, table] : Tables() | std::views::filter([](auto const& tab) { return std::get<1>(tab).EntityType() == EMyEntityType::view; })) {
      table.SQL_Create_View(os);
      os << '\n';
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

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create primary keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables())  if(table.SQL_Create_Primary_Key(os)) os << '\n';
 
   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create unique keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Unique_Keys(os)) os << '\n';
   
   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create foreign keys for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Foreign_Keys(os)) os << '\n'; 

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create check conditions for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Check_Conditions(os)) os << '\n';

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

   os << "\n-- create indices for tables of the dictionary " << Name() << '\n';
   for (auto const& [_, table] : Tables()) if(table.SQL_Create_Indices(os)) os << '\n';

   os << "\nGO\n\n"; // problem of MS SQL with Statement Stacks and commit before next step

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




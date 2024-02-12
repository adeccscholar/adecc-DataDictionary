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

bool TMyTable::SQL_Create_Table(std::ostream& os) const {

   auto SQLRow = [this](TMyAttribute const& attr, size_t len) {
      auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
      return std::format("   {:<{}} {}{}{}", attr.DBName(), len, datatype.DatabaseType(),
         (datatype.UseLen() && datatype.UseScale() ? std::format("({}, {})", attr.Len(), attr.Scale()) :
            (datatype.UseLen() ? std::format("({})", attr.Len()) : ""s)), (attr.NotNull() ? " NOT NULL"s : ""s));
      };

   try {
      os << "CREATE TABLE " << FullyQualifiedSQLName() << " (\n";

      auto maxLength = std::ranges::max_element(Attributes(), [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != Attributes().end()) {
         auto len = maxLength->DBName().size();
         for (auto const& attribute : Attributes() | std::ranges::views::take(Attributes().size() - 1)) {
            os << SQLRow(attribute, len) << ",\n";
         }
         os << SQLRow(Attributes().back(), len) << '\n';
      }
      os << "   );\n\n";

   }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
   }
   return true;
}

bool TMyTable::SQL_Create_Primary_Key(std::ostream& os) const {
   try {
      std::ostringstream tmp_os;
      tmp_os << "ALTER TABLE " << FullyQualifiedSQLName() << " ADD CONSTRAINT pk_" << SQLName() << " PRIMARY KEY (";
      size_t i = 0;
      std::ranges::for_each(Attributes() | std::views::filter([](auto const& a) { return a.Primary() == true; }),
         [&tmp_os, &i](auto const& a) { tmp_os << (i++ > 0 ? ", " : "") << a.DBName();  });
      if (i > 0) os << tmp_os.str() << ");\n";
   }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
   }
   return true;
}

bool TMyTable::SQL_Create_Foreign_Keys(std::ostream& os) const {
   try {
      if (!References().empty()) {
         std::ranges::for_each(References(), [&os](auto ref) { os << ref.SQLRow() << ";\n";  });
         os << '\n';
      }
   }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
   }
   return true;
}

bool TMyTable::SQL_Create_Unique_Keys(std::ostream& os) const {
   try {
      auto unique_keys = Indices() | std::views::filter([](auto const& i) { return i.IndexType() == EMyIndexType::key; });
      if (!unique_keys.empty()) {
         std::ranges::for_each(unique_keys, [&os](auto ref) { os << ref.SQLRow() << ";\n";  });
         os << '\n';
      }
   }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
   }
   return true;
}

bool TMyTable::SQL_Create_Indices(std::ostream& os) const {
   try {
      auto indices = Indices() | std::views::filter([](auto const& i) { return i.IndexType() != EMyIndexType::key; });
      if (!indices.empty()) {
         std::ranges::for_each(indices, [&os](auto ref) { os << ref.SQLRow() << ";\n";  });
         os << '\n';
      }
   }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
   }
   return true;
}

bool TMyTable::SQL_Create_Check_Conditions(std::ostream& os) const {
   auto processing_data = Attributes() | std::views::transform([this](auto const& attr) {
                                             return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); })
                                       | std::views::filter([](auto const& data) {
                                             return data.first.CheckSeq().size() > 0 || data.second.CheckSeq().size() > 0;
                                             })
                                       | std::ranges::to<std::vector>();

   // create check conditions for this table
   std::ranges::for_each(processing_data, [this, &os](auto const& p) {
                   auto Exists = [](auto const& val) { return val.CheckSeq().size() > 0; };
                   os << std::format("ALTER TABLE {} ADD CONSTRAINT ck_{}_{} CHECK (", FullyQualifiedSQLName(), SQLName(), p.first.DBName());
                   if (Exists(p.first) && Exists(p.second)) os << std::format("{0} {1} AND {0} {2}", p.first.DBName(), p.second.CheckSeq(), p.first.CheckSeq());
                   else if(Exists(p.first))  os << std::format("{0} {1}", p.first.DBName(), p.first.CheckSeq());
                   else if(Exists(p.second)) os << std::format("{0} {1}", p.first.DBName(), p.second.CheckSeq());
                   os << ");\n";
                   });
   return true;
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

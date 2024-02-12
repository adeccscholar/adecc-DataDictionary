/** \file
   \brief Implementierung der Klassen für die Metadatenverwaltung
   \version 1.0
   \since Version 1.0
   \authors Volker Hillmann (VH)
   \date 29.01.2024 VH erstellt
   \date 31.01.2024 VH Trennung und Aufteilung in Definition und Implementierung
   \date 02.02.2024 VH Referenzen ergänzt (TMyReferences, myReferences)
   \date 04.02.2024 VH Dokumentation ergänzt, Bereich für Indizes ergänzt, Referenzen typisiert und Beschreibung / Notizen ergänzt
   \copyright copyright &copy; 2024. Alle Rechte vorbehalten.
   This project is released under the MIT License.
*/

/**
* \include{doc} Documentation_MIT_License.dox
* \include{doc} Documentation_adecc_Scholar.dox
* \include{doc} Documentation_adeccDatabase.dox
* \include{doc} Documentation.dox
*/

#include "DataDictionary.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;

// TMyAttribute

/*
// type, ref, prefix, init, header
std::tuple<std::string, bool, std::string, std::string, std::string, std::string> TMyAttribute::FullSourceInformations() const {
   auto const& dt = Table().Dictionary().FindDataType(DataType());
   return std::make_tuple(dt.SourceType(), dt.UseReference(), dt.Prefix(),  dt.InitSeq(), dt.Headerfile());
   }
*/


std::string TMyAttribute::Comment_Attribute() const {
   if (Comment().size() > 0) return Comment();
   else {
      std::ostringstream sComment;
      sComment << "attribute \"" << Name() << "\" in entity \"" << table.SourceName() << "\"";
      return sComment.str();
      }
   }

// TMyReferences

std::string TMyReferences::ReferenceTypeTxt() const {
   switch(ReferenceType()) {
      case EMyReferenceType::undefined: return "undefined"s;
      case EMyReferenceType::generalization: return "generalization"s;
      case EMyReferenceType::range: return "range"s;
      case EMyReferenceType::assoziation: return "assoziation"s;
      case EMyReferenceType::aggregation: return "aggregation"s;
      case EMyReferenceType::komposition: return "komposition"s;
      default: return "error"s;
      }
   }

std::string TMyReferences::SQLRow() const {
   std::string strSQL;
   std::ostringstream sList1, sList2;

   TMyTable const& ref = table.Dictionary().FindTable(RefTable());

   sList1 << "(";
   sList2 << "(";
   int i = 0;
   std::ranges::for_each(Values(), [this, &ref, &sList1, &sList2, &i](auto const& row) {
      sList1 << (i   > 0 ? ", " : "") << table.FindAttribute(row.first).DBName();
      sList2 << (i++ > 0 ? ", " : "") << ref.FindAttribute(row.second).DBName();
      });
   sList1 << ")";
   sList2 << ")";

   strSQL = "ALTER TABLE "s + table.FullyQualifiedSQLName() + " ADD CONSTRAINT "s + Name() + " FOREIGN KEY "s + sList1.str() + 
            " REFERENCES "s + ref.FullyQualifiedSQLName() + " "s + sList2.str();
   return strSQL;
   }

// TMyIndices

std::string TMyIndices::SQLRow() const {
   std::ostringstream sSQL;
   if(IndexType() == EMyIndexType::key) {
      sSQL << "ALTER TABLE " << table.FullyQualifiedSQLName() << " ADD CONSTRAINT " << Name()
           << " UNIQUE (";
      unsigned int i = 0;
      std::ranges::for_each(Values(), [this, &sSQL, &i](auto const& row) {
                           sSQL << (i++ > 0 ? ", " : "") << table.FindAttribute(row.first).DBName(); });
      sSQL << ")";
      }
   else {
      sSQL << "CREATE";
      switch(IndexType()) {
         case EMyIndexType::unique:       sSQL << " UNIQUE";       break;
         case EMyIndexType::clustered:    sSQL << " CLUSTERED";    break;
         case EMyIndexType::nonclustered: sSQL << " NONCLUSTERED"; break;
         }      
      sSQL << " INDEX "s + Name() + " ON " << table.FullyQualifiedSQLName() << " (";
      unsigned int i = 0;
      std::ranges::for_each(Values(), [this, &sSQL, &i](auto const& row) {
                 sSQL << (i++ > 0 ? ", " : "") << table.FindAttribute(row.first).DBName(); });
      sSQL << ")";
      }
   return sSQL.str();
   }

// TMyTable

TMyAttribute const& TMyTable::FindAttribute(std::string const& strName) const {
   if (auto it = std::find_if(Attributes().begin(), Attributes().end(), [&strName](auto const& attr) {
      return strName == attr.Name();
      }); it == Attributes().end()) [[unlikely]] {
         throw std::runtime_error("Attribute: \""s + strName + "\", in table: \""s + Name() + "\" not found."s);
         }
   else return *it;
   }

TMyAttribute const& TMyTable::FindAttribute(size_t iID) const {
   if (auto it = std::find_if(Attributes().begin(), Attributes().end(), [&iID](auto const& attr) {
      return iID == attr.ID();
      }); it == Attributes().end()) [[unlikely]] {
         throw std::runtime_error("Attribute with ID: "s + /*iID + */ ", in table: \""s + Name() + "\" not found."s);
         }
   else return *it;
}

TMyTable& TMyTable::AddAttribute(int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
                                 size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pCheck, 
                                 std::string const& pInit, std::string const& pComment) {
   Attributes().emplace_back(TMyAttribute(*this, pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, pCheck, pInit, pComment));
   return *this;
   }

TMyTable& TMyTable::AddReference(std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                                 std::string const& pDescription, std::string const& pComment, 
                                 std::vector<std::pair<size_t, size_t>> && pValues) {
   References().emplace_back(TMyReferences(*this, pName, pRefType, pRefTable, pDescription, pComment, std::move(pValues)));
   return *this;
   }

TMyTable& TMyTable::AddIndex(std::string const& pName, EMyIndexType pIdxType, std::string const& pComment, std::vector<std::pair<size_t, bool>>&& pValues) {
   Indices().emplace_back(TMyIndices(*this, pName, pIdxType, pComment, std::move(pValues)));
   return *this;
   }


// TMyDictionary

std::string TMyDictionary::Identifier() const {
   std::string result;
   std::ranges::unique_copy(Name(), std::back_inserter(result), [](char a, char b) { return std::isspace(a) && std::isspace(b); });
   std::ranges::for_each(result, [](char& c) { if (std::isspace(c)) c = '_'; });
   return result;
   }

TMyDatatype const& TMyDictionary::FindDataType(std::string const& strDataType) const {
   if (auto it = datatypes.find(strDataType); it == datatypes.end()) [[unlikely]] throw std::runtime_error("datatype \""s + strDataType + " couldn't found."s);
   else return it->second;
   }

TMyDatatype& TMyDictionary::AddDataType(std::string const& pDataType, std::string const& pDatabaseType, bool pUseLen, bool pUseScale,
                                        std::string const& pCheck, std::string const& pSourceType, std::string const& pHeader, 
                                        std::string const& pPrefix, bool pUseReference, std::string const& pComment) {
   TMyDatatype datatype(*this, pDataType, pDatabaseType, pUseLen, pUseScale, pCheck, pSourceType, pHeader, pPrefix, pUseReference, pComment);

   if (auto [val, success] = datatypes.emplace(pDataType, std::move(datatype)); !success) [[unlikely]]
      throw std::runtime_error("datatype \""s + pDataType + "\" in dictionary \""s + Name() + "\" couldn't inserted."s);
   else return val->second;
   }

TMyTable const& TMyDictionary::FindTable(std::string const& strTable) const {
   if (auto it = tables.find(strTable); it == tables.end()) [[unlikely]] throw std::runtime_error("table \""s + strTable + "\" couldn't found."s);
   else return it->second;
   }

TMyTable& TMyDictionary::FindTable(std::string const& strTable) {
   if (auto it = tables.find(strTable); it == tables.end()) [[unlikely]] throw std::runtime_error("table \""s + strTable + "\" couldn't found."s);
   else return it->second;
}

TMyTable& TMyDictionary::AddTable(std::string const& pName, EMyEntityType pType, std::string const& pSQLName, std::string const& pSchema,
                                  std::string const& pSourceName, std::string const& pNamespace,
                                  std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pComment) {
   TMyTable table(*this, pName, pType, pSQLName, pSchema, pSourceName, pNamespace, pSrcPath, pSQLPath, pComment);
   auto [val, success] = tables.emplace(myTables::value_type{ pName, std::forward<TMyTable>(table) });
   if (!success) [[unlikely]] throw std::runtime_error("table \""s + pName + " couldn't inserted."s);
   else return val->second;
   }


void TMyDictionary::CreateTable(std::string const& strTable, std::ostream& out) const {
   auto const& table = FindTable(strTable);
   table.SQL_Create_Table(out);
   table.SQL_Create_Primary_Key(out);
   table.SQL_Create_Foreign_Keys(out);
   table.SQL_Create_Unique_Keys(out);
   table.SQL_Create_Indices(out);
   }


void TMyDictionary::CreateClass(std::string const& strTable, std::ostream& out) const {
   auto const& table = FindTable(strTable);
   table.CreateHeader(out);
   }



void  TMyDictionary::Create_All(std::ostream& out, std::ostream& err) const {
   try {
      out << "Dictionary:  " << Name() << '\n'
          << "Identifier:  " << Identifier() << '\n'
          << "Denotation:  " << Description() << '\n'
          << "Version:     " << Version() << '\n'
          << "Author:      " << Author() << '\n'
          << "Date:        " << CurrentTimeStamp() << '\n';
      if (Copyright().size() > 0) {
         out << "Copyright © " << Copyright() << '\n';

         if (License().size() > 0) out << License() << '\n';
         }
       
      out << "-----------------------------------------------------------------------------\n";

      fs::path sqlPath = SQLPath();
      fs::create_directories(sqlPath);
      out << "create sql files in directory: " << sqlPath.string() << '\n';

      std::ofstream of_sql(sqlPath / "create_tables.sql");
      of_sql << "/* ------------------------------------------------------------------------------------\n"
             << " * script with statements to create tables for the project " << Name() << '\n'
             << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
             << " * author:       " << Author() << '\n'
             << " * copyright © " << Copyright() << '\n'
             << " * ------------------------------------------------------------------------------------ */\n\n";

      for (auto const& [name, table] : Tables()) {
         out << "create sql for table: " << name << " ... ";
         table.SQL_Create_Table(of_sql);
         of_sql << "\n";
         out << "done\n";
         }
      of_sql.close();

      of_sql.open(sqlPath / "create_additinals.sql");
      of_sql << "/* ------------------------------------------------------------------------------------\n"
             << " * script with keys, references and indices for the project " << Name() << '\n'
             << " * generated at: " << CurrentTimeStamp() << " with the adecc Scholar metadata generator\n"
             << " * author:       " << Author() << '\n'
             << " * copyright © " << Copyright() << '\n'
             << " * ----------------------------------------------------------------------------------- */\n";

      of_sql << "\n-- create primary keys for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create primary key for table: " << name << " ... ";
         table.SQL_Create_Primary_Key(of_sql);
         out << "done\n";
         }

      of_sql << "\n-- create unique keys for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create unique keys for table: " << name << " ... ";
         table.SQL_Create_Unique_Keys(of_sql);
         out << "done\n";
         }

      of_sql << "\n-- create foreign keys for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create foreign keys for table: " << name << " ... ";
         table.SQL_Create_Foreign_Keys(of_sql);
         out << "done\n";
         }

      of_sql << "\n-- create check conditions for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create check conditions for table: " << name << " ... ";
         table.SQL_Create_Check_Conditions(of_sql);
         out << "done\n";
         }

      of_sql << "\n-- create indices for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create indices for table: " << name << " ... ";
         table.SQL_Create_Indices(of_sql);
         out << "done\n";
         }

      of_sql.close();


      // create the general documentation page with all informations
      fs::path doxPath = DocPath();
      fs::create_directories(doxPath);
      std::ofstream of_dox(doxPath / (Identifier() + ".dox"s));
      Create_Doxygen(of_dox);

      fs::create_directories(SourcePath());
      fs::create_directories(DocPath());

      out << "\ncreate source files in directory: " << SourcePath().string() << '\n';

      for (auto const& [name, table] : Tables()) {
         auto srcPath = SourcePath() / table.SrcPath();
         fs::create_directories(srcPath);
         out << "files for table " << name << " ... ";
         std::ofstream of_src(srcPath / (name + ".h"s));
         table.CreateHeader(of_src);
         of_src.close();

         of_src.open(srcPath / (name + ".cpp"s));
         table.CreateSource(of_src);

         auto doxPath = DocPath() / table.SrcPath();
         fs::create_directories(doxPath);
         std::ofstream of_dox(doxPath / (name + ".dox"s));
         table.CreateDox(of_dox);

         out << "done.\n";
         }
      }
   catch(std::exception& ex) {
      err << ex.what() << '\n';
      }
   }
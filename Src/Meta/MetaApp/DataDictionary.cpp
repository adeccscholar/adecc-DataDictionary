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
#include <tuple>
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
      case EMyReferenceType::composition: return "composition"s;
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

/// \brief seek all parents of a table to add headers and use the data for processing
std::vector<TMyTable> TMyTable::GetParents() const {
   return References() | std::views::filter([](auto const& r) { return r.ReferenceType() == EMyReferenceType::generalization; })
                       | std::views::transform([this](auto const& r) { return Dictionary().FindTable(r.RefTable()); }) 
                       | std::ranges::to<std::vector<TMyTable>>();
   }

/// \brief seek all compositions to this table to add header files and data for processing
/// \details compositions only possible when the primary key of the holding table a part of composed table and in the reference
std::vector<TMyTable::my_part_of_type> TMyTable::GetPart_ofs() const {
   auto filter_refs = [this](auto const& r) {
      return r.RefTable() == Name() && r.ReferenceType() == EMyReferenceType::composition;
      };

   // retval for the function
   std::vector<TMyTable::my_part_of_type> parts;

   // traverse the range with all other tables and seek for a composition with this table
   for (auto const& [_, ref_table] : Dictionary().Tables() | std::views::filter([this](auto const& t) { return t.first != Name(); })) {
      auto composed = ref_table.References() | std::views::filter(filter_refs) | std::ranges::to<std::vector>();
   
      // there should be only 1, otherwise is there an error
      if(composed.size() == 1) {      
         auto comp_keys = composed[0].Values() | std::views::transform([](auto const& p) { return p.first; }) | std::ranges::to<std::set>();

         // seek for primary keys which are NOT in the reference, this are the keys for the composed datatype
         auto filter_prim = [&comp_keys](auto const& attr) { 
            return attr.Primary() && comp_keys.find(attr.ID()) == comp_keys.end(); 
            };

         std::string strTypeName = "ty_for_" + ref_table.Name();
         std::string strVarName  = "the" + ref_table.Name();

         // vector with the keys of the composed table which are not part of the reference, part of the retval type
         auto prim_keys = ref_table.Attributes() | std::views::filter(filter_prim)
                                                 | std::views::transform([](auto const& attr) { return attr.ID(); })
                                                 | std::ranges::to<std::vector<size_t>>();
         
         /*
         auto find_key = [&comp_keys](size_t val) { 
            return std::ranges::find_if(comp_keys, [&val](size_t e) { return e == val;  }) != comp_keys.end();
            };
         */

         //std::ranges::for_each(composed[0].Values(), [](std::pair<size_t, size_t>& p) { std::swap(p.first, p.second); });
         // swap the referenced values that the first value represented attributes of own table
         auto swapped = composed[0].Values() | std::views::transform([](const auto& pair) {
                                                    return std::make_pair(pair.second, pair.first);
                                                    }) | std::ranges::to<std::vector>();

         // add constructed values to the retval
         parts.emplace_back(std::make_tuple(ref_table, strTypeName, strVarName, prim_keys, swapped));
         }
      }
   return parts;
   }

/*
// generates datatypes, attribute name, und table + data of relationship
auto composition_values = part_of_data | std::views::transform([](auto const& data) {
   auto const& [tab, ref] = data; // split pair in both elements, the composed table and the data for the relationship
   std::string strTypeName = "ty"s + tab.Name();
   std::string strMember = "m"s + tab.Name();
   return std::make_tuple(strTypeName, strMember, tab, ref);
   }) | std::ranges::to<std::vector>();

*/

/// \brief build a container with pairs of attributes with database informations to this
std::vector<std::pair<TMyAttribute, TMyDatatype>> TMyTable::GetProcessing_Data() const {
   return Attributes() | std::views::transform([this](auto const& attr) {
                             return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); })
                       | std::ranges::to<std::vector>();
   }


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

TMyTable& TMyTable::AddDescription(std::string const& pDescription) {
   Description() += ("\n"s + pDescription);
   return *this;
   }

TMyTable& TMyTable::AddComment(std::string const& pComment) {
   Comment() += ("\n"s + pComment);
   return *this;
   }


TMyTable& TMyTable::AddAttribute(int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
                                 size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pCheck, 
                                 std::string const& pInit, std::string const& pComputed, std::string const& pComment) {
   Attributes().emplace_back(TMyAttribute(*this, pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, pCheck, pInit, pComputed, pComment));
   return *this;
   }

TMyTable& TMyTable::AddReference(std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                                 std::string const& pDescription, std::string const& pCardinality, std::optional<size_t> const& pShowAttribute,
                                 std::string const& pComment, std::vector<std::pair<size_t, size_t>> && pValues) {
   References().emplace_back(TMyReferences(*this, pName, pRefType, pRefTable, pDescription, pCardinality, pShowAttribute, pComment, std::move(pValues)));
   return *this;
   }

TMyTable& TMyTable::AddIndex(std::string const& pName, EMyIndexType pIdxType, std::string const& pComment, std::vector<std::pair<size_t, bool>>&& pValues) {
   Indices().emplace_back(TMyIndices(*this, pName, pIdxType, pComment, std::move(pValues)));
   return *this;
   }

TMyTable& TMyTable::AddRangeValue(std::string const& pStatement) {
   RangeValues().emplace_back(pStatement);
   return *this;
   }

TMyTable& TMyTable::AddPostConditions(std::string const& pStatement) {
   PostConditions().emplace_back(pStatement);
   return *this;
   }

TMyTable& TMyTable::AddCleanings(std::string const& pStatement) {
   Cleanings().emplace_back(pStatement);
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
                                  std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pDenotation) {
   TMyTable table(*this, pName, pType, pSQLName, pSchema, pSourceName, pNamespace, pSrcPath, pSQLPath, pDenotation);
   auto [val, success] = tables.emplace(myTables::value_type{ pName, std::forward<TMyTable>(table) });
   if (!success) [[unlikely]] throw std::runtime_error("table \""s + pName + "\" couldn't inserted."s);
   else return val->second;
   }

TMyNameSpace& TMyDictionary::AddNameSpace(std::string const& pName, std::string const& pDonation, std::string const& pDescription) {
   TMyNameSpace theSpace(*this, pName, pDonation, pDescription);
   auto [val, success] = namespaces.emplace(myNameSpaces::value_type{ pName, std::forward<TMyNameSpace>(theSpace) });
   if (!success) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't inserted."s);
   else return val->second;
   }


void TMyDictionary::CreateTable(std::string const& strTable, std::ostream& out) const {
   auto const& table = FindTable(strTable);
   table.SQL_Create_Table(out, false);
   table.SQL_Create_PostConditions(out);
   table.SQL_Create_Alter_Table(out);
   table.SQL_Create_Primary_Key(out);
   table.SQL_Create_Foreign_Keys(out);
   table.SQL_Create_Unique_Keys(out);
   table.SQL_Create_Indices(out);
   table.SQL_Create_RangeValues(out);
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
      Create_SQL_Tables(of_sql);
      of_sql.close();

      of_sql.open(sqlPath / "create_additinals.sql");
      Create_SQL_Additionals(of_sql);
      of_sql.close();

      of_sql.open(sqlPath / "create_rangevalues.sql");
      Create_SQL_RangeValues(of_sql);
      of_sql.close();

      of_sql.open(sqlPath / "drop_all.sql");
      SQL_Drop_Tables(of_sql);
      of_sql.close();


      // create the general documentation page with all informations
      fs::path doxPath = DocPath();
      fs::create_directories(doxPath);
      std::ofstream of_dox(doxPath / (Identifier() + ".dox"s));
      Create_Doxygen(of_dox);
      of_dox.close();

      fs::create_directories(doxPath / "sql");
      of_dox.open(doxPath / "sql" / (Identifier() + "_sql.dox"s));
      Create_Doxygen_SQL(of_dox);
      of_dox.close();



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
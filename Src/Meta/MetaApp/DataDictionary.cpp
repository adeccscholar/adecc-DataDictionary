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
#include "GenerateSQL.h"
#include "DictionaryHelper.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <tuple>
#include <vector>
#include <set>
//#include <queue>
#include <deque>
#include <functional>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <ranges>
#include <regex>
#include <cctype>
#include <locale>

#include <windows.h>


namespace fs = std::filesystem;
using namespace std::string_literals;

/// \brief convert an ANSI file to UTF-8 + BOM (this method is Windows-specific and must be adapted for other platforms)
/// \todo check error messages with system_error 
void convertToUTF8WithBOM(fs::path const& filePath) {
   std::ifstream inputFile(filePath);
   if (!inputFile.is_open()) {
      throw std::runtime_error("Error when opening the input file \""s + filePath.string() + "\"."s);
      }

   std::string tmpFile = filePath.string() + ".tmp";
   std::ofstream outputFile(tmpFile);
   if (!outputFile.is_open()) {
      throw std::runtime_error("Error when opening the output file \""s + tmpFile + "\"."s);
      }

   // write BOM (Byte Order Mark) for UTF-8 in the output file
   outputFile.put(static_cast<char>(0xEF));
   outputFile.put(static_cast<char>(0xBB));
   outputFile.put(static_cast<char>(0xBF));

   // perform conversion line by line
   std::string line;
   while (std::getline(inputFile, line)) {
      // convert a ANSI to a UTF-8 using windows specific function MultiByteToWideChar and write this directly to the output file
      int utf8_length = ::MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, NULL, 0);  
      if (utf8_length == 0) {  // must be > 0 for an empty line too because its contain '\0'
         throw std::runtime_error("error when determining the UTF-8 length in \""s + line + "\"."s);
         }

      std::wstring utf16_string(utf8_length, L'\0');
      if (!::MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, &utf16_string[0], utf8_length)) {
         throw std::runtime_error("error when converting from ANSI to UTF-16 in \""s + line + "\"."s);
         }

      int utf8_buffer_size = ::WideCharToMultiByte(CP_UTF8, 0, utf16_string.c_str(), -1, NULL, 0, NULL, NULL);
      if (utf8_buffer_size == 0) {
         throw std::runtime_error("error when determining the UTF - 8 buffer size in \""s + line + "\"."s);
         }

      std::string utf8_string(utf8_buffer_size - 1, '\0'); // -1, to ignore the NULL character at the end
      if (!::WideCharToMultiByte(CP_UTF8, 0, utf16_string.c_str(), -1, &utf8_string[0], utf8_buffer_size, NULL, NULL)) {
         throw std::runtime_error("Error when converting from UTF-16 to UTF-8 in \""s + line + "\"."s);
         }

      outputFile << utf8_string << "\n";
      }

   // close files before deleting and renaming
   inputFile.close();
   outputFile.close();

   // delete original file and rename temporary file
   fs::remove(filePath);
   fs::rename(tmpFile, filePath);
   }

// ---------------------------------------------------------------------------------------------------------------------------
// TMyAttribute

/*
// type, ref, prefix, init, header
std::tuple<std::string, bool, std::string, std::string, std::string, std::string> TMyAttribute::FullSourceInformations() const {
   auto const& dt = Table().Dictionary().FindDataType(DataType());
   return std::make_tuple(dt.SourceType(), dt.UseReference(), dt.Prefix(),  dt.InitSeq(), dt.Headerfile());
   }
*/
TMyAttribute& TMyAttribute::AddDescription(std::string const& pText) {
   if (Description().size() > 0) {
      Description() += "\n"s;
      Description() += pText;
      }
   else Description() = pText;
   return *this;
   }

TMyAttribute& TMyAttribute::AddComment(std::string const& pText) {
   if (Comment().size() > 0) {
      Comment() += "\n"s;
      Comment() += pText;
      }
   else Comment() = pText;
   return *this;
   }



std::string TMyAttribute::Comment_Attribute() const {
   if (Denotation().size() > 0) return Denotation();
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




// TMyTable

std::string TMyTable::EntityTypeTxt() const {
   switch (EntityType()) {
      case EMyEntityType::table:        return "Entity"; 
      case EMyEntityType::range:        return "Domain"; 
      case EMyEntityType::relationship: return "Relationship";
      case EMyEntityType::view:         return "View"; 
      default:                          return "unknown";
      }
   }


std::set<std::string> TMyTable::GetPrecursors(bool boAll) const {
   std::set<std::string> retval ;
   // seek all generalizations
   auto parents = References() | std::views::filter([](auto const& r) { 
                                            return r.ReferenceType() == EMyReferenceType::generalization ||
                                                   r.ReferenceType() == EMyReferenceType::composition ||
                                                   r.ReferenceType() == EMyReferenceType::range; })
                               | std::views::transform([this](auto const& r) { return Dictionary().FindTable(r.RefTable()).Name(); })
                               ;

   std::ranges::copy(parents, std::inserter(retval, retval.end()));

   if (boAll) {
      std::ranges::for_each(parents, [this, &retval](std::string const& tab) {
         auto succ = Dictionary().FindTable(tab).GetPrecursors(true);
         std::ranges::copy(succ, std::inserter(retval, retval.end()));
         });
   }

   return retval;
   }

std::set<std::string> TMyTable::GetSuccessors(bool boAll) const {
   std::set<std::string> retval;
   // seek all part of relationships
   auto condition = [this](TMyTable const& table) {
      return std::ranges::any_of(table.References(), [this](TMyReferences const& value) { 
                    return value.RefTable() == Name() && (value.ReferenceType() == EMyReferenceType::generalization ||
                                                          value.ReferenceType() == EMyReferenceType::composition ||
                                                          value.ReferenceType() == EMyReferenceType::range); });
      };

   auto parents = Dictionary().Tables() | std::views::filter([this](auto const& t) { return t.first != Name(); })
                                        | std::views::transform([this](auto const& t) -> TMyTable const& { return t.second; })
                                        | std::views::filter(condition)
                                        | std::views::transform([](auto const& t)  { return t.Name(); });

   std::ranges::copy(parents, std::inserter(retval, retval.end()));
   if(boAll) {
      std::ranges::for_each(parents, [this, &retval](std::string const& tab) {
            auto succ = Dictionary().FindTable(tab).GetSuccessors(true);
            std::ranges::copy(succ, std::inserter(retval, retval.end()));
            });
      }
   return retval;
   }



/// \brief seek all parents of a table to add headers and use the data for processing
std::vector<TMyTable> TMyTable::GetParents(EMyReferenceType ref_type) const {
   return References() | std::views::filter([&ref_type](auto const& r) { return r.ReferenceType() == ref_type; })
                       | std::views::transform([this](auto const& r) { return Dictionary().FindTable(r.RefTable()); }) 
                       | std::ranges::to<std::vector<TMyTable>>();
   }


std::vector<TMyTable::my_part_of_type> TMyTable::GetParent_ofs(EMyReferenceType ref_type) const {
   std::vector<TMyTable::my_part_of_type> parts;
   auto parts_ = References() | std::views::filter([&ref_type](auto const& ref) { return ref.ReferenceType() == ref_type;  });
   if(!parts.empty()) {
      for(auto const& ref_part : parts_) {
         TMyTable const& ref_table = Dictionary().FindTable(ref_part.RefTable());
         //std::string strRefType = 
         }
      }
   return parts;
   }



/// \brief seek all compositions to this table to add header files and data for processing
/// \details compositions only possible when the primary key of the holding table a part of composed table and in the reference
std::vector<TMyTable::my_part_of_type> TMyTable::GetPart_ofs(EMyReferenceType ref_type) const {
   auto filter_refs = [this, ref_type](auto const& r) {
      return r.RefTable() == Name() && r.ReferenceType() == ref_type;
      };

   std::vector<TMyTable::my_part_of_type> parts;

   for (auto const& [_, ref_table] : Dictionary().Tables() | std::views::filter([this](auto const& t) { return t.first != Name(); })) {
      auto composed = ref_table.References() | std::views::filter(filter_refs) | std::ranges::to<std::vector>();
   
      for(auto & comp_val : composed) {      
         auto comp_keys = std::views::all(comp_val.Values()) | own::views::first  | std::ranges::to<std::set>();
 
         auto filter_prim = [&comp_keys](auto const& attr) { 
            return attr.Primary() && comp_keys.find(attr.ID()) == comp_keys.end(); 
            };

         std::string strTypeName;
         std::transform(ref_table.Name().begin(), ref_table.Name().end(), std::back_inserter(strTypeName), [](char c) { return std::tolower(c); });
         strTypeName += "_ty";
         std::string strVarName  = "m_" + ref_table.Name();

         auto prim_keys = ref_table.Attributes() | std::views::filter(filter_prim)
                                                 | std::views::transform([](auto const& attr) { return attr.ID(); })
                                                 | std::ranges::to<std::vector<size_t>>();
         
         auto swapped = comp_val.Values() | std::views::transform([](const auto& pair) {
                                                    return std::make_pair(pair.second, pair.first);
                                                    }) | std::ranges::to<std::vector>();

         parts.emplace_back(std::make_tuple(ref_table, strTypeName, strVarName, prim_keys, swapped));
         }
      }
   return parts;
   }

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
   if (Description().size() > 0) Description() += "\n"s;
   Description() += pDescription;
   return *this;
   }

TMyTable& TMyTable::AddComment(std::string const& pComment) {
   if (Comment().size() > 0) Comment() += "\n"s;
   Comment() += pComment;
   return *this;
   }


TMyTable& TMyTable::AddAttribute(int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
                                 size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pCheck, 
                                 std::string const& pInit, std::string const& pComputed, std::string const& pDenotation) {
   auto determineCondition = [](const std::string& str) -> std::pair<int, std::string> {
      static std::regex parser(R"((^\((.*)\)$)|(^\[(.*)\]$)|(^\{(.*)\}$))"); // Regex für (), [] oder {}
      std::smatch match;
      if (str.empty()) return { 0, ""s };
      if (std::regex_match(str, match, parser)) {
         if (match[1].matched)      return { 1, match[2] }; // () found, retval of 1 and the text inside the brackets
         else if (match[3].matched) return { 2, match[4] }; // [] found, retval of 2 and the text inside the brackets
         else if (match[5].matched) return { 3, match[6] }; // {} found, retval of 3 and the text inside the brackets
         }
      return { 4, str }; // others, retval of 4 with the parameter
      };

   auto [computed_type, str_computed] = determineCondition(pComputed);
   
   EMyCalculationKinds calc_kind;
   switch(computed_type) {
      case 4: calc_kind = EMyCalculationKinds::attribute; break;
      case 1: calc_kind = EMyCalculationKinds::attribute_permanent; break;
      case 2: calc_kind = EMyCalculationKinds::table; break;
      case 3: calc_kind = EMyCalculationKinds::table_permanent; break;
      default: calc_kind = EMyCalculationKinds::undefined;
      }

   auto [check_type, str_check] = determineCondition(pCheck);
   
   EMyCheckKinds check_kind;
   switch (check_type) {
      case 1:  check_kind = EMyCheckKinds::attribute; break;
      case 4:  check_kind = EMyCheckKinds::direct; break;
      case 3:  // following {} and []
      case 2:  check_kind = EMyCheckKinds::table; break;
      default: check_kind = EMyCheckKinds::undefined;
      }

   Attributes().emplace_back(TMyAttribute(*this, pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, str_check, check_kind, pInit,
                                          str_computed, calc_kind, pDenotation));
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
                                        std::string const& pPrefix, 
                                        std::string const& pCorbaType, std::string const& pCorbaModule,
                                        bool pUseReference, std::string const& pComment) {
   TMyDatatype datatype(*this, pDataType, pDatabaseType, pUseLen, pUseScale, pCheck, pSourceType, pHeader, pPrefix, 
                               pCorbaType, pCorbaModule, pUseReference, pComment);

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

TMyNameSpace& TMyDictionary::FindNameSpace(std::string const& pName) {
   if (auto it = namespaces.find(pName); it == namespaces.end()) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't found."s);
   else return it->second;
   }

TMyNameSpace const& TMyDictionary::FindNameSpace(std::string const& pName) const {
   if (auto it = namespaces.find(pName); it == namespaces.end()) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't found."s);
   else return it->second;
   }

TMyDictionary& TMyDictionary::AddNameSpace(std::string const& pName, std::string const& pCorbaName, 
                                           std::string const& pDenotation, std::string const& pDescription, 
                                           std::string const& pComment) {
   TMyNameSpace theSpace(*this, pName, pCorbaName, pDenotation, pDescription, pComment);
   auto [val, success] = namespaces.emplace(myNameSpaces::value_type{ pName, std::forward<TMyNameSpace>(theSpace) });
   if (!success) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't inserted."s);
   else return *this;
   }

TMyDirectory& TMyDictionary::FindDirectory(std::string const& pName) {
   if (auto it = directories.find(pName); it == directories.end()) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't found."s);
   else return it->second;
   }

TMyDirectory const& TMyDictionary::FindDirectory(std::string const& pName) const {
   if (auto it = directories.find(pName); it == directories.end()) [[unlikely]] throw std::runtime_error("namespace \""s + pName + "\" couldn't found."s);
   else return it->second;
   }

TMyDictionary& TMyDictionary::AddDirectory(std::string const& pName, std::string const& pDenotation, std::string const& pDescription) {
   TMyDirectory theDir(*this, pName, pDenotation, pDescription);
   auto [val, success] = directories.emplace(myDirectories::value_type{ pName, std::forward<TMyDirectory>(theDir) });
   if (!success) [[unlikely]] throw std::runtime_error("directory \""s + pName + "\" couldn't inserted."s);
   else return *this;
   }



void TMyDictionary::CreateTable(std::string const& strTable, std::ostream& out) const {
   if (auto const& table = FindTable(strTable); table.EntityType() != EMyEntityType::view) {
      sql_builder()
         .WriteCreateTable(table, out)
         .WriteCreatePostConditions(table, out)
         .WriteAlterTable(table, out)
         .WritePrimaryKey(table, out)
         .WriteForeignKeys(table, out)
         .WriteUniqueKeys(table, out)
         .WriteCreateIndices(table, out)
         .WriteRangeValues(table, out);
      }
   else {
      sql_builder()
         .WriteCreateView(table, out);
      }
   }


void TMyDictionary::CreateClass(std::string const& strTable, std::ostream& out) const {
   auto const& table = FindTable(strTable);
   table.CreateHeader(out);
   }



void  TMyDictionary::Create_All(std::ostream& out, std::ostream& err) const {
   using job_type = std::pair<std::string, std::function<Generator_SQL const&(std::ostream&)>>;
   static std::vector<job_type> jobs = {
      /*
      * using job_type = std::pair<std::string, std::function<bool (std::ostream&)>>;
      job_type { "create_tables.sql"s,      std::bind(&TMyDictionary::Create_SQL_Tables, this, std::placeholders::_1) },
      job_type { "create_additinals.sql"s,  std::bind(&TMyDictionary::Create_SQL_Additionals, this, std::placeholders::_1) },
      job_type { "create_rangevalues.sql"s, std::bind(&TMyDictionary::Create_SQL_RangeValues, this, std::placeholders::_1) },
      job_type { "drop_all.sql"s,           std::bind(&TMyDictionary::SQL_Drop_Tables, this, std::placeholders::_1) },
      job_type { "add_documentation.sql"s,  std::bind(&TMyDictionary::Create_SQL_Documentation, this, std::placeholders::_1) }
      */
      job_type { "create_tables.sql"s,      std::bind(&Generator_SQL::WriteSQLTables, &this->sql_builder(), std::placeholders::_1) },
      job_type { "create_additinals.sql"s,  std::bind(&Generator_SQL::WriteSQLAdditionals, &this->sql_builder(), std::placeholders::_1) },
      job_type { "create_rangevalues.sql"s, std::bind(&Generator_SQL::WriteSQLRangeValues, &this->sql_builder(), std::placeholders::_1) },
      job_type { "drop_all.sql"s,           std::bind(&Generator_SQL::WriteSQLDropTables, &this->sql_builder(), std::placeholders::_1) },
      job_type { "add_documentation.sql"s,  std::bind(&Generator_SQL::WriteSQLDocumentation, &this->sql_builder(), std::placeholders::_1) }

      };
   try {
      out << "Dictionary:  " << Name() << '\n'
          << "Identifier:  " << Identifier() << '\n'
          << "Denotation:  " << Denotation() << '\n'
          << "Version:     " << Version() << '\n'
          << "Author:      " << Author() << '\n'
          << "Date:        " << CurrentTimeStamp() << '\n';
      if (Copyright().size() > 0) {
         out << "Copyright (c) " << Copyright() << '\n';

         if (License().size() > 0) out << License() << '\n';
         }
       
      out << "-----------------------------------------------------------------------------\n";

      fs::path sqlPath = SQLPath();
      fs::create_directories(sqlPath);
      out << "create sql files in directory: " << sqlPath.string() << '\n';

      for(auto const& [filename, funct] : jobs) {
         auto fileName = sqlPath / filename;
         std::ofstream of_sql(fileName);
         funct(of_sql);
         of_sql.close();
         convertToUTF8WithBOM(fileName);
         }

      // create the general documentation page with all informations
      fs::path doxPath = DocPath();
      fs::create_directories(doxPath);
      out << "create documentation files in directory: " << doxPath.string() << '\n';
      auto fileName = doxPath / (Identifier() + ".dox"s);
      std::ofstream of_dox(fileName);
      Create_Doxygen(of_dox);
      of_dox.close();
      convertToUTF8WithBOM(fileName);

      fs::create_directories(doxPath / "sql");
      fileName = doxPath / "sql" / (Identifier() + "_sql.dox"s);
      of_dox.open(fileName);
      Create_Doxygen_SQL(of_dox);
      of_dox.close();
      convertToUTF8WithBOM(fileName);


      fs::create_directories(SourcePath());
      fs::create_directories(DocPath());

      out << "\ncreate source files in directory: " << SourcePath().string() << '\n';

      // ---------create base header when used ----------------------------
      if(UseBaseClass()) {
         auto srcPath = SourcePath() / PathToBase();
         fs::create_directories(srcPath.parent_path());
         std::ofstream of_base(srcPath);
         if(of_base) {
            CreateBaseHeader(of_base);
            of_base.close();
            convertToUTF8WithBOM(srcPath);
            }
         }
 
      // ---- create header and source files for tables -------------------
      for (auto const& [name, table] : Tables()) {
         auto srcPath = SourcePath() / table.SrcPath();
         fs::create_directories(srcPath);
         out << "files for table " << name << " ... ";
         auto fileName = srcPath / (name + ".h"s);
         std::ofstream of_src(fileName);
         table.CreateHeader(of_src);
         of_src.close();
         convertToUTF8WithBOM(fileName);

         fileName = srcPath / (name + ".cpp"s);
         of_src.open(fileName);
         table.CreateSource(of_src);
         of_src.close();
         convertToUTF8WithBOM(fileName);

         auto doxPath = DocPath() / table.SrcPath();
         fs::create_directories(doxPath);
         fileName = doxPath / (name + ".dox"s);
         std::ofstream of_dox(fileName);
         table.CreateDox(of_dox);
         of_dox.close();
         convertToUTF8WithBOM(fileName);
         out << "done.\n";
         }

      // ------- generate code for the persistence layer -------------------------
      //  only when a class name for the persistence layer defined before
      // -------------------------------------------------------------------------
      if(HasPersistenceClass()) {
         auto PathToPers = [this]() {
            if (PathToPersistence().root_path() == fs::path()) return SourcePath() / PathToPersistence();
            else return PathToPersistence();
            }();
         // create directories to the folder with the persistence layer files
         fs::create_directories(PathToPers);
         out << "\ncreate reader files in directory: " << PathToPers.string() << '\n';

         auto fileName = PathToPers / (PersistenceName() + "_sql.h"s);
         std::ofstream of_db(fileName);
         if (of_db) CreateSQLStatementHeader(of_db);
         of_db.close();
         convertToUTF8WithBOM(fileName);

         fileName = PathToPers / (PersistenceName() + "_sql.cpp"s);
         of_db.open(fileName);
         if (of_db) CreateSQLStatementSource(of_db);
         of_db.close();
         convertToUTF8WithBOM(fileName);

         fileName = PathToPers / (PersistenceName() + ".h"s);
         of_db.open(fileName);
         if (of_db) CreateReaderHeader(of_db);
         of_db.close();
         convertToUTF8WithBOM(fileName);

         fileName = PathToPers / (PersistenceName() + ".cpp"s);
         of_db.open(fileName);
         if (of_db) CreateReaderSource(of_db);
         of_db.close();
         convertToUTF8WithBOM(fileName);
         }

      if(boWithCorba) {  // eventuell später über if steuern
         auto idlBasicPath = IDLPath() / "Basic.idl";
         out << "\ncreate basic corba idl file: " << idlBasicPath.string();
         fs::create_directories(idlBasicPath.parent_path());
         std::ofstream of_base(idlBasicPath);
         CreateBasicCorbaIDL(of_base);
         of_base.close();
         // convertToUTF8WithBOM(idlBasicPath);

         auto corbaIDL = IDLPath() / (Identifier() + ".idl"s);
         out << "\ncreate corba idl file: " << corbaIDL.string();
         of_base.open(corbaIDL);
         CreateCorbaIDL(of_base);
         of_base.close();
         // convertToUTF8WithBOM(corbaIDL);

         auto corbaBasisPath = CorbaPath() / "Basic_impl.h";
         out << "\ncreate corba implementationfile for basic module: " << corbaBasisPath.string();
         fs::create_directories(corbaBasisPath.parent_path());
         of_base.open(corbaBasisPath);
         CreateBasicCorbaHeader(of_base);
         of_base.close();
         convertToUTF8WithBOM(corbaBasisPath);

         std::string strHeader = Identifier() + "_Impl.h"s;
         auto corbaImplHeader = CorbaPath() / strHeader;
         out << "\ncreate corba header file for implementation the corba servlet: " << corbaImplHeader.string();
         of_base.open(corbaImplHeader);
         CreateCorbaImplementationHeader(of_base);
         of_base.close();
         convertToUTF8WithBOM(corbaImplHeader);

         auto corbaImplSource = CorbaPath() / (Identifier() + "_Impl.cpp"s);
         out << "\ncreate corba header file for implementation the corba servlet: " << corbaImplSource.string();
         of_base.open(corbaImplSource);
         CreateCorbaImplementationSource(of_base, strHeader);
         of_base.close();
         convertToUTF8WithBOM(corbaImplSource);

         }

      }
   catch(std::exception& ex) {
      err << ex.what() << '\n';
      }
   }

std::vector<std::string> TMyDictionary::TopologicalSequence() const {
  auto tmpTables = Tables() | std::views::keys | std::ranges::to<std::vector>();

  std::sort(tmpTables.begin(), tmpTables.end(), [this](auto const& lhs, auto const& rhs) {
          if (auto cmp = this->FindTable(lhs).Namespace() <=> this->FindTable(rhs).Namespace(); cmp != 0) return cmp < 0;
          else return lhs < rhs;
          });


   std::unordered_map<std::string, std::set<std::string>> dependencies;
   for (auto const& tab : tmpTables) {
      auto const& table = FindTable(tab);
      auto parents = table.GetParents(EMyReferenceType::generalization)
                             | std::views::transform([](auto const& t) -> std::string { return t.Name(); })
                             | std::ranges::to<std::vector>();
      auto part_of_data = table.GetPart_ofs(EMyReferenceType::composition)
                             | std::views::transform([](auto const& t) -> std::string { return std::get<0>(t).Name(); })
                             | std::ranges::to<std::vector>();

      //auto concatenated = std::ranges::views::concat(parents, part_of_data);
      auto concatenated = std::views::join(std::array { parents, part_of_data } ) | std::ranges::to<std::set>();
      if (concatenated.size() > 0)
         dependencies.emplace(std::piecewise_construct, std::forward_as_tuple(tab), std::forward_as_tuple(concatenated));
      }

   auto zero_indegree = tmpTables | std::views::filter([&dependencies](auto const& t) { 
                                              return dependencies.find(t) == dependencies.end(); 
                                              })
                                  | std::ranges::to<std::deque>();


   std::sort(zero_indegree.begin(), zero_indegree.end(), [this](auto const& lhs, auto const& rhs) {
         if (auto cmp = this->FindTable(lhs).Namespace() <=> this->FindTable(rhs).Namespace(); cmp != 0) return cmp < 0;
         else return lhs < rhs;
         } );

   std::vector<std::string> sorted;

   while (!zero_indegree.empty()) {
      std::string class_name = zero_indegree.front();
      zero_indegree.pop_front();
      sorted.emplace_back(class_name);

      for (auto& dep_tables : dependencies) {
         if (dep_tables.second.find(class_name) != dep_tables.second.end()) {
            dep_tables.second.erase(class_name);
            if (dep_tables.second.size() == 0) {
               zero_indegree.push_back(dep_tables.first);
               }
            }
         }

      std::erase_if(dependencies, [](const auto& value) {
         return value.second.size() == 0;
         });
      }

   if (sorted.size() != tmpTables.size()) {
      throw std::runtime_error("There is an inconsistency (a cycle) in the dependencies!");
      }

   return sorted;
   }


#include "fibunacci.h"

void TMyDictionary::Test() const {
   /*
   std::vector<std::string> test_data = { "banana"s, "apple"s, "pear"s, "pineapple"s, "peach"s, "mango"s };
   auto test = test_data | own::views::size_co;

   std::cout << "\nTest:";
   for (size_t i = 0; auto t : test) std::cout << (i++ > 0 ? ", " : " ") << t;
   std::cout << "\n\n";
   */

   fibunacci_test();

   /*
   auto topologic = TopologicalSequence();

   std::ranges::for_each(topologic | std::views::take(topologic.size() - 1), [](auto const& val) { std::cout << val << ", "; });
   std::cout << topologic.back();

   auto all_datatypes = Tables() 
                        | std::views::values
                        | std::views::transform([](auto const& t) { return t.Attributes(); })
                        | std::views::join
                        | std::views::transform([](auto const& a) { return a.Table().Dictionary().FindDataType(a.DataType()); })
                        | std::ranges::to<std::set>();

   for (auto const& datatype : all_datatypes) {
      std::cout << std::format("{}\n", datatype.CorbaType());
      }

   auto all_attributes = Tables() | std::views::values
                                  | std::views::transform([](auto const& t) { return t.Attributes(); })
                                  | std::views::join;

   for (auto const& attr : all_attributes) {
      auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
      std::cout << std::format("{} {}::{}\n", datatype.CorbaType(), attr.Table().Name(), attr.Name());
      }
   */
   }

   /*
   auto  sort_func = [this](std::string const& lhs, std::string const& rhs) -> bool {
         auto prec = FindTable(lhs).GetPrecursors();
         if (prec.find(rhs) != prec.end()) {
            return false;
            }
         else {
            auto succ = FindTable(lhs).GetSuccessors();
            if (succ.find(rhs) != succ.end()) {
               return true;
            }
            else return false; // lhs < rhs;
            }
         };

   std::vector<std::string> work;
   std::ranges::copy(tables | std::views::transform([](auto const& t) { return t.first; }), std::back_inserter(work));
   std::ranges::sort(work, sort_func);

   std::ranges::for_each(work, [](auto const& t) { std::cout << t << "\n"; });
   }
*/
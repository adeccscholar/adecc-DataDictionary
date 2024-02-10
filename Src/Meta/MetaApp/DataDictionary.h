#pragma once
/** \file
   \brief Schnittstellen der Metadatenverwaltung
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

#include <iostream>
#include <iomanip>
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <ranges>
#include <format>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

using namespace std::string_literals;

inline std::string CurrentTimeStamp(std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now()) {
   auto const cz_ts = std::chrono::current_zone()->to_local(now);
   auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
   return std::format("{:%d.%m.%Y %X},{:03d}", cz_ts, millis.count());
   }

inline std::string CurrentDate(std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now()) {
   auto const cz_ts = std::chrono::current_zone()->to_local(now);
   return std::format("{:%d.%m.%Y}", cz_ts);
   }


class TMyDictionary;
class TMyTable;
class TMyAttribute;

template <typename ty>
concept MySize_T =  std::same_as<ty, unsigned int> || std::is_convertible_v<ty, std::string>;

class TMyDatatype {
private:
   using myData = std::tuple<std::string, std::string, bool, bool, std::string, std::string, std::string, std::string, bool, std::string>;
   myData data;
   TMyDictionary const& dictionary;
public:
   TMyDatatype() = delete;
   TMyDatatype(TMyDatatype const& other) : dictionary(other.dictionary), data(other.data) { }
   //TMyDatatype(TMyDatatype&& other) noexcept : dictionary(std::move(other.dictionary)) , data(std::move(other.data)) { }
   ~TMyDatatype() = default;

   auto operator <=> (TMyDatatype const& other) const { return data <=> other.data; }

   TMyDatatype(TMyDictionary const& dict, std::string const& pDataType, std::string const& pDatabaseType, bool pUseLen, bool pUseScale,
               std::string const& pSourceType, std::string const& pHeader, std::string const& pPrefix, 
               std::string const& pInit, bool pUseReference, std::string const& pComment) :
      dictionary { dict },
      data{ myData { pDataType, pDatabaseType, pUseLen, pUseScale, pSourceType, pHeader, pPrefix, pInit, pUseReference, pComment } } { }

   auto const& GetKey() const { return std::get<0>(data); }

   std::string const& DataType() const { return std::get<0>(data); }
   std::string const& DatabaseType() const { return std::get<1>(data); }
   bool               UseLen() const { return std::get<2>(data); }
   bool               UseScale() const { return std::get<3>(data); }
   std::string const& SourceType() const { return std::get<4>(data); }
   std::string const& Headerfile() const { return std::get<5>(data); }

   std::string const& Prefix() const { return std::get<6>(data); }
   std::string const& InitSeq() const { return std::get<7>(data); }
   bool               UseReference() const { return std::get<8>(data); }
   std::string const& Comment() const { return std::get<9>(data); }

};

using myDataTypes = std::map<std::string, TMyDatatype>;


class TMyAttribute {
private:
   using myData = std::tuple<int, std::string, std::string, std::string, size_t, size_t, bool, bool, std::string>;
   myData data;
   TMyTable const& table;
public:
   TMyAttribute() = delete;
   TMyAttribute(TMyAttribute const& other) : table(other.table), data(other.data) {  }
   //TMyAttribute(TMyAttribute&& other) noexcept : table(std::move(other.table)), data(std::move(other.data)) { }
   ~TMyAttribute() = default;

   auto operator <=> (TMyAttribute const& other) const { return data <=> other.data; }

   auto const& GetKey() const { return std::get<0>(data); }

   TMyAttribute(TMyTable const& pTable, int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
      size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pComment) :
      table { pTable },
      data{ myData { pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, pComment} } { }

   size_t             ID() const { return std::get<0>(data); }
   std::string const& Name() const { return std::get<1>(data); }
   std::string const& DBName() const { return std::get<2>(data); }
   std::string const& DataType() const { return std::get<3>(data); }
   size_t             Len() const { return std::get<4>(data); }
   size_t             Scale() const { return std::get<5>(data); }
   bool               NotNull() const { return std::get<6>(data); }
   bool               Primary() const { return std::get<7>(data); }

   std::string const& Comment() const { return std::get<8>(data); }

   TMyTable const& Table() const { return table;  }

   std::string Comment_Attribute() const;

  // std::tuple<std::string, bool, std::string, std::string, std::string> FullSourceInformations() const;

   std::string SQLRow(size_t len) const;
};

using myAttributes = std::vector<TMyAttribute>;

enum class EMyReferenceType : uint32_t { undefined = 0, generalization, range, assoziation, aggregation, komposition};

class TMyReferences {
   using myValues = std::pair<size_t, size_t>;
   using myData = std::tuple<std::string, EMyReferenceType, std::string, std::string, std::vector<myValues>>;
   myData data;
   TMyTable const& table;
public:
   TMyReferences() = delete;
   TMyReferences(TMyReferences const& other) : table(other.table), data(other.data) {  }
   ~TMyReferences() = default;

   auto operator <=> (TMyReferences const& other) const { return data <=> other.data; }

   auto const& GetKey() const { return std::get<0>(data); }

   TMyReferences(TMyTable const& pTable, std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                 std::string const& pComment, std::vector<myValues>&& pValues) :
      table{ pTable },
      data{ myData { pName, pRefType, pRefTable, pComment, std::move(pValues) } } { }

   std::string const& Name() const { return std::get<0>(data); }
   EMyReferenceType   ReferenceType() const { return std::get<1>(data); }
   std::string const& RefTable() const { return std::get<2>(data); }
   std::string const& Comment() const { return std::get<3>(data); }
   std::vector<myValues> const& Values() const { return std::get<4>(data); }

   std::string SQLRow() const;

   };

using myReferences = std::vector<TMyReferences>;


enum class EMyIndexType : uint32_t { undefined, key, unique, clustered, nonclustered };

class TMyIndices {
private:
   using myValues = std::pair<size_t, bool>;
   using myData = std::tuple<std::string, EMyIndexType, std::string, std::vector<myValues>>;
   myData data;
   TMyTable const& table;
public:
   TMyIndices() = delete;
   TMyIndices(TMyIndices const& other) : table(other.table), data(other.data) {  }
   ~TMyIndices() = default;

   auto operator <=> (TMyIndices const& other) const { return data <=> other.data; }

   auto const& GetKey() const { return std::get<0>(data); }

   TMyIndices(TMyTable const& pTable, std::string const& pName, EMyIndexType pIndexType, std::string const& pComment, std::vector<myValues>&& pValues) :
      table{ pTable },
      data{ myData { pName, pIndexType, pComment, std::move(pValues) } } { }

   std::string const& Name() const { return std::get<0>(data); }
   EMyIndexType       IndexType() const { return std::get<1>(data); }
   std::string const& Comment() const { return std::get<2>(data); }
   std::vector<myValues> const& Values() const { return std::get<3>(data); }

   std::string SQLRow() const;

};

using myIndices = std::vector<TMyIndices>;

enum class EMyEntityType : uint32_t { undefined, table, view };

class TMyTable {
private:
   using myData = std::tuple<std::string, EMyEntityType, std::string, std::string, std::string, std::string, std::string, 
                             std::string, std::string, myAttributes, myReferences, myIndices>;
   myData data;
   TMyDictionary const& dictionary;
public:
   TMyTable() = delete;
   TMyTable(TMyTable const& other) : dictionary(other.dictionary), data(other.data) { }
   //TMyTable(TMyTable&& other) noexcept : dictionary(std::move(other.dictionary)), data(std::move(other.data)) { }

   TMyTable(TMyDictionary const& dict, std::string const& pName, EMyEntityType pType, std::string const& pSQLName, std::string const& pSchema,
            std::string const& pSourceName, std::string const& pNamespace,     
            std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pComment) :
      dictionary { dict },
      data{ myData { pName, pType, pSQLName, pSchema, pSourceName, pNamespace, pSrcPath, pSQLPath, pComment, { }, { }, { } } } { }

   std::string const& Name() const { return std::get<0>(data); }
   EMyEntityType      EntityType() const { return std::get<1>(data); }
   std::string const& SQLName() const { return std::get<2>(data); }
   std::string const& SQLSchema() const { return std::get<3>(data); }
   std::string const& SourceName() const { return std::get<4>(data); }
   std::string        ClassName() const { return "T"s + SourceName(); }

   std::string const& Namespace() const { return std::get<5>(data); }

   std::string const& SrcPath() const { return std::get<6>(data); }
   std::string const& SQLPath() const { return std::get<7>(data); }

   std::string const& Comment() const { return std::get<8>(data); }

   std::string FullyQualifiedSQLName() const { return SQLSchema().size() > 0 ? SQLSchema() + "." + SQLName() : SQLName(); }
   std::string FullyQualifiedSourceName() const { return Namespace().size() > 0 ? Namespace() + "::" + SourceName() : SourceName(); }


   TMyDictionary const& Dictionary() const { return dictionary; }

   myAttributes&       Attributes() { return std::get<9>(data); }
   myAttributes const& Attributes() const { return std::get<9>(data); }

   myReferences&       References() { return std::get<10>(data); }
   myReferences const& References() const { return std::get<10>(data); }

   myIndices&          Indices() { return std::get<11>(data); }
   myIndices const&    Indices() const { return std::get<11>(data); }

   TMyAttribute const& FindAttribute(std::string const& strName) const;
   TMyAttribute const& FindAttribute(size_t iID) const;

   TMyTable& AddAttribute(int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
                          size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pComment);

   TMyTable& AddReference(std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                          std::string const& pComment, std::vector<std::pair<size_t, size_t>>&& pValues);

   TMyTable& AddIndex(std::string const& pName, EMyIndexType pIdxType, std::string const& pComment, std::vector<std::pair<size_t, bool>> && pValues);

   bool CreateHeader(std::ostream& os) const;
   bool CreateSource(std::ostream& os, bool boInline = false) const;
   bool CreateReadData(std::ostream& os, std::string const& data = "data");
   bool CreateWriteData(std::ostream& os, std::string const& data = "data");

   bool SQL_Create_Table(std::ostream& os) const;
   bool SQL_Create_Primary_Key(std::ostream& os) const;
   bool SQL_Create_Foreign_Keys(std::ostream& os) const;
   bool SQL_Create_Unique_Keys(std::ostream& os) const;
   bool SQL_Create_Indices(std::ostream& os) const;



   template <MySize_T... Args>
   std::vector<std::pair<size_t, size_t>> Funktion(std::string strName, Args&&... args) const {
      std::vector<std::pair<size_t, size_t>> result;

      std::vector<size_t> values1;
      std::vector<size_t> values2;
      std::string separator;

      bool foundSeparator = false;

      (([&](const auto& arg) {
         if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) [[unlikely]] {
            separator = arg;
            foundSeparator = true;
            }
         else if (foundSeparator) {
            values2.emplace_back(static_cast<size_t>(arg));
            }
         else {
            values1.emplace_back(static_cast<size_t>(arg));
            }
         }(args)), ...);

      // Überprüfe, ob die Gruppen die gleiche Länge haben
      if (values1.size() != values2.size()) {
         throw std::invalid_argument("Ungleich lange Gruppen von size_t-Werten.");
         }

      auto zipped = std::views::zip(values1, values2);
      std::copy(zipped.begin(), zipped.end(), std::back_inserter(result));


      return result;
      }

};

using myTables = std::map<std::string, TMyTable>;


/// \brief class represented the metadata collection
class TMyDictionary {
   std::string strName        = "default"s;   ///< name of the project
   std::string strDenotation  = ""s;          ///< denotation / denomination of the Project
   std::string strVersion     = "1.0"s;       ///< version id for the project
   std::string strDescription = ""s;          ///< description / synopsis for the project
   std::string strComment     = ""s;          ///< comments / notes for the project
   std::string strAuthor      = ""s;          ///< author 
   std::string strCopyright   = ""s;          ///< copyright statement for this project
   std::string strLicense     = ""s;          ///< additional statement for the license

   myDataTypes datatypes;                     ///< container with the datatypes for this project
   myTables    tables;                        ///< container with all tables inside of this project

   std::string strReaderClass = "TMyReader"s; ///< class name for the database reader 
   std::string strReaderFile  = "MyReader"s;  ///< file name for the database reader class


   fs::path    pathSource;                    ///< file path to the location of the source files
   fs::path    pathSQL;                       ///< file path to the location of the sql scripts
   fs::path    pathDoc;                       ///< file path to the location of the documentation files


public:

   TMyDictionary() = default;
   TMyDictionary(std::string const& pName, std::string const& pVersion = "1.0"s, std::string const& pComment = ""s) : 
        strName(pName), strVersion(pVersion), strComment(pComment) { }

   /** \name selectors for class TMyDictionary
       \{ */ 
   std::string const& Name() const { return strName; } ///< selector for the member strName
   std::string const& Version() const { return strVersion; }
   std::string const& Denotation() const { return strDenotation; }
   std::string const& Description() const { return strDescription; }
   std::string const& Comment() const { return strComment; }
   std::string const& Author() const { return strAuthor; }
   std::string const& Copyright() const { return strCopyright; }
   std::string const& License() const { return strLicense; }

   std::string const& ReaderClass() const { return strReaderClass; } 
   std::string const& ReaderFile() const { return strReaderFile;  }

   fs::path const&    SourcePath() const { return pathSource; }
   fs::path const&    SQLPath() const { return pathSQL; }
   fs::path const&    DocPath() const { return pathDoc; }

   std::string        Identifier() const;  ///< methode to generate a identifier from project name

   myDataTypes const& DataTypes() const { return datatypes; }; 
   myTables const&    Tables() const { return tables; };
   /// \}
   
   /** \name manipulators for class TMyDictionary
   \{ */
   std::string const& Name(std::string const& newVal) { return strName = newVal; } ///< selector for the member strName
   std::string const& Version(std::string const& newVal) { return strVersion = newVal; }
   std::string const& Denotation(std::string const& newVal) { return strDenotation = newVal; }
   std::string const& Description(std::string const& newVal) { return strDescription = newVal; }
   std::string const& Comment(std::string const& newVal) { return strComment = newVal; }
   std::string const& Author(std::string const& newVal) { return strAuthor = newVal; }
   std::string const& Copyright(std::string const& newVal) { return strCopyright = newVal; }
   std::string const& License(std::string const& newVal) { return strLicense = newVal; }

   std::string const& ReaderClass(std::string const& newVal) { return strReaderClass = newVal; }
   std::string const& ReaderFile(std::string const& newVal) { return strReaderFile = newVal; }

   fs::path const& SourcePath(fs::path const& newPath) { return pathSource = newPath; }
   fs::path const& SQLPath(fs::path const& newPath) { return pathSQL = newPath; }
   fs::path const& DocPath(fs::path const& newPath) { return pathDoc = newPath; }
   /// \}

   /** \name methods to work with the datatypes
   \{ */
   TMyDatatype const& FindDataType(std::string const& strDataType) const;

   TMyDatatype& AddDataType(std::string const& pDataType, std::string const& pDatabaseType, bool pUseLen, bool pUseScale,
                            std::string const& pSourceType, std::string const& pHeader, std::string const& pPrefix, 
                            std::string const& pInit, bool pUseReference, std::string const& pComment);
   /// \}


   /** \name methods to work with the tables
   \{ */
   TMyTable& FindTable(std::string const& strTable);
   TMyTable const& FindTable(std::string const& strTable) const;

   TMyTable& AddTable(std::string const& pName, EMyEntityType pType, std::string const& pSQLName, std::string const& pSchema,
                      std::string const& pSourceName, std::string const& pNamespace, 
                      std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pComment);
   /// \}


   void CreateTable(std::string const& strTable, std::ostream& out) const;
   void CreateClass(std::string const& strTable, std::ostream& out) const;

   void Create_All(std::ostream& out = std::cout, std::ostream& err = std::cerr) const;

};


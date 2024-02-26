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
#include <set>
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


class TMyDatatype;
class TMyDictionary;
class TMyTable;
class TMyReferences;
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
               std::string const& pCheck, std::string const& pSourceType, std::string const& pHeader, std::string const& pPrefix,
               bool pUseReference, std::string const& pComment) :
      dictionary { dict },
      data{ myData { pDataType, pDatabaseType, pUseLen, pUseScale, pCheck, pSourceType, pHeader, pPrefix, pUseReference, pComment } } { }

   auto const& GetKey() const { return std::get<0>(data); }

   std::string const& DataType() const { return std::get<0>(data); }
   std::string const& DatabaseType() const { return std::get<1>(data); }
   bool               UseLen() const { return std::get<2>(data); }
   bool               UseScale() const { return std::get<3>(data); }
   std::string const& CheckSeq() const { return std::get<4>(data); }

   std::string const& SourceType() const { return std::get<5>(data); }
   std::string const& Headerfile() const { return std::get<6>(data); }

   std::string const& Prefix() const { return std::get<7>(data); }
   
   bool               UseReference() const { return std::get<8>(data); }
   std::string const& Comment() const { return std::get<9>(data); }

};

using myDataTypes = std::map<std::string, TMyDatatype>;

enum class EMyCheckKinds : uint32_t { undefined = 0, direct, attribute, table };
enum class EMyCalculationKinds : uint32_t { undefined = 0, attribute, attribute_permanent, table, table_permanent };


class TMyAttribute {
private:
   using myData = std::tuple<int, std::string, std::string, std::string, size_t, size_t, bool, bool, std::string, 
                             EMyCheckKinds, std::string, std::string, EMyCalculationKinds, std::string, std::string, std::string>;
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
                size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pCheck, EMyCheckKinds pCheckKind,
                std::string const& pInit, std::string const& pComputed, EMyCalculationKinds pCalcKind, std::string const& pDenotation) :
      table { pTable },
      data{ myData { pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, pCheck, pCheckKind, pInit, pComputed, pCalcKind, pDenotation, "", "" } } { }

   size_t              ID() const { return std::get<0>(data); }
   std::string const&  Name() const { return std::get<1>(data); }
   std::string const&  DBName() const { return std::get<2>(data); }
   std::string const&  DataType() const { return std::get<3>(data); }
   size_t              Len() const { return std::get<4>(data); }
   size_t              Scale() const { return std::get<5>(data); }
   bool                NotNull() const { return std::get<6>(data); }
   bool                Primary() const { return std::get<7>(data); }
   std::string const&  CheckSeq() const { return std::get<8>(data); }
   EMyCheckKinds       CheckAtTable() const { return std::get<9>(data); }
   std::string const&  InitSeq() const { return std::get<10>(data); }
   std::string const&  Computed() const { return std::get<11>(data); }
   bool                IsComputed() const { return Computed().size() > 0; }
   EMyCalculationKinds KindOfCalulate() const { return std::get<12>(data); }

   std::string const& Denotation() const { return std::get<13>(data); }
   std::string const& Description() const { return std::get<14>(data); }
   std::string const& Comment() const { return std::get<15>(data); }

   TMyTable const& Table() const { return table;  }

   std::string Comment_Attribute() const;

   TMyAttribute& AddDescription(std::string const& pText);
   TMyAttribute& AddComment(std::string const& pText);

private:
   std::string& Denotation() { return std::get<13>(data); }
   std::string& Description() { return std::get<14>(data); }
   std::string& Comment() { return std::get<15>(data); }

};

// sorting order needed, do not change this container type
using myAttributes = std::vector<TMyAttribute>;

enum class EMyReferenceType : uint32_t { undefined = 0, generalization, range, assoziation, aggregation, composition };

/// \brief datatype for references of tables
class TMyReferences {
   /// \\brief internal type for pairs of ids with references
   using myValues = std::pair</** \brief attribute in own table */        size_t, 
                              /** \brief attribute in referenced table */ size_t>;

   /// \\brief internal type for all attributes in this class
   using myData = std::tuple<std::string, EMyReferenceType, std::string, std::string, 
                            std::string, std::optional<size_t>, std::string, std::vector<myValues>>;

   myData data; ///< internal data element 
   TMyTable const& table; ///< reference to the table which use this as foreign key

public:
   TMyReferences() = delete;
   TMyReferences(TMyReferences const& other) : table(other.table), data(other.data) {  }
   ~TMyReferences() = default;

   auto operator <=> (TMyReferences const& other) const { return data <=> other.data; }

   auto const& GetKey() const { return std::get<0>(data); }

   TMyReferences(TMyTable const& pTable, std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                 std::string const& pDescription, std::string const& pCardinality, std::optional<size_t> const& pShowAttribute,
                 std::string const& pComment, std::vector<myValues>&& pValues) :
      table{ pTable },
      data{ myData { pName, pRefType, pRefTable, pDescription, pCardinality,  pShowAttribute, pComment, std::move(pValues) } } { }

   std::string const& Name() const { return std::get<0>(data); }
   EMyReferenceType             ReferenceType() const { return std::get<1>(data); }
   std::string const&           RefTable() const { return std::get<2>(data); }
   std::string const&           Description() const { return std::get<3>(data); }
   std::string const&           Cardinality() const { return std::get<4>(data); }
   std::optional<size_t> const& ShowAttribute() const { return std::get<5>(data); }
   std::string const&           Comment() const { return std::get<6>(data); }
   std::vector<myValues> const& Values() const { return std::get<7>(data); }

   std::string SQLRow() const;
   std::string ReferenceTypeTxt() const;
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

enum class EMyEntityType : uint32_t { undefined, table, range, relationship, view };

using myStatements = std::vector<std::string>;


class TMyNameSpace {
private:
   using myData = std::tuple<std::string, std::string, std::string, std::string>;
   myData data;
   TMyDictionary const& dictionary;
public:
   TMyNameSpace() = delete;
   TMyNameSpace(TMyNameSpace const& other) : dictionary(other.dictionary), data(other.data) { }
   TMyNameSpace(TMyDictionary const& dict, std::string const& pName, std::string const& pDenotation, 
                std::string const& pDescription = "", std::string const& pComment = "") :
        dictionary(dict), data { myData { pName, pDenotation, pDescription, pComment } } { }

   /** \name Selectors for TMyTable
       \{ */
   std::string const& Name() const { return std::get<0>(data);  }
   std::string const& Denotation() const { return std::get<1>(data); }
   std::string const& Description() const { return std::get<2>(data); }
   std::string const& Comment() const { return std::get<3>(data); }
   /// \}

};

using myNameSpaces = std::map<std::string, TMyNameSpace>;

class TMyDirectory {
private:
   using myData = std::tuple<std::string, std::string, std::string>;
   myData data;
   TMyDictionary const& dictionary;
public:
   TMyDirectory() = delete;
   TMyDirectory(TMyDirectory const& other) : dictionary(other.dictionary), data(other.data) { }
   TMyDirectory(TMyDictionary const& dict, std::string const& pName, std::string const& pDenotation, std::string const& pDescription) :
      dictionary(dict), data{ myData { pName, pDenotation, pDescription } } { }

   /** \name Selectors for TMyTable
       \{ */
   std::string const& Name() const { return std::get<0>(data); }
   std::string const& Denotation() const { return std::get<1>(data); }
   std::string const& Description() const { return std::get<2>(data); }
   /// \}

};

using myDirectories = std::map<std::string, TMyDirectory>;


class TMyTable {
private:
   using myData = std::tuple<std::string, EMyEntityType, std::string, std::string, std::string, std::string, std::string, std::string, 
                             std::string, std::string, std::string, myAttributes, myReferences, myIndices, myStatements, myStatements, 
                             myStatements>;
   myData data;
   TMyDictionary const& dictionary;
 
   /// internal datatype with all composed tables and necessary informations about this
   using my_part_of_type = std::tuple<TMyTable, std::string, std::string, std::vector<size_t>, std::vector<std::pair<size_t, size_t>>>;

public:
   TMyTable() = delete;
   TMyTable(TMyTable const& other) : dictionary(other.dictionary), data(other.data) { }
   TMyTable(TMyTable&& other) noexcept : dictionary(other.dictionary), data(other.data) { }


   TMyTable(TMyDictionary const& dict, std::string const& pName, EMyEntityType pType, std::string const& pSQLName, std::string const& pSchema,
            std::string const& pSourceName, std::string const& pNamespace,     
            std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pDenotation) :
      dictionary { dict },
      data { myData { pName, pType, pSQLName, pSchema, pSourceName, pNamespace, pSrcPath, pSQLPath, pDenotation, ""s, ""s,
                     { }, { }, { }, { }, { }, { } } } { }




   /** \name Selectors for TMyTable
       \{ */
   std::string const& Name() const { return std::get<0>(data); }
   EMyEntityType      EntityType() const { return std::get<1>(data); }
   std::string const& SQLName() const { return std::get<2>(data); }
   std::string const& SQLSchema() const { return std::get<3>(data); }
   std::string const& SourceName() const { return std::get<4>(data); }
   std::string const& Namespace() const { return std::get<5>(data); }
   std::string        ClassName() const { return "T"s + SourceName(); }
   std::string        FullClassName() const { return Namespace() + "::"s + ClassName(); }


   
   std::string const& SrcPath() const { return std::get<6>(data); }
   std::string const& SQLPath() const { return std::get<7>(data); }

   std::string const& Denotation() const { return std::get<8>(data); }

   std::string const& Description() const { return std::get<9>(data); }
   std::string const& Comment() const { return std::get<10>(data); }

private:
   // keep thinks together 
   std::string&       Description() { return std::get<9>(data); }
   std::string&       Comment() { return std::get<10>(data); }

public:

   std::string FullyQualifiedSQLName() const { return SQLSchema().size() > 0 ? SQLSchema() + "." + SQLName() : SQLName(); }
   std::string FullyQualifiedSourceName() const { return Namespace().size() > 0 ? Namespace() + "::" + ClassName() : ClassName(); }

   std::string Doc_RefName() const { return "datamodel_table_"s + Name(); }

   std::string Include() const { return "\""s + (SrcPath().size() > 0 ? SrcPath() + "/"s : ""s) + SourceName() + ".h\""; }
   /// \}

   TMyDictionary const& Dictionary() const { return dictionary; }

   myAttributes&       Attributes() { return std::get<11>(data); }
   myAttributes const& Attributes() const { return std::get<11>(data); }

   myReferences&       References() { return std::get<12>(data); }
   myReferences const& References() const { return std::get<12>(data); }

   myIndices&          Indices() { return std::get<13>(data); }
   myIndices const&    Indices() const { return std::get<13>(data); }

   myStatements&       RangeValues() { return std::get<14>(data); }
   myStatements const& RangeValues() const { return std::get<14>(data); }

   myStatements&       PostConditions() { return std::get<15>(data); }
   myStatements const& PostConditions() const { return std::get<15>(data); }

   myStatements&       Cleanings() { return std::get<16>(data); }
   myStatements const& Cleanings() const { return std::get<16>(data); }

   TMyAttribute const& FindAttribute(std::string const& strName) const;
   TMyAttribute const& FindAttribute(size_t iID) const;


   std::vector<TMyTable> GetParents(EMyReferenceType ref_type = EMyReferenceType::generalization) const;
   std::vector<my_part_of_type>  GetPart_ofs(EMyReferenceType ref_type = EMyReferenceType::composition) const;
   std::vector<std::pair<TMyAttribute, TMyDatatype>> GetProcessing_Data() const;


   TMyTable& AddAttribute(int pID, std::string const& pName, std::string const& pDBName, std::string const& pDataType,
                          size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pCheck,
                          std::string const& pInit, std::string const& pComputed, std::string const& pDenotation);

   TMyTable& AddReference(std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                          std::string const& pDescription, std::string const& pCardinality, std::optional<size_t> const& pShowAttribute,
                          std::string const& pComment, std::vector<std::pair<size_t, size_t>>&& pValues);

   TMyTable& AddIndex(std::string const& pName, EMyIndexType pIdxType, std::string const& pComment, std::vector<std::pair<size_t, bool>> && pValues);

   TMyTable& AddRangeValue(std::string const& pStatement);
   TMyTable& AddPostConditions(std::string const& pStatement);
   TMyTable& AddCleanings(std::string const& pStatement);

   TMyTable& AddDescription(std::string const& pDescription);
   TMyTable& AddComment(std::string const& pComment);

   bool CreateHeader(std::ostream& os) const;
   bool CreateSource(std::ostream& os, bool boInline = false) const;
   bool CreateReadData(std::ostream& os, std::string const& data = "data");
   bool CreateWriteData(std::ostream& os, std::string const& data = "data");

   bool CreateDox(std::ostream& os) const;

   myStatements Create_Table_Statements() const;
   myStatements Create_View_Statements() const;
   myStatements Create_Alter_Table_Statements() const; 
   myStatements Create_Primary_Key_Statements() const;
   myStatements Create_Foreign_Keys_Statements() const;
   myStatements Create_Unique_Keys_Statements() const;
   myStatements Check_Conditions_Statements() const;
   myStatements Create_Indices_Statements() const;
   myStatements Create_RangeValues_Statements() const;
   myStatements Create_PostConditions_Statements() const;
   myStatements Create_Cleaning_Statements() const;

   bool SQL_Create_Table(std::ostream& os) const;
   bool SQL_Create_View(std::ostream& os) const;
   bool SQL_Create_Alter_Table(std::ostream& os) const;
   bool SQL_Create_Primary_Key(std::ostream& os) const;
   bool SQL_Create_Foreign_Keys(std::ostream& os) const;
   bool SQL_Create_Unique_Keys(std::ostream& os) const;
   bool SQL_Create_Indices(std::ostream& os) const;
   bool SQL_Create_Check_Conditions(std::ostream& os) const;
   bool SQL_Create_RangeValues(std::ostream& os) const;
   bool SQL_Create_PostConditions(std::ostream& os) const;
   bool SQL_Drop_Tables(std::ostream& os) const;
   bool SQL_Create_Cleaning(std::ostream& os) const;

   bool SQL_Create_Descriptions(std::ostream& os) const;

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
   //private:
      std::set<std::string> GetPrecursors(bool boAll = true) const;
      std::set<std::string> GetSuccessors(bool boAll = true) const;
};

using myTables = std::map<std::string, TMyTable>;


/// \brief class represented the metadata collection
class TMyDictionary {
   std::string   strName        = "default"s;   ///< name of the project
   std::string   strDenotation  = ""s;          ///< denotation / denomination of the Project
   std::string   strVersion     = "1.0"s;       ///< version id for the project
   std::string   strDescription = ""s;          ///< description / synopsis for the project
   std::string   strComment     = ""s;          ///< comments / notes for the project
   std::string   strAuthor      = ""s;          ///< author 
   std::string   strCopyright   = ""s;          ///< copyright statement for this project
   std::string   strLicense     = ""s;          ///< additional statement for the license

   std::string   strReaderClass = "TMyReader"s; ///< class name for the database reader 
   std::string   strReaderFile  = "MyReader"s;  ///< file name for the database reader class


   fs::path      pathSource;                    ///< file path to the location of the source files
   fs::path      pathSQL;                       ///< file path to the location of the sql scripts
   fs::path      pathDoc;                       ///< file path to the location of the documentation files

   std::string strBaseClass;                    ///< BaseClass for all Classes. BaseClass created and used if set
   std::string strBaseNamespace;                ///< namespace for the BaseClass 
   fs::path    pathToBase;                      ///< path with filename to BaseClass, relative to SourcePath !! 


   myDataTypes   datatypes;                     ///< container with the datatypes for this project
   myTables      tables;                        ///< container with all tables inside of this project
   myNameSpaces  namespaces;                    ///< container with all defined namespaces of this project
   myDirectories directories;                   ///< container with defined subdirectories of this project
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

   std::string        Identifier() const;  ///< methode to generate a identifier from project name (remove spaces)

   myDataTypes const& DataTypes() const { return datatypes; }; 
   myTables const&    Tables() const { return tables; };

   bool               UseBaseClass() const { return strBaseClass.size() > 0; }
   std::string const& BaseClass() const { return strBaseClass; }
   std::string const& BaseNamespace() const { return strBaseNamespace; } 
   fs::path const&    PathToBase() const { return pathToBase; }

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

   std::string const& BaseClass(std::string const& newVal) { return strBaseClass = newVal; }
   std::string const& BaseNamespace(std::string const& newVal) { return strBaseNamespace = newVal; }
   fs::path const& PathToBase(fs::path const& newVal) { return pathToBase = newVal; }

   /// \}

   /** \name methods to work with the datatypes
   \{ */
   TMyDatatype const& FindDataType(std::string const& strDataType) const;

   TMyDatatype& AddDataType(std::string const& pDataType, std::string const& pDatabaseType, bool pUseLen, bool pUseScale,
                            std::string const& pCheck, std::string const& pSourceType, std::string const& pHeader, 
                            std::string const& pPrefix, bool pUseReference, std::string const& pComment);
   /// \}


   /** \name methods to work with the tables
   \{ */
   TMyTable& FindTable(std::string const& strTable);
   TMyTable const& FindTable(std::string const& strTable) const;

   TMyTable& AddTable(std::string const& pName, EMyEntityType pType, std::string const& pSQLName, std::string const& pSchema,
                      std::string const& pSourceName, std::string const& pNamespace, 
                      std::string const& pSrcPath, std::string const& pSQLPath, std::string const& pDenotation);
   /// \}

   TMyNameSpace&       FindNameSpace(std::string const& pName);
   TMyNameSpace const& FindNameSpace(std::string const& pName) const;
   TMyDictionary&      AddNameSpace(std::string const& pName, std::string const& pDenotation, 
                                    std::string const& pDescription = "", std::string const& pComment = "");

   TMyDirectory&       FindDirectory(std::string const& pName);
   TMyDirectory const& FindDirectory(std::string const& pName) const;
   TMyDictionary&      AddDirectory(std::string const& pName, std::string const& pDenotation, std::string const& pDescription);

   void CreateTable(std::string const& strTable, std::ostream& out) const;
   void CreateClass(std::string const& strTable, std::ostream& out) const;

   void Create_Doxygen(std::ostream&) const;
   void Create_Doxygen_SQL(std::ostream&) const;
   bool Create_SQL_Tables(std::ostream&) const;
   bool Create_SQL_Additionals(std::ostream&) const;
   bool Create_SQL_RangeValues(std::ostream&) const;

   void Create_SQL_Documentation(std::ostream&) const;

   bool SQL_Drop_Tables(std::ostream& os) const;

   bool CreateBaseHeader(std::ostream& out = std::cout) const;
   void Create_All(std::ostream& out = std::cout, std::ostream& err = std::cerr) const;

   void Test() const;
};


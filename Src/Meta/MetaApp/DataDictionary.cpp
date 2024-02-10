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

std::string TMyAttribute::SQLRow(size_t len) const {
   auto const& datatype = Table().Dictionary().FindDataType(DataType());
   return std::format("   {:<{}} {}{}{}", DBName(), len, datatype.DatabaseType(),
                        (datatype.UseLen() && datatype.UseScale() ? std::format("({}, {})", Len(), Scale()) :
                        (datatype.UseLen() ? std::format("({})", Len()) : ""s)), (NotNull() ? " NOT NULL"s : ""s));
   }


std::string TMyAttribute::Comment_Attribute() const {
   if (Comment().size() > 0) return Comment();
   else {
      std::ostringstream sComment;
      sComment << "attribute \"" << Name() << "\" in entity \"" << table.SourceName() << "\"";
      return sComment.str();
      }
   }

// TMyReferences

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
   size_t pLen, size_t pScale, bool pNotNull, bool pPrimary, std::string const& pComment) {
   Attributes().emplace_back(TMyAttribute(*this, pID, pName, pDBName, pDataType, pLen, pScale, pNotNull, pPrimary, pComment));
   return *this;
   }

TMyTable& TMyTable::AddReference(std::string const& pName, EMyReferenceType pRefType, std::string const& pRefTable, 
                                 std::string const& pComment, std::vector<std::pair<size_t, size_t>> && pValues) {
   References().emplace_back(TMyReferences(*this, pName, pRefType, pRefTable, pComment, std::move(pValues)));
   return *this;
   }

TMyTable& TMyTable::AddIndex(std::string const& pName, EMyIndexType pIdxType, std::string const& pComment, std::vector<std::pair<size_t, bool>>&& pValues) {
   Indices().emplace_back(TMyIndices(*this, pName, pIdxType, pComment, std::move(pValues)));
   return *this;
   }


bool TMyTable::CreateHeader(std::ostream& os) const {
   static const std::string strTab = "      ";
   static auto constexpr StartNameDoc = [](size_t tab, std::string const& strName, std::string const& strClass) {
      return std::format("{:>{}}/** \\name {} for class {}\n        \\{{ */\n", ' ', tab, strName, strClass);
      };

   static auto constexpr EndNameDoc = [](size_t tab) {
      return std::format("{:>{}}/// \\}}\n\n", ' ', tab);
      };

   try {
      os << "/** \\file\n"
         << "  * \\brief Definition for the dataclass of table " << Name() << " in dictionary " << Dictionary().Name() << '\n';
      if (Comment().size() > 0)
         os << "  * \\details " << Comment() << '\n';

      os << "  * \\version " << Dictionary().Version() << '\n'
         << "  * \\author " << Dictionary().Author() << '\n'
         << "  * \\date " << CurrentDate() << "  file created with adecc Scholar metadata generator\n";

      if (Dictionary().Copyright().size() > 0) {
         os << "  * \\copyright " << Dictionary().Copyright() << '\n';
         if(Dictionary().License().size() > 0) {
            os << "  * " << Dictionary().License() << '\n';
            }
         }
      os << "\n */\n\n";

      auto types = Attributes() | std::views::transform([this](auto const& s) { 
                                    auto const& dt = Dictionary().FindDataType(s.DataType());
                                    return dt.Headerfile(); })
                                | std::ranges::to<std::set<std::string>>();
                                    
      std::ranges::for_each(types | std::views::filter([](const std::string& line) {
                                            return !line.empty(); }), [&os](const std::string& line) {
                                                                           os << "#include " << line << '\n'; 
                                                                           });
      os << '\n';

      auto parents = References() | std::views::filter([](auto const& r) { return r.ReferenceType() == EMyReferenceType::generalization; })
                                  | std::views::transform([this](auto const& r) { return Dictionary().FindTable(r.RefTable()); });

      auto processing_data = Attributes() | std::views::transform([this](auto const& attr) {
                                                 return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); })
                                          | std::ranges::to<std::vector>();
         
      if(!parents.empty()) {
         std::ranges::for_each(parents, [&os](auto const& p) { os << "#include \"" << p.SrcPath() << "/" << p.SourceName() << ".h\"\n"; });
         }

      os << "#include <optional>\n\n";

      bool boHasNamespace = Namespace().size() > 0;

      if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

      os << "/// " << Comment() << "\n";
      os << "class " << ClassName();
      if (!parents.empty()) {
         size_t i = 0;
         std::ranges::for_each(parents, [&os, &i](auto const& p) { os << (i++ > 0 ? ", public " : ": public ") << p.ClassName(); });
         }
      os << " { \n" 
         << "   private:\n";

      auto maxElement = std::ranges::max_element(processing_data, [](auto const& a, auto const& b) { 
                                                            return a.second.SourceType().size() < b.second.SourceType().size(); });

      size_t maxLengthType = 0;
      if (maxElement != processing_data.end()) {
         maxLengthType = maxElement->second.SourceType().size() + 1;
         }

      maxElement = std::ranges::max_element(processing_data, [](auto const& a, auto const& b) {
                                                          static auto constexpr len = [](auto const& e) {
                                                              return e.second.SourceType().size() + (e.second.UseReference() ? 9 : 1);
                                                              };
                                                          return len(a) < len(b); 
                                                          });

      size_t maxLengthRet = 0;
      if (maxElement != processing_data.end()) {
         maxLengthRet = maxElement->second.SourceType().size();
         maxLengthRet += maxElement->second.UseReference() ? 9 : 1;
         }


      maxElement = std::ranges::max_element(processing_data, [](auto const& a, auto const& b) {
                                         static auto constexpr len = [](auto const& e) {
                                            return e.second.Prefix().size() + e.first.Name().size();
                                            };
                                         return len(a) < len(b); 
                                         });
      size_t maxLengthAttr = 0;
      if (maxElement != processing_data.end()) {
         maxLengthAttr = maxElement->second.Prefix().size() + maxElement->first.Name().size() + 1;
         }

      os << "\n   private:\n";
      
      // ------------------ generate the attributes for the table  -----------------------------------
      for (auto const& [attr, dtype] : processing_data) {
         std::string strType      = "std::optional<"s + dtype.SourceType() + ">"s;
         std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
         std::string strComment   = attr.Comment_Attribute();
         //os << std::format("{0}{1:<{2}}{3:<{4}} ///< {5}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr, strComment);
         os << std::format("{0}/// {5}\n{0}{1:<{2}}{3:<{4}}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr, strComment);
      }

      os << "\n   public:\n" << StartNameDoc(6, "constructors and destructor",ClassName());
      os << std::format("{0}{1:}();\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} const&);\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} &&);\n\n", strTab, ClassName())
         << std::format("{0}virtual ~{1:}();\n\n", strTab, ClassName());
      os << EndNameDoc(6);

      os << StartNameDoc(6, "virtual functions",ClassName());
      os << std::format("{0}virtual void init();\n", strTab)
         << std::format("{0}virtual void copy({1} const& other);\n\n", strTab, ClassName());
      os << EndNameDoc(6);

      // ------------------ generate the selectors for the table  -----------------------------------
      os << StartNameDoc(6, "selectors", ClassName());
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
         std::string strSelector = attr.Name();
         std::string strAttribute = dtype.Prefix() + attr.Name() + ">";
         std::string strComment = attr.Comment_Attribute();
         os << std::format("{0}/// selector with std::optional as retval for {5}\n{0}{1:<{2}}{3}() const {{ return {4}; }}\n", strTab, strRetType, maxLengthType + 22, strSelector, strAttribute, strComment);
         }
      os << "\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType   = dtype.SourceType() + (dtype.UseReference() ?  " const&"s : ""s);
         std::string strSelector  = attr.Name();
         std::string strComment   = attr.Comment_Attribute();
         os << std::format("{0}///< selector with value for {4}\n{0}{1:<{2}}_{3}() const;\n", strTab, strRetType, maxLengthRet, strSelector, strComment);
         }
      os << EndNameDoc(6);

      // ------------------ generate the manipulators for the table  -----------------------------------
      os << StartNameDoc(6, "manipulators", ClassName());
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = "std::optional<"s + dtype.SourceType() + (dtype.UseReference() ? "> const&"s : ">"s);
         std::string strManipulator = attr.Name();
         std::string strAttribute   = dtype.Prefix() + attr.Name();
         std::string strComment     = attr.Comment_Attribute();
         //os << std::format("{0}{1:<{2}}{3}({1} newVal); ///< manipulator for {4}\n", strTab, strRetType, maxLengthRet + 15, strManipulator, strComment);
         os << std::format("{0}/// manipulator for {4}\n{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthRet + 15, strManipulator, strComment);
         }
      os << EndNameDoc(6);

      os << "   private:\n"
         << StartNameDoc(6, "internal functions", ClassName())
         << "      void _init();\n"
         << "      void _copy(" << ClassName() << " const& other);\n";
      os << EndNameDoc(6);

      os << "   };\n\n";

      os << "// Implementations of the special selectors for return values instead std::optional\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
         std::string strSelector = attr.Name();
         std::string strAttribute = dtype.Prefix() + attr.Name();
         std::string strComment = attr.Comment_Attribute();
         std::string strReturn = std::format("   if({0}) [[likely]] return {0}.value();\n"
                                             "   else throw std::runtime_error(\"value for attribute \\\"{1}\\\" in class \\\"{2}\\\" is empty.\");",
                                             strAttribute, attr.Name(), ClassName());
         os << std::format("inline {1} {0}::_{2}() const {{\n{3};\n   }}\n\n", ClassName(), strRetType, strSelector, strReturn);
      }

      

      os << '\n';
      os << "// Implementations of the manipulators\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = dtype.UseReference() ? dtype.SourceType() + " const&"s : dtype.SourceType();
         std::string strManipulator = ClassName() + "::"s + attr.Name();
         std::string strAttribute   = dtype.Prefix() + attr.Name();
         os << std::format("inline {0} {1}({0} newVal) {{\n   return {2} = newVal;\n   }}\n\n", strRetType, strManipulator, strAttribute);
         }


      if (boHasNamespace) os << "} // end of namespace " << Namespace() << "\n";
      }
   catch(std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
      }
   return true;
   }

bool TMyTable::CreateSource(std::ostream& os, bool boInline) const {
   const std::string strTab = "   "s;
   try {
      auto parents = References() | std::views::filter([](auto const& r) { return r.ReferenceType() == EMyReferenceType::generalization; })
                                  | std::views::transform([this](auto const& r) { return Dictionary().FindTable(r.RefTable()); });

      auto processing_data = Attributes() | std::views::transform([this](auto const& attr) {
                                                      return std::make_pair(attr, Dictionary().FindDataType(attr.DataType())); }) 
                                          | std::ranges::to<std::vector>();

      auto maxElement = std::ranges::max_element(processing_data, [](auto const& a, auto const& b) {
            static auto constexpr len = [](auto const& e) { return e.second.Prefix().size() + e.first.Name().size(); };
            return len(a) < len(b);
            });

      size_t maxLengthAttr = 0;
      if (maxElement != processing_data.end()) {
         maxLengthAttr = maxElement->second.Prefix().size() + maxElement->first.Name().size() + 1;
         }


      os << "/** \\file\n"
         << "  * \\brief Definition for the dataclass of table " << Name() << " in dictionary " << Dictionary().Name() << '\n';
      if (Comment().size() > 0)
         os << "  * \\details " << Comment() << '\n';

      os << "  * \\version " << Dictionary().Version() << '\n'
         << "  * \\author " << Dictionary().Author() << '\n'
         << "  * \\date " << CurrentDate() << "  file created with adecc Scholar metadata generator\n";

      if (Dictionary().Copyright().size() > 0) {
         os << "  * \\copyright " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0) {
            os << "  * " << Dictionary().License() << '\n';
            }
         }
      os << " */\n\n";

      os << "#include \"" << SrcPath() << "/" << SourceName() << ".h\"\n\n";

      bool boHasNamespace = Namespace().size() > 0;

      if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

      // create the default constructor for the class

      os << ClassName() << "::" << ClassName() << "()";
      if (!parents.empty()) {
         size_t i = 0;
         std::ranges::for_each(parents, [&os, &i](auto const& p) { os << (i++ > 0 ? ", " : " : ") << p.ClassName() << "()"; });
         }

      os << " {\n" 
         << strTab << "_init();\n" 
         << strTab << "}\n\n";

      // create the copy constructor for the class

      os << ClassName() << "::" << ClassName() << "(" << ClassName() << " const& other)";
      if (!parents.empty()) {
         size_t i = 0;
         std::ranges::for_each(parents, [&os, &i](auto const& p) { os << (i++ > 0 ? ", " : " : ") << p.ClassName() << "(other)"; });
         }

      os << "{\n" 
         << strTab << "_copy(other);\n" 
         << strTab << "}\n\n";

      // _init: internal initialization method for the class

      os << "void " << ClassName() << "::_init() {\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strAttribute = dtype.Prefix() + attr.Name();
         std::string strInit      = dtype.InitSeq(); //attr.InitSeq();
         os << std::format("   {0:<{1}} = {2};\n", strAttribute, maxLengthAttr, strInit);
         }
      os << "   return;\n"
         << "   };\n\n";

      os << "void " << ClassName() << "::_copy(" << ClassName() << " const& other) {\n";
      for (auto const& [attr, dtype] : processing_data) {
         os << std::format("   {0}(other.{0}());\n", attr.Name());
         }
      os << "   return;\n"
         << "   };\n\n";

      if (boHasNamespace) os << "} // end of namespace " << Namespace() << "\n";

      }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
      }
   return true;
   }


bool TMyTable::CreateReadData(std::ostream& os, std::string const& data) {
   return true;
   }

bool TMyTable::CreateWriteData(std::ostream& os, std::string const& data) {
   return true;
   }

// -------------------------------------------------------------------------------------------------
// sql statements
// -------------------------------------------------------------------------------------------------

bool TMyTable::SQL_Create_Table(std::ostream& os) const {
   try {
      os << "CREATE TABLE " << FullyQualifiedSQLName() << " (\n";

      auto maxLength = std::ranges::max_element(Attributes(), [](auto const& a, auto const& b) { return a.DBName().size() < b.DBName().size(); });
      if (maxLength != Attributes().end()) {
         auto len = maxLength->DBName().size();
         for (auto const& attribute : Attributes() | std::ranges::views::take(Attributes().size() - 1)) {
            os << attribute.SQLRow(len) << ",\n";
            }
         os << Attributes().back().SQLRow(len) << '\n';
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
      if(i > 0) os << tmp_os.str() <<  ");\n";
      }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
      }   
   return true;
   }

bool TMyTable::SQL_Create_Foreign_Keys(std::ostream& os) const {
   try {
      if(!References().empty()) {
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
                                        std::string const& pSourceType, std::string const& pHeader, std::string const& pPrefix, 
                                        std::string const& pInit, bool pUseReference, std::string const& pComment) {
   TMyDatatype datatype(*this, pDataType, pDatabaseType, pUseLen, pUseScale, pSourceType, pHeader, pPrefix, pInit, pUseReference, pComment);

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

      of_sql << "\n-- create indices for tables of the dictionary " << Name() << '\n';
      for (auto const& [name, table] : Tables()) {
         out << "addtional sql, create indices for table: " << name << " ... ";
         table.SQL_Create_Indices(of_sql);
         out << "done\n";
         }

      of_sql.close();


      fs::create_directories(SourcePath());
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
         out << "done.\n";
         }
      }
   catch(std::exception& ex) {
      err << ex.what() << '\n';
      }
   }
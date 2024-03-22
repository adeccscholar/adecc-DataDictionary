#include "DataDictionary.h"

#include <iostream>
#include <format>

using namespace std::string_literals;

bool TMyTable::CreateReadData(std::ostream& os) {
   return true;
   }

bool TMyTable::CreateWriteData(std::ostream& os) {
   return true;
   }


bool TMyDictionary::CreateSQLStatementHeader(std::ostream& os) const {
   if (HasPersistenceClass()) [[likely]] {
      // write the comment for the base class when used
      // -----------------------------------------------------------------------------------------
      os << "/*\n"
         << "* Project: " << Denotation() << "\n"
         << "* Definition of sql statement for access with class " << PersistenceClass() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
      if (Copyright().size() > 0) os << "* copyright (c) " << Copyright() << '\n';
      if (License().size() > 0)   os << "* " << License() << '\n';
      os << "*/\n\n"
         << "#pragma once\n\n"
         << "#include <string>\n\n";

      if (PersistenceNamespace().size() > 0) os << "namespace " << PersistenceNamespace() << " {\n\n";

      for (auto const& [_, table] : Tables()) {
         os << "extern const std::string strSQL" << table.Name() << "SelectAll;\n"
            << "extern const std::string strSQL" << table.Name() << "SelectDetail;\n"
            << "extern const std::string strSQL" << table.Name() << "DeleteAll;\n"
            << "extern const std::string strSQL" << table.Name() << "DeleteDetail;\n"

            ;
         }

      if (PersistenceNamespace().size() > 0) os << "} // close namespace " << PersistenceNamespace() << "\n";

      return true;
      }
   else return false;
   }

bool TMyDictionary::CreateSQLStatementSource(std::ostream& os) const {
   if (HasPersistenceClass()) [[likely]] {
      // write the comment for the base class when used
      // -----------------------------------------------------------------------------------------
      os << "/*\n"
         << "* Project: " << Denotation() << "\n"
         << "* Implementation of sql statement for access with class " << PersistenceClass() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
      if (Copyright().size() > 0) os << "* copyright © " << Copyright() << '\n';
      if (License().size() > 0)   os << "* " << License() << '\n';
      os << "*/\n\n"
         << "#include <" << (PathToPersistence() / (PersistenceName() + "_sql.h"s)).string() << ">\n\n";

      if (PersistenceNamespace().size() > 0) os << "namespace " << PersistenceNamespace() << " {\n\n";

      for (auto const& [_, table] : Tables()) {
         std::ostringstream tmp, tmpDetail;
         auto prim_keys = table.Attributes() | std::views::filter([](auto const& a) { return a.Primary(); });
         auto maxPrimElement = std::ranges::max_element(prim_keys, [](auto const& a, auto const& b) {
            return a.DBName().size() < b.DBName().size(); });

         size_t maxLengthPrimAttr = 0;
         if (maxPrimElement != prim_keys.end()) {
            maxLengthPrimAttr = maxPrimElement->DBName().size();
            }

         os << "// --------------------------------------------------------------------\n"
            << "//   Statements for table / view: " << table.Name() << "\n"
            << "// --------------------------------------------------------------------\n";
         tmp << "     \"SELECT ";
         tmp << std::format("{0}", std::begin(table.Attributes())->DBName());
         size_t len = std::begin(table.Attributes())->DBName().size();
         for (auto const& attrib : table.Attributes() | std::views::drop(1)) {
            if(len > 60) {
               tmp << ",\\n\"\n     \"       ";
               len = attrib.DBName().size();
               }
            else {
               tmp << ", ";
               len += attrib.DBName().size();
               }
            tmp << std::format("{0}", attrib.DBName());
            }
         tmp << "\\n\"\n"
             << "     \"FROM " << table.FullyQualifiedSQLName();
         os << "const std::string strSQL" << table.Name() << "SelectAll = \n" << tmp.str() << "\";\n\n";
         os << "const std::string strSQL" << table.Name() << "SelectDetail = \n" << tmp.str()
            << "\\n\"\n";
         tmpDetail << "     \"WHERE " << std::format("{0:<{1}} = :key{0}", std::begin(prim_keys)->DBName(), maxLengthPrimAttr);
         for (auto const& attrib : prim_keys | std::views::drop(1)) 
            tmpDetail << std::format(" AND\\n\"\n     \"      {0:<{1}} = :key{0}", attrib.DBName(), maxLengthPrimAttr);
         
         os << tmpDetail.str() << "\";\n\n";

         tmp.str("");
         tmp << "     \"DELETE FROM " << table.FullyQualifiedSQLName();
         os << "const std::string strSQL" << table.Name() << "DeleteAll = \n"
            << tmp.str() << "\";\n\n";
         
         os << "const std::string strSQL" << table.Name() << "DeleteDetail = \n"
            << tmp.str() << "\\n\"\n";
         os << tmpDetail.str() << "\";\n\n";

         os << "\n";
         }

      if (PersistenceNamespace().size() > 0) os << "\n} // close namespace " << PersistenceNamespace() << "\n";

      return true;
      }
   else return false;
   }


// ------------------------------------------------------------------------------------------------
/// Create the Header file for the reader class of this dictionary
bool TMyDictionary::CreateReaderHeader(std::ostream& os) const {
   if (HasPersistenceClass()) [[likely]] {
      // write the comment for the base class when used
      // -----------------------------------------------------------------------------------------
      os << "/*\n"
         << "* Project: " << Denotation() << "\n"
         << "* Definition of the persistence class " << PersistenceClass() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
      if (Copyright().size() > 0) os << "* copyright (c) " << Copyright() << '\n';
      if (License().size() > 0)   os << "* " << License() << '\n';
      os << "*/\n\n"
         << "#pragma once\n\n";

      os << "#include <vector>\n"
         << "#include <map>\n\n";

      for (auto const& [_, table] : Tables()) {
         os << "#include <" << (table.SrcPath().size() > 0 ? table.SrcPath() + "\\"s : ""s) + table.SourceName() + ".h>\n"s;
         }

      os << "\n"
         << "#include <adecc_Database/MyDatabase.h>\n\n"
         << "#include <string>\n"
         << "#include <memory>\n\n";


      if (PersistenceNamespace().size() > 0) os << "namespace " << PersistenceNamespace() << " {\n\n";

      os << "// data access class for the " << Denotation() << " project.\n"
         << "class " << PersistenceClass() << " {\n"
         << "   private:\n"
         << "      using concrete_db_server = " << PersistenceServerType() << ";\n"  // could be TMyMSSQL TMyMySQL TMyOracle TMyInterbase TMySQLite
         << "      using concrete_framework = TMyQtDb<concrete_db_server>;\n"
         << "      using concrete_db_connection = TMyDatabase<TMyQtDb, concrete_db_server>;\n"
         << "      using concrete_query = TMyQuery<TMyQtDb, concrete_db_server>;\n\n"
         << "      // member to produce the data access\n"
         << "      concrete_db_connection database;\n\n"
         << "   public:\n"
         << "      " << PersistenceClass() << "();\n"
         << "      " << PersistenceClass() << "(" << PersistenceClass() << " const&) = delete;\n"
         << "      " << PersistenceClass() << "(" << PersistenceClass() << "&&) noexcept = delete;\n"
         << "      ~" << PersistenceClass() << "() = default;\n\n";

      os << "      bool IsConnectedToDatabase() const { return database.Connected(); }\n"
         << "      bool GetServerHasIntegratedSecurity(void) const { return database.HasIntegratedSecurity(); }\n"
         << "      std::string GetDatabaseInformations(void) const;\n"
         << "      std::pair<std::string, std::string> GetConnectionInformations(void) const;\n"
         << "      std::pair<bool, std::string> LoginToDb(TMyCredential && credentials);\n"
         << "      void LogoutFromDb(void);\n\n";



      for(auto const& [_, table] : Tables()) {
         os << std::format("      // access methods for class {}\n", table.ClassName());
         //os << std::format("      bool Read(std::map<{0}::primary_key, {0}>&);\n", table.FullClassName());
         os << std::format("      bool Read({0}::container_ty&);\n", table.FullClassName());
         os << std::format("      bool Read({0}::primary_key const&, {0}&);\n", table.FullClassName());
         os << "\n";
         }

      os << "   };\n";

      if (PersistenceNamespace().size() > 0) os << "\n} // close namespace " << PersistenceNamespace() << "\n";

      }
    return true;
   }

bool TMyDictionary::CreateReaderSource(std::ostream& os) const {
   if (HasPersistenceClass()) [[likely]] {
      os << "/*\n"
         << "* Project: " << Denotation() << "\n"
         << "* Definition of the persistence class " << PersistenceClass() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
      if (Copyright().size() > 0) os << "* copyright (c) " << Copyright() << '\n';
      if (License().size() > 0)   os << "* " << License() << '\n';
      os << "*/\n\n"
         << "#include <" << (PathToPersistence() / (PersistenceName() + "_sql.h"s)).string() << ">\n"
         << "#include <" << (PathToPersistence() / (PersistenceName() + ".h"s)).string() << ">\n\n"
         << "#include <adecc_Database\\MyDatabaseExceptions.h>\n\n";

      /*
      for (auto const& [_, table] : Tables()) {
         os << "#include <" << (table.SrcPath().size() > 0 ? table.SrcPath() + "\\"s : ""s) + table.SourceName() + ".h>\n"s;
         }
      os << "\n";
      */
      if (PersistenceNamespace().size() > 0) os << "namespace " << PersistenceNamespace() << " {\n\n";
   


      
      os << PersistenceClass() << "::" << PersistenceClass() << "() "
         <<                   ": database { " << PersistenceServerType() << " { \"" << PersistenceDatabase() << "\" } } { }\n"
         << "\n"
         << "std::pair<std::string, std::string> " << PersistenceClass() << "::GetConnectionInformations(void) const {\n"
         << "   return { database.GetDatabase(), database.ServerType() };\n"
         << "   }\n"
         << "\n"
         << "std::string " << PersistenceClass() << "::GetDatabaseInformations(void) const {\n"
         << "   return database.GetInformations();\n"
         << "   }\n"
         << "\n"
         << "std::pair<bool, std::string> " << PersistenceClass() << "::LoginToDb(TMyCredential && credentials) {\n"
         << "   try {\n"
         << "      database += std::move(credentials);\n"
         << "      database.Open();\n"
         << "      return { true, database.GetInformations() };\n"
         << "      }\n"
         << "   catch (TMy_Db_Exception& ex) {\n"
         << "      return { false, \"error while login to database\\n\"s + ex.information() };\n"
         << "      }\n"
         << "   catch (std::exception& ex) {\n"
         << "      return { false, \"error while login to database\\n\"s + ex.what() };\n"
         << "      }\n"
         << "   }\n"
         << "\n"
         << "void " << PersistenceClass() << "::LogoutFromDb() {\n"
         << "   database.Close();\n"
         << "   }\n"
         << "\n";



      for (auto const& [_, table] : Tables()) {
         os << std::format("// access methods for class {}\n", table.ClassName());
         os << std::format("bool {1}::Read({0}::container_ty& data) {{\n", table.FullClassName(), PersistenceClass());
         os << "   auto query = database.CreateQuery();\n"
            << "   query.SetSQL(" << "strSQL" << table.Name() << "SelectAll);\n"
            << "   for(query.Execute(), query.First();!query.IsEof();query.Next()) {\n"
            << "      " << table.FullClassName() << " element;\n";
         for(auto const& attr : table.Attributes()) {
            auto const& datatype = table.Dictionary().FindDataType(attr.DataType());
            os << "      element." << attr.Name() << "(query.Get<" << datatype.SourceType() << ">(\"" << attr.Name() << "\"";
            if (attr.Primary()) os << ", true";
            os << "));\n";
            }
         os << "      data.insert({ element.GetKey(), element });\n"
            << "      }\n";

         os << "   return true;\n"
            << "   }\n\n";
         os << std::format("bool {1}::Read({0}::primary_key const& key_val, {0}& data) {{\n", table.FullClassName(), PersistenceClass());
         os << "   auto query = database.CreateQuery();\n"
            << "   query.SetSQL(" << "strSQL" << table.Name() << "SelectDetail);\n";

         for (auto const& attr : table.Attributes() | std::views::filter([](auto const& a) { return a.Primary(); })) {
            os << "   query.Set(\"key" << attr.Name() << "\", key_val." << attr.Name() << "());\n";
            }

         os << "   query.Execute();\n"
            << "   query.First();\n"
            << "   if(!query.IsEof()) {\n";
         
         for (auto const& attr : table.Attributes()) {
            auto const& datatype = table.Dictionary().FindDataType(attr.DataType());
            os << "      data." << attr.Name() << "(query.Get<" << datatype.SourceType() << ">(\"" << attr.Name() << "\"";
            if (attr.Primary()) os << ", true";
            os << "));\n";
            }

         os << "      }\n"
            << "   else return false;\n"
            << "   if(query.Next(); !query.IsEof()) {\n"
            << "      std::ostringstream os1, os2;\n"
            << "      os1 << \"error while reading data for " << table.ClassName() << "\";\n"
            << "      os2 << \"couldn't read unique data for primary key element\\n\";\n"
            << "      key_val.write(os2);\n"
            << "      throw TMy_Db_Exception(os1.str(), os2.str(), database.Status(), strSQL" << table.Name() << "SelectDetail);\n"
            << "      }\n"
            ;

         os << "   return true;\n"
            << "   }\n\n";
         os << "\n";
      }

      if (PersistenceNamespace().size() > 0) os << "\n} // close namespace " << PersistenceNamespace() << "\n";
      return true;
      }
   else return false;
   }

#include "DataDictionary.h"
#include "DictionaryHelper.h"

#include <iostream>
#include <format>

using namespace std::string_literals;

template <bool boNeedKey = true>
bool CreateReadData(TMyTable const& table, std::ostream& os) {
   os << std::format("/// method to read data from the table {}\n", table.SQLName());
   return true;
   }

template <bool boKey = true>
bool CreateWriteData(TMyTable const& table, std::ostream& os) {
   os << std::format("/// method to write data in the table {}\n", table.SQLName());

   return true;
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
         << "#include <map>\n"
         << "#include <typeinfo>\n\n";

      for (auto const& [_, table] : Tables()) {
         os << "#include <" << (table.SrcPath().size() > 0 ? table.SrcPath() + "\\"s : ""s) + table.SourceName() + ".h>\n"s;
         }

      os << "\n"
         << "#include <adecc_Database/MyDatabase.h>\n"
         << "#include <adecc_Database/MyDatabaseExceptions.h>\n\n"
         << "#include <" << (PathToPersistence() / (PersistenceName() + "_sql.h"s)).string() << ">\n"
         << "#include \"" << (PathToBase() / "BaseDefinitions.h"s).string() << "\"\n"
         << "#include <string>\n"
         << "#include <memory>\n\n";

      if (PersistenceNamespace().size() > 0) os << "namespace " << PersistenceNamespace() << " {\n\n";

      auto maxLengthElement = std::ranges::max_element(Tables(), [](auto const& a, auto const& b) {
            return a.second.FullClassName().size() < b.second.FullClassName().size();
               });

      auto maxLengthAttr = [this, &maxLengthElement]() -> size_t {
         if (maxLengthElement != Tables().end()) return maxLengthElement->second.FullClassName().size();
         else                        return 0u;
         }();

      // -----------------------------------------------------------------------------------
      // erzeugen des concept für alle Tabellen
      os << "template <typename ty>\n"
         << "concept my_dataclasses =\n";
      {
      auto const& [_, table] = *std::begin(Tables());
      os << std::format("          (std::is_same_v<ty, {}> ||\n", table.FullClassName());
      }
      for (auto const& table : Tables() | own::views::second | std::views::drop(1) | std::views::take(Tables().size() - 2)) {
         os << std::format("           std::is_same_v<ty, {}> ||\n", table.FullClassName());
         }
      
      if(auto it = std::prev(Tables().end()); it != Tables().end()) {
         os << std::format("           std::is_same_v<ty, {}>) &&\n", it->second.FullClassName());
         }

      os << "          requires {\n"
         << "               typename ty::primary_key;\n"
         << "               typename ty::container_ty;\n"
         << "               typename ty::vector_ty;\n"
         << "               typename ty::func_ty;\n"
         << "          };\n\n";


      // -----------------------------------------------------------------------------------
      // erzeugen des concept für alle Primären Schlüssel der jeweiligen Tabellen
      os << "template <typename ty>\n"
         << "concept my_data_primkey =\n";
      for (auto const& table : Tables() | own::views::second | std::views::take(Tables().size() - 1)) {
         os << std::format("          std::is_same_v<ty, {}::primary_key> ||\n", table.FullClassName());
         }

      if (auto it = std::prev(Tables().end()); it != Tables().end()) {
         os << std::format("          std::is_same_v<ty, {}::primary_key>;\n\n", it->second.FullClassName());
         }


      os << "template <typename ty>\n"
         << "concept my_datacontainer =\n"
         << "      (own::is_vector_v<ty> && my_dataclasses<typename ty::value_type>) ||\n"
         << "      (own::is_set_v<ty> && my_dataclasses<typename ty::key_type>) ||\n"
         << "      (own::is_map_v<ty> &&\n"
         << "                     my_dataclasses<typename ty::mapped_type> &&\n"
         << "                     std::is_same_v<typename ty::key_type, typename ty::mapped_type::primary_key>);\n\n";

      os << "using concrete_db_server = " << PersistenceServerType() << ";\n"  // could be TMyMSSQL TMyMySQL TMyOracle TMyInterbase TMySQLite
         << "using concrete_framework = TMyQtDb<concrete_db_server>;\n"
         << "using concrete_db_connection = TMyDatabase<TMyQtDb, concrete_db_server>;\n"
         << "using concrete_query = TMyQuery<TMyQtDb, concrete_db_server>;\n\n";

      // erzeugen der Zugriffsklasse
      os //<< "template <my_dataclasses data_ty>\n"
         << "class DataAccess {\n"
         << "   private:\n"
         << "      // member to produce the data access\n"
         << "      concrete_db_connection& database;\n"
         << "      //concrete_query          query;\n"
         << "   public:\n"
         << "      DataAccess() = delete;\n"
         << "      DataAccess(concrete_db_connection& con) : database(con) { }\n"
         << "      DataAccess(DataAccess const&) = delete;\n\n"
         << "      template <my_dataclasses data_ty>\n"
         << "      data_ty GetTuple(concrete_query& query) const {\n"
         << "         data_ty data;\n";

      auto WriteGet4Table = [&os](auto const& table) {
         for (auto const& attr : table.Attributes()) {
            auto const& datatype = table.Dictionary().FindDataType(attr.DataType());
            os << "            data." << attr.Name() << "(query.Get<" << datatype.SourceType() << ">(\"" << attr.Name() << "\"";
            if (attr.Primary()) os << ", true";
            os << "));\n";
            }
         };


      { 
         auto const& table = std::begin(Tables())->second;
         os << std::format("         if constexpr (std::is_same_v<data_ty, {}>) {{\n", table.FullClassName());
         WriteGet4Table(table);
         os << "            }\n";
         }
      for(auto const& table :Tables() | own::views::second | std::views::drop(1))   {
         os << std::format("         else if constexpr(std::is_same_v<data_ty, {}>) {{\n", table.FullClassName());
         WriteGet4Table(table);
         os << "            }\n";
         }

      os << "         else static_assert(own::always_false<data_ty>, \"unexpected datatype for this class\");\n";

      os << "         return data;\n"
         << "         }\n\n";

      // =============================================================================================================
      os << "      template <my_dataclasses data_ty>\n"
         << "      std::optional<data_ty> Read(typename data_ty::primary_key const& key_val) {\n"
         << "            auto query = database.CreateQuery();\n";

      auto primkey2table = [&os](TMyTable const& table) {
         os //<< std::format("               auto const& strSQLQuery = {};\n", std::format(GetSQLQueryName<EQueryType::SelectPrim>(), table.Name()))
            << std::format("               query.SetSQL({});\n", std::format(GetSQLQueryName<EQueryType::SelectPrim>(), table.Name()));
         for (auto const& attr : table.Attributes() | own::views::primary) {
            os << std::format("               query.Set(\"key{0:}\", key_val.{0:}());\n", attr.Name());
            }
         os << "               }\n";
         };

      {
         auto const& [_, table] = *std::begin(Tables());
         os << std::format("            if constexpr (std::is_same_v<data_ty, {}>) {{\n", table.FullClassName());
         primkey2table(table);
         }
      for (auto const& table : Tables() | own::views::second | std::views::drop(1)) {
         os << std::format("            else if constexpr (std::is_same_v<data_ty, {}>) {{\n", table.FullClassName());
         primkey2table(table);
         }
      os << "            else static_assert(own::always_false<data_ty>, \"unexpected datatype for this class\");\n\n";
 
      os << "            if(query.Execute(), query.First();!query.IsEof()) [[likely]] {\n"
         << "               auto retval = GetTuple<data_ty>(query);\n"
         << "               if(query.Next(); !query.IsEof()) [[unlikely]] {\n"
         << "                  std::ostringstream os1, os2;\n"
         << "                  os1 << \"error while reading data for \" << typeid(retval).name();\n"
         << "                  os2 << \"couldn't read unique data for primary key element\\n\";\n"
         << "                  key_val.write(os2);\n"
         << "                  throw TMy_Db_Exception(os1.str(), os2.str(), database.Status(), query.GetSQL());\n"
         << "                  }\n"
         << "               else return retval;\n"
         << "               }\n"
         << "            else return {};\n"
         << "         }\n\n";


      // =============================================================================================================
      os << "      template <my_datacontainer data_ty>\n"
         << "      data_ty Read() {\n"
         << "         data_ty data;\n"
         << "         using used_type = own::used_type_t<data_ty>;\n\n"
         << "         auto query = database.CreateQuery();\n";


      {
         auto const& [_, table] = *std::begin(Tables());
         os << std::format("            if constexpr (std::is_same_v<used_type, {}>) {{\n", table.FullClassName())
            << std::format("               query.SetSQL({});\n", std::format(GetSQLQueryName<EQueryType::SelectAll>(), table.Name()))
            <<             "               }\n";
         }
      for (auto const& table : Tables() | own::views::second | std::views::drop(1)) {
         os << std::format("            else if constexpr (std::is_same_v<used_type, {}>) {{\n", table.FullClassName())
            << std::format("               query.SetSQL({});\n", std::format(GetSQLQueryName<EQueryType::SelectAll>(), table.Name()))
            << "               }\n";
         }

      os << "            else static_assert(own::always_false<used_type>, \"unexpected datatype for this class\");\n\n"
         << "            for (query.Execute(), query.First(); !query.IsEof(); query.Next()) {\n"
         << "               auto dataset = GetTuple<used_type>(query);\n"
         << "               if constexpr (own::is_vector<data_ty>::value) {\n"
         << "                  data.emplace_back(std::move(dataset));\n"
         << "                  }\n"
         << "               else  if constexpr (own::is_set<data_ty>::value) {\n"
         << "                  data.emplace(std::move(dataset));\n"
         << "                  }\n"
         << "               else if constexpr (own::is_map<data_ty>::value) {\n"
         << "                  auto keyval = dataset.GetKey();\n"
         << "                  data.emplace(std::move(keyval), std::move(dataset));\n"
         << "                  }\n"
         << "               else {\n"
         << "                  static_assert(own::always_false<data_ty>::value, \"unexpected container type\");\n"
         << "                  }\n"
         << "               }\n"
         << "            return data;\n"
         << "            }\n\n";

      // =============================================================================================================
      os << "      template <my_datacontainer data_ty>\n"
         << "      data_ty Read(std::string const& strSQL, my_db_params const& params = {}) {\n"
         << "         data_ty data;\n"
         << "         using used_type = own::used_type_t<data_ty>;\n\n"

         << "         auto query = database.CreateQuery();\n"
         << "         query.SetSQL(strSQL);\n"
         << "         query.Set(params);\n\n"

         << "         for (query.Execute(), query.First(); !query.IsEof(); query.Next()) {\n"
         << "            auto dataset = GetTuple<used_type>(query);\n"
         << "            if constexpr (own::is_vector<data_ty>::value) {\n"
         << "               data.emplace_back(std::move(dataset));\n"
         << "               }\n"
         << "            else  if constexpr (own::is_set<data_ty>::value) {\n"
         << "               data.emplace(std::move(dataset));\n"
         << "               }\n"
         << "            else if constexpr (own::is_map<data_ty>::value) {\n"
         << "               auto keyval = dataset.GetKey();\n"
         << "               data.emplace(std::move(keyval), std::move(dataset));\n"
         << "               }\n"
         << "            else {\n"
         << "               static_assert(own::always_false<data_ty>::value, \"unexpected container type\");\n"
         << "               }\n"
         << "            }\n"
         << "         return data;\n"
         << "         }\n\n";


      os << "      template <my_dataclasses data_ty>\n"
         << "      void Process(data_ty::func_ty func, std::string const& strSQL, my_db_params const& params = {}) {\n"
         << "         auto query = database.CreateQuery();\n"
         << "         query.SetSQL(strSQL);\n"
         << "         query.Set(params);\n\n"

         << "         for (query.Execute(), query.First(); !query.IsEof(); query.Next()) {\n"
         << "            func(std::move(GetTuple<data_ty>(query)));\n"
         << "            }\n"
         << "         }\n\n";

      os << "   };\n";


      os << "\n\n";
      os << "// data access class for the " << Denotation() << " project.\n"
         << "class " << PersistenceClass() << " {\n"
         << "   private:\n"
         << "      // member to produce the data access\n"
         << "      concrete_db_connection database;\n\n"
         << "   public:\n"
         << "      " << PersistenceClass() << "();\n"
         << "      " << PersistenceClass() << "(" << PersistenceClass() << " const&) = delete;\n"
         << "      " << PersistenceClass() << "(" << PersistenceClass() << "&&) noexcept = delete;\n"
         << "      ~" << PersistenceClass() << "() = default;\n\n"
         << "      concrete_db_connection& GetDatabase() { return database; }\n"
         << "      bool IsConnectedToDatabase() const { return database.Connected(); }\n"
         << "      bool GetServerHasIntegratedSecurity(void) const { return database.HasIntegratedSecurity(); }\n"
         << "      std::string GetDatabaseInformations(void) const;\n"
         << "      std::pair<std::string, std::string> GetConnectionInformations(void) const;\n"
         << "      std::pair<bool, std::string> LoginToDb(TMyCredential && credentials);\n"
         << "      void LogoutFromDb(void);\n\n";



      for(auto const& [_, table] : Tables()) {
         os << std::format("      // access methods for class {}\n", table.ClassName())
         //   << std::format("      bool Read(std::map<{0}::primary_key, {0}>&);\n", table.FullClassName())
            << std::format("      bool Read({0}::container_ty&);\n", table.FullClassName())
            << std::format("      bool Read({0}::primary_key const&, {0}&);\n", table.FullClassName())
            << "\n"
            << std::format("      // bool Delete({0}::primary_key const&);\n", table.FullClassName())
            << std::format("      // bool Update({0}::primary_key const&, {0} const&, bool = false);\n", table.FullClassName())
            << "\n"
            ;
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
            << "   query.SetSQL(" << "strSQLSelect" << table.Name() << "_All);\n"
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
            << "   query.SetSQL(" << "strSQLSelect" << table.Name() << "_Detail);\n";

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
            << "      throw TMy_Db_Exception(os1.str(), os2.str(), database.Status(), strSQLSelect" << table.Name() << "_Detail);\n"
            << "      }\n"
            ;

         os << "   return true;\n"
            << "   }\n\n";
         os << "\n";

         /*
         << std::format("      bool Delete({0}::primary_key const&);\n", table.FullClassName())
            << std::format("      bool Update({0}::primary_key const&, {0} const&, bool = false);\n", table.FullClassName())

         os << std::format("bool {1}::Read({0}::primary_key const& key_val, {0}& data) {{\n", table.FullClassName(), PersistenceClass());
         os << "   auto query = database.CreateQuery();\n"
            << "   query.SetSQL(" << "strSQL" << table.Name() << "SelectDetail);\n";

         for (auto const& attr : table.Attributes() | std::views::filter([](auto const& a) { return a.Primary(); })) {
            os << "   query.Set(\"key" << attr.Name() << "\", key_val." << attr.Name() << "());\n";
            }

         os << "   query.Execute();\n"
            << "   query.First();\n"

            */
      }

      if (PersistenceNamespace().size() > 0) os << "\n} // close namespace " << PersistenceNamespace() << "\n";
      return true;
      }
   else return false;
   }



// ------------------------------------------------------------------------------------------------
/// create the header file with definition for all generated sql statements
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
         os << "// sql statements for table " << table.FullyQualifiedSQLName() << '\n';
         sql_builder()
            .WriteQueryHeader<EQueryType::SelectAll>(table, os)
            .WriteQueryHeader<EQueryType::SelectPrim>(table, os);

         for (auto const& idx : table.Indices() | own::views::is_unique_key) {
            sql_builder()
               .WriteQueryHeader<EQueryType::SelectUnique>(table, idx, os);
            }

         for (auto const& idx : table.Indices() | own::views::is_index) {
            sql_builder()
               .WriteQueryHeader<EQueryType::SelectIdx>(table, idx, os);
            }

         for (auto const& ref : table.References()) {
            sql_builder()
               .WriteQueryHeader<EQueryType::SelectRelation>(table, ref, os)
               .WriteQueryHeader<EQueryType::SelectRevRelation>(table, ref, os);
            }

         if(table.EntityType() != EMyEntityType::view) {
            sql_builder()
               .WriteQueryHeader<EQueryType::Insert>(table, os)
               .WriteQueryHeader<EQueryType::UpdateAll>(table, os)
               .WriteQueryHeader<EQueryType::UpdateWithoutPrims>(table, os)
               .WriteQueryHeader<EQueryType::DeleteAll>(table, os)
               .WriteQueryHeader<EQueryType::DeletePrim>(table, os);
            }
         os << "\n";
         }

      if (PersistenceNamespace().size() > 0) os << "} // close namespace " << PersistenceNamespace() << "\n";

      return true;
      }
   else return false;
   }

// ------------------------------------------------------------------------------------------------
/// create the source file for all generated sql statements
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
         os << "// --------------------------------------------------------------------\n"
            << "//   Statements for table / view: " << table.Name() << "\n"
            << "// --------------------------------------------------------------------\n";
         sql_builder()
            .WriteQuerySource<EQueryType::SelectAll>(table, os)
            .WriteQuerySource<EQueryType::SelectPrim>(table, os);

         for (auto const& idx : table.Indices() | own::views::is_unique_key) {
            sql_builder()
               .WriteQuerySource<EQueryType::SelectUnique>(table, idx, os);
            }

         for (auto const& idx : table.Indices() | own::views::is_index) {
            sql_builder()
               .WriteQuerySource<EQueryType::SelectIdx>(table, idx, os);
            }

         for (auto const& ref : table.References()) {
            sql_builder()
               .WriteQuerySource<EQueryType::SelectRelation>(table, ref, os)
               .WriteQuerySource<EQueryType::SelectRevRelation>(table, ref, os);
            }

         if (table.EntityType() != EMyEntityType::view) {
            sql_builder()
               .WriteQuerySource<EQueryType::Insert>(table, os)
               .WriteQuerySource<EQueryType::UpdateAll>(table, os)
               .WriteQuerySource<EQueryType::UpdateWithoutPrims>(table, os)
               .WriteQuerySource<EQueryType::DeleteAll>(table, os)
               .WriteQuerySource<EQueryType::DeletePrim>(table, os);
            }
         os << "\n";
         }

      if (PersistenceNamespace().size() > 0) os << "\n} // close namespace " << PersistenceNamespace() << "\n";

      return true;
      }
   else return false;
   }


#include "DataDictionary.h"

#include <sstream>
#include <set>
#include <string_view>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;


inline std::string toHTML(const std::string& input) {
   std::string output;
   std::ranges::for_each(input, [&output](char c) {
      switch (c) {
      case 'ä': output += "&auml;"; break;
      case 'ö': output += "&ouml;"; break;
      case 'ü': output += "&uuml;"; break;
      case 'Ä': output += "&Auml;"; break;
      case 'Ö': output += "&Ouml;"; break;
      case 'Ü': output += "&Uuml;"; break;
      case 'ß': output += "&szlig;"; break;
      case '<': output += "&lt;"; break;
      case '>': output += "&gt;"; break;
      default: output.push_back(c); break;
      }
      });
   return output;
}

void TMyDictionary::Create_Doxygen_SQL(std::ostream& os) const {
   os << "\n\\page pg" << Identifier() << "_sql_informations informations to create database\n"
      << "\\section segPg" << Identifier() << "_preamble preamble\n"
      << "\\details the database contains " << Tables().size() << " tables with additional informations. this "
      << "          part of the documentations contains all information to create the database with tables, primary keys,\n"
      << "          foreign keys, key canditates and indices.\n"
      << "\\details there is only a script to drop all informations in the database. at first the script will drop\n"
      << "          all foreign key, before the tables can be dropped too.\n\n"
      << "\\section secPg" << Identifier() << "_sql_create create tables of the application\n"
      << "\\code{.sql}\n";
   Create_SQL_Tables(os);
   os << "\\endcode\n";

   os << "\\section secPg" << Identifier() << "_sql_additional create all additional informations\n"
      << "\\code{.sql}\n";
   Create_SQL_Additionals(os);
   os << "\\endcode\n";

   os << "\\section secPg" << Identifier() << "_sql_drop drop all elements in the database\n"
      << "\\code{.sql}\n";
   SQL_Drop_Tables(os);
   os << "\\endcode\n";

   }

// --------------------------------------------------------------------------------------------------------------------------------
// Create central documentation files
// --------------------------------------------------------------------------------------------------------------------------------
void TMyDictionary::Create_Doxygen(std::ostream& os) const {
   os << "\n";
   if (!directories.empty()) {
      for (auto const& [_, dir] : directories) {
         os << "\n\\dir " << dir.Name() << '\n'
            << "\\brief " << dir.Denotation() << '\n';
         std::ranges::for_each(dir.Description() | std::views::split('\n'), [&os](const auto& line) {
            os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
            });
         os << '\n';
         }
      }

   std::ranges::for_each(Tables(), [&os](auto const val) { 
                  os << std::format("\\include{{doc}} {}/{}.dox\n", std::get<1>(val).SrcPath(), std::get<1>(val).Name()); });
   os << std::format("\\include{{doc}} {}/{}.dox\n", "sql"s, Identifier() + "_sql"s);

   if(!namespaces.empty()) {
      for(auto const& [_, nsp] : namespaces) {
         os << "\n\\namespace " << nsp.Name() << '\n'
            << "\\brief " << nsp.Denotation() << '\n';
         std::ranges::for_each(nsp.Description() | std::views::split('\n'), [&os](const auto& line) {
            os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
            });
         os << '\n';
         std::ranges::for_each(nsp.Comment() | std::views::split('\n'), [&os](const auto& line) {
            os << "\\note "s << std::string_view(line.begin(), line.end()) << '\n';
            });
         os << '\n';
         }
      }

   if(UseBaseClass()) {
      std::string strFullName = (BaseNamespace().size() > 0 ? BaseNamespace() + "::"s + BaseClass() : BaseClass());
      os << "\n\\file " << PathToBase().string() << "\n"
         << "\\brief definition auf the common base class " << strFullName << " for the project " << Denotation() << ".\n\n"
         << "\\class " << strFullName << "\n"
         << "\\brief virtual base class for the project which defined the functions init and copy and can use as base for operations.\n\n"
         << "\\fn " << strFullName << "::init()\n"
         << "\\brief pure virtual function to init the concret instance of a derived class.\n\n"
         << "\\fn " << strFullName << "::copy(" << strFullName << " const& other)\n"
         << "\\brief pure virtual function to copy the concret instance of a derived class into another matching instance.\n"
         << "\\param[in] other const reference to an concrete instance of a derived class.\n\n";
      }

   if(HasPersistenceClass()) {
      auto PathToPers = [this]() {
         //if (PathToPersistence().root_path() == fs::path()) return SourcePath() / PathToPersistence();
         //else 
            return PathToPersistence();
         }();

      std::string strFullName = (PersistenceNamespace().size() > 0 ? PersistenceNamespace() + "::"s + PersistenceClass() : PersistenceClass());
      if(PersistenceNamespace().size() > 0) {
         os << "\\namespace " << PersistenceNamespace() << "\n"
            << "\\brief namespace for the classes to implement the data access for all table in the project.\n\n";
         }

      os << "\\dir " << PathToPers << "\n"
         << "\\brief directory with files for the access to database.\n\n"
         << "\\file " << PathToPers / (PersistenceName() + "_sql.h"s) << "\n"
         << "\\brief defintion of the necessary sql statements to work with the tables.\n\n"
         << "\\file " << PathToPers / (PersistenceName() + "_sql.cpp"s) << "\n"
         << "\\brief implementation of the necessary sql statements to work with the tables.\n\n"
         << "\\file " << PathToPers / (PersistenceName() + ".h"s) << "\n"
         << "\\brief definition of the reader class " << strFullName << " with methods for all tables for access to the database.\n\n"
         << "\\file " << PathToPers / (PersistenceName() + ".cpp"s) << "\n"
         << "\\brief implementation of the reader class " << strFullName << " with methods for all tables for access to the database.\n\n"
         << "\\class " << strFullName << "\n"
         << "\\brief class with the access methods to database for all tables.\n"
         << "\\details this class contain all functions for the access to entities of the project << " << Name() << ".\n"
         << "\\details The example project \\ref pgadeccDatabase is use to implement the access to the concrete database.\n"
         << "\\details The used sql database server is from type " << PersistenceServerType() << ".\n\n";

      for (auto const& [_, table] : Tables()) {
         std::string nameSpace = PersistenceNamespace().size() > 0 ? PersistenceNamespace() + "::" : ""s;
         os << "\\var " << nameSpace << "strSQL" << table.Name() << "All\n"
            << "\\brief constant std string with the sql statement to read all tuples from table \\ref "s + table.Doc_RefName() << ".\n\n"
            << "\\var " << nameSpace << "strSQL" << table.Name() << "Detail\n"
            << "\\brief constant std string with the sql statement to read a tuple from table \\ref "s + table.Doc_RefName() << " with the primary key.\n\n"
            << "\\fn " << strFullName << "::" << std::format("Read({0}::container_ty& data)\n", table.FullClassName())
            << "\\brief method to read all data of the table \\ref " << table.Doc_RefName() << " in a container.\n"
            << "\\details the method use the const std::string " << nameSpace << "strSQL" << table.Name() << "All to read the data.\n"
            << "\\param[out] data reference to " << std::format("{0}::container_ty", table.FullClassName()) << " with the container to take the data.\n"
            << "\\returns bool when data readed successfull (0 .. n)\n\n"
            ;
         }
      }

   os << "\n\\mainpage " << Denotation() << '\n'
      << "\\tableofcontents\n";
   os << "<a id = \"top_of_mainpage\"></a>\n"
      << "\n\\section secMainDescription project description / preamble\n";
   std::ranges::for_each(Description() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
      });

   os << "\n";
   std::ranges::for_each(Comment() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\note "s << std::string_view(line.begin(), line.end()) << '\n';
      });

   os << "\n\\section secMainDatatypes used datatypes\n"
      << "\\details the following table show datatypes and the transformation into types for the used sql server "
      << "and the programming language. hungarian notation is used in the program. the table contain "
      << "prefixes which are used for the data types.\n"
      << "\\details for some data types(e.g.integers), no exact distinction is made.\n";

   os << "\\details "
      /*
      << "<table><thead>\n"
      << "<tr><th rowspan=\"2\">datatype</th>"
      << "    <th>db datatype</th>"
      << "    <th>db check</th>"
      << "    <th>src type</th>"
      << "    <th>header</th>"
      << "    <th>prefix</th></tr>"
      << "<tr><th colspan=\"5\">comment</th>"
      << "</tr></thead><tbody>\n";
   */
      << "<table>\n"
      << "<tr><th>datatype</th>"
      << "    <th>db datatype</th>"
      << "    <th>db check</th>"
      << "    <th>src type</th>"
      << "    <th>header</th>"
      << "    <th>prefix</th>"
      << "</tr>\n";
   for(auto const& [_, datatype] : DataTypes()) {
      os << "<tr><td rowspan= \"2\" valign=\"top\">" << "<a id=\"datatyp_" << datatype.DataType() << "\"></a>" << datatype.DataType() << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.DatabaseType().size() > 0 ? datatype.DatabaseType() : " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.CheckSeq().size() > 0     ? datatype.CheckSeq() :     " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.SourceType().size() > 0   ? datatype.SourceType() :   " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.Headerfile().size() > 0   ? datatype.Headerfile() :   " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.Prefix().size() > 0       ? datatype.Prefix() :       " &nbsp;"s) << '\n'
         << "</tr><tr>"
         << "    <td colspan=\"5\" valign=\"top\">" << toHTML(datatype.Comment().size() > 0 ? datatype.Comment() : " &nbsp;"s) << "</td>\n"
         << "</tr>\n";
      }
   os << "</table>\n\n";


   os << "\\section datamodel_all_tables overview about all tables\n"
      << "\\details the following tables are created or used by the application in the database:\n"
      << "\\details "
      /*
      << "<table><thead>\n"
      << "<tr><th rowspan=\"2\" valign=\"top\">name</th>\n"
      << "    <th>db name</th>\n"
      << "    <th>type</th>\n"
      << "    <th>source name</th></tr>\n"
      << "<tr><th colspan=\"3\">description</th></tr>\n"
      << "</thead><tbody>\n";
      */
   << "<table><thead>\n"
      << "<tr><th>name</th>\n"
      << "    <th>db name</th>\n"
      << "    <th>type</th>\n"
      << "    <th>source name</th></tr>\n"
      << "</thead><tbody>\n";


   for (auto const& [db_name, table] : Tables()) {
      os << "<tr><td rowspan=\"2\" valign=\"top\">\\ref " << table.Doc_RefName() << "</td>\n"
         << "    <td>" << table.FullyQualifiedSQLName() << "</td>\n";
      os << "    <td>" << table.EntityTypeTxt() << "</td>\n";
      os << "    <td>" << table.FullyQualifiedSourceName() << "</td></tr>\n"
         << "<tr><td colspan=\"3\">" << (table.Denotation().size() > 0 ? table.Denotation() : " &nbsp;"s) << "</td>\n"
         << "</tr>";
      }
   os << "</tbody></table>\n\n"
      << "<hr><a href = \"top_of_mainpage\">top of the page</a>\n";

   // ----------------- create the entity relationship model for all tables ------------------------
   static constexpr auto processString = [](std::string const& input) {
         if (input.size() == 0) return std::make_pair(""s, ""s);
         else {
            auto pos = input.find(':');
            std::string val1 = input.substr(0, pos);
            std::string val2 = input.substr(pos + 1);
            val1.erase(0, val1.find_first_not_of(' ')); 
            val1.erase(val1.find_last_not_of(' ') + 1); 
            val2.erase(0, val2.find_first_not_of(' ')); 
            val2.erase(val2.find_last_not_of(' ') + 1); 
            return std::make_pair(val1, val2);
            }
         };

   std::string strEntityFmt = "shape=box, fontname=\"Helvetica\", fontsize=8, fillcolor = \"white\", style = \"filled\"";
   std::string strRelationshipFmt = "shape=diamond,fontname=\"Helvetica\", fontsize=8, fillcolor = \"white\", style = \"filled\"";
   std::string strGraphFmt = "color=\"darkorchid3\", style=\"solid\"";
   std::string strGraphLabelFmt = "fontname=\"Helvetica\", fontsize=8, fontcolor=\"grey\""; // labelfloat=true, 
   /*dir=\"back\",color="darkorchid3",style="solid"  / dashed  label=" boWorkday",fontcolor="grey"  */

   os << "\\section datamodel_er_diagram database diagram\n"
      << "\\details \\dot\n"
      << "graph ER {\n"
      << "   bgcolor=\"transparent\";\n"
      << "   graph [fontname=\"Helvetica\", fontsize=8];\n";
   // create nodes with tables
   std::ranges::for_each(Tables(), [&os, &strEntityFmt](auto const& val) {
            os << "   " << std::get<1>(val).Name() << " [label=\"" << std::get<1>(val).SQLName() 
               << "\", " << strEntityFmt << ", URL=\"\\ref " << std::get<1>(val).Doc_RefName() << "\"];\n";
            });

   if(false) {
      // create nodes with the relationships
      std::ranges::for_each(Tables(), [&os, &strRelationshipFmt](auto const& table) {
         std::ranges::for_each(std::get<1>(table).References(), [&os, &strRelationshipFmt](auto const& ref) {
              os << "   " << ref.Name() << " [label=\"" << ref.Description();
              if (ref.Cardinality().size() > 0) os << "\n(" << ref.Cardinality() << ")";
              os << "\", " << strRelationshipFmt << "];\n";
              });
         });

      // create connections between nodes
      std::ranges::for_each(Tables(), [&os, &strGraphFmt](auto const& pair) {
           TMyTable const& table = std::get<1>(pair);
           std::ranges::for_each(table.References(), [&os, &table, &strGraphFmt](auto const& ref) {
                 TMyTable const& refTable = table.Dictionary().FindTable(ref.RefTable());
                 os << "   " << table.Name() << " -- " << ref.Name() << " [" << strGraphFmt << "];\n";
                 os << "   " << ref.Name() << " -- " << refTable.Name() << " [" << strGraphFmt << "];\n";
                 });
           });
      }
   else {
      // create connections between nodes
      std::ranges::for_each(Tables(), [&os, &strGraphFmt, &strGraphLabelFmt](auto const& pair) {
         TMyTable const& table = std::get<1>(pair);
         std::ranges::for_each(table.References(), [&os, &table, &strGraphFmt, &strGraphLabelFmt](auto const& ref) {
            TMyTable const& refTable = table.Dictionary().FindTable(ref.RefTable());
            auto cardinality = processString(ref.Cardinality());
            os << "   " << table.Name() << " -- " << refTable.Name()    // 
               << " [label=\"" << ref.Description() << "\", " 
               << " headlabel = \"" << cardinality.first << "\", "
               << " taillabel = \"" << cardinality.second << "\", "
               << strGraphFmt << ", " << strGraphLabelFmt << "];\n";
            });
         });
      }

   //os << '\n';
   os << "   label = \"\\n\\nEntity Relation Diagram\\n" << Denotation() << "\"\n"
      << "   }\n"
      << "\\enddot\n\n";

   os << "\\section datamodel_all_attributes all attributes and relationships in tables\n";

   for (auto const& [db_name, table] : Tables()) {
      os << "\\subsection " << table.Doc_RefName() << ' ' << db_name 
         //<< (table.)
         << '\n';
      
      if (table.Denotation().size() > 0) os << "\\details <b>" << table.Denotation() << "</b>\n";
      else os << "\\attention <b>descritions for the table missed.</b>\n";
      os << "\\details the system class which represent this table in source is \\ref " << table.FullClassName() << ".\n";

      if (table.Description().size() > 0)
         std::ranges::for_each(table.Description() | std::views::split('\n'), [&os](const auto& line) {
                          os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n'; });

      os << "\n\\subsubsection " << table.Doc_RefName() << "_attributes attributes\n";

      os << "\\details <table>\n"
         << "<tr><th rowspan=\"2\" valign=\"center\">name\n"
         << "    <th>type\n"
         << "    <th>db type\n"
         << "    <th>size\n"
         << "    <th>not null\n"
         << "    <th>primary\n"
         << "    <th>check\n"
         << "    <th>init seq\n"
         << "    <th>computed\n"
         << "    <th>source name\n"
         << "    <th>source type\n</tr>"
         << "<tr><th colspan=\"11\">description\n"
         << "</tr>\n";
      for (auto const& attr : table.Attributes()) {
         auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
         os << "<tr><td rowspan=\"2\" valign =\"top\">" << attr.Name() << '\n'
            << "    <td valign = \"top\"><a href =\"#datatyp_" << attr.DataType() << "\">" << attr.DataType() << "</a>\n"
            << "    <td valign = \"top\">" << datatype.DatabaseType() << '\n';
         os << std::format("    <td valign = \"top\">{}\n", 
                              (datatype.UseLen() && datatype.UseScale() ? std::format("({}, {})", attr.Len(), attr.Scale()) :
                                               (datatype.UseLen() ? std::format("({})", attr.Len()) : ""s)));
         os << "    <td valign = \"top\">" << std::boolalpha << attr.NotNull() << '\n'
            << "    <td valign = \"top\">" << std::boolalpha << attr.Primary() << '\n';

         os << "    <td valign = \"top\">";
         if(datatype.CheckSeq().size() > 0) {
            os << toHTML(datatype.CheckSeq());
            if(attr.CheckSeq().size() > 0 && attr.CheckAtTable() != EMyCheckKinds::table) os << " / " << toHTML(attr.CheckSeq());
            }
         else if (attr.CheckSeq().size() > 0 && attr.CheckAtTable() != EMyCheckKinds::table) os << toHTML(attr.CheckSeq());

         os << "    <td valign = \"top\">" << toHTML(attr.InitSeq().size() > 0 ? attr.InitSeq() : " &nbsp;"s) << '\n'
            << "    <td valign = \"top\">" << toHTML(attr.Computed().size() > 0 ? attr.Computed() : " &nbsp;"s) << '\n'
            << "    <td valign = \"top\">" << datatype.Prefix() + attr.Name() << '\n'
            << "    <td valign = \"top\">" << datatype.SourceType() << "</td></tr>\n"
            << "<tr><td colspan=\"11\" valign = \"top\">" << toHTML(attr.Denotation().size() > 0 ? attr.Denotation() : " &nbsp;"s) << '\n'
            << "</tr>"; 
         }

      os << "</table>\n\n";

      // -------------------------------------------------------------------------------------------
      // create a section for the relationships (only when there relationships exists)
      // -------------------------------------------------------------------------------------------
      if(!table.References().empty()) {
         os << "\n\\subsubsection " << table.Doc_RefName() << "_relationships_from relationships from " << table.Name() << "\n";

         os << "\\details <table>\n"
            << "<tr><th>type\n"
            << "    <th>related\n"
            << "    <th>to table\n"
            << "    <th>description\n"
            << "</tr>\n";

         for(auto const& reference : table.References()) {
            TMyTable const& refTable = table.Dictionary().FindTable(reference.RefTable());
            os << "<tr><td valign =\"top\">" << reference.ReferenceTypeTxt() << '\n'
               << "    <td valign =\"top\">" << reference.Description() << '\n'
               << "    <td valign =\"top\"> \\ref " << refTable.Doc_RefName() << '\n'
               << "    <td valign =\"top\">" << reference.Comment() << '\n'
               << "</tr>";
            }
         os << "</table>\n\n";
         }

      // --------------------------------------------------------------------------------------------
      // create a section for the relationships to this entity (only when there relationships exists)
      // --------------------------------------------------------------------------------------------
      std::vector<std::pair<TMyTable, TMyReferences>> process_data;
      for(auto const& [_, ref_table] : Tables() | std::views::filter([&table](auto const& t) { return t.first != table.Name(); })) {
         for(auto const& ref : ref_table.References() | std::views::filter([&table](auto const& r) { return r.RefTable() == table.Name(); } )) {
            process_data.emplace_back(std::make_pair(ref_table, ref));
            }
         }

      if (!process_data.empty()) {
         os << "\n\\subsubsection " << table.Doc_RefName() << "_relationships_to relationships to " << table.Name() << "\n";

         os << "\\details <table>\n"
            << "<tr><th>from table\n"
            << "    <th>type\n"
            << "    <th>related\n"
            << "    <th>description\n"
            << "</tr>\n";

         for(auto const& [tab, ref] : process_data) {
            os << "<tr><td valign =\"top\"> \\ref " << tab.Doc_RefName() << '\n'
               << "    <td valign =\"top\">" << ref.ReferenceTypeTxt() << '\n'
               << "    <td valign =\"top\">" << ref.Description() << '\n'
               << "    <td valign =\"top\">" << ref.Comment() << '\n'
               << "</tr>";
            }
         os << "</table>\n\n";
         }

      // --------------------------------------------------------------------------------------
      // create a section for the sql statements
      // --------------------------------------------------------------------------------------
      os << "\n\\subsubsection " << table.Doc_RefName() << "_create create statement\n"
         << "\\code{.sql}\n";
      table.SQL_Create_Table(os);
      os << '\n';
      table.SQL_Create_Alter_Table(os);
      os << '\n';
      table.SQL_Create_PostConditions(os);
      os << '\n';
      table.SQL_Create_Check_Conditions(os);
      os << '\n';
      table.SQL_Create_Primary_Key(os);
      os << '\n';
      table.SQL_Create_Unique_Keys(os);
      os << '\n';
      table.SQL_Create_Foreign_Keys(os);
      os << "\n\\endcode\n";
      os << "\n";

      // --------------------------------------------------------------------------------------
      // create a section for the sql statements as range values or preset values
      // --------------------------------------------------------------------------------------
      if(table.RangeValues().size() > 0) {
         os << "\n\\subsubsection " << table.Doc_RefName() << "_values insert values\n"
            << "\\code{.sql}\n";
         table.SQL_Create_RangeValues(os);
         os << "\n\\endcode\n";
         }

      os << "<hr><a href = \"#top_of_mainpage\">top of the page</a>\n";
      os << "<hr>\n";

      }

   os << "<hr>\n"
      << "\\see \\ref pg" << Identifier() << "_sql_informations\n\n"
      << "<hr>\n";

   if(License().size() > 0) {
      os << "\\section secMainLicense license conditions\n"
         << License() << '\n';
      }
   os << "\\author " << Author() << '\n'
      << "\\version " << Version() << '\n'
      << "\\date " << CurrentDate() << '\n';

   if(Copyright().size() > 0)  {
      os << "\\copyright " << Copyright() << '\n';
      }

   // create a subpage for sql
   }


// -------------------------------------------------------------------------------------------------------------------
//  Create Documantation for a table
// -------------------------------------------------------------------------------------------------------------------
bool TMyTable::CreateDox(std::ostream& os) const {

   auto parents         = GetParents(EMyReferenceType::generalization);
   auto part_of_data    = GetPart_ofs(EMyReferenceType::composition);
   auto processing_data = GetProcessing_Data();

   // ------------- write the file documentation for the header file of dataclasses -----------------
   os << "\\file " << (SrcPath().size() > 0 ? SrcPath() + "/"s : ""s) + SourceName() + ".h\n"
      << "\\brief definition of dataclass " << FullyQualifiedSourceName() << " of table \\ref " << Doc_RefName();
   if (Denotation().size() > 0) os << " with " << Denotation();
   os << " in dictionary \"" << Dictionary().Name() << "\"\n";

   std::ranges::for_each(Description() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\details " << std::string_view(line.begin(), line.end()) << '\n'; });

   os << "\\see table \\ref " << Doc_RefName() << '\n';
   os << "\\version " << Dictionary().Version() << '\n'
      << "\\author " << Dictionary().Author() << '\n'
      << "\\date " << CurrentDate() << "  file created with adecc Scholar metadata generator\n";

   if (Dictionary().Copyright().size() > 0) {
      os << "<HR>\n"
         << "\\copyright &copy; " << Dictionary().Copyright() << '\n';
      if (Dictionary().License().size() > 0) {
         os << "<BR>" << Dictionary().License() << '\n';
         }
      }
   os << "\n\n";

   // ------------- write the file documentation for the source file of dataclasses -----------------
   os << "\\file " << (SrcPath().size() > 0 ? SrcPath() + "/"s : ""s) + SourceName() + ".cpp\n"
      << "\\brief implementation of dataclass " << FullyQualifiedSourceName() << " of table \\ref " << Doc_RefName();
   if (Denotation().size() > 0) os << " with " << Denotation();
   os << " in dictionary \"" << Dictionary().Name() << "\"\n";
   std::ranges::for_each(Description() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\details " << std::string_view(line.begin(), line.end()) << '\n'; });
   std::ranges::for_each(Comment() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\note " << std::string_view(line.begin(), line.end()) << '\n'; });

   os << "\\see table \\ref " << Doc_RefName() << '\n';
   os << "\\version " << Dictionary().Version() << '\n'
      << "\\author " << Dictionary().Author() << '\n'
      << "\\date " << CurrentDate() << "  file created with adecc Scholar metadata generator\n";

   if (Dictionary().Copyright().size() > 0) {
      os << "<HR>\n"
         << "\\copyright &copy; " << Dictionary().Copyright() << '\n';
      if (Dictionary().License().size() > 0) {
         os << "<BR>" << Dictionary().License() << '\n';
      }
   }
   os << "\n\n";


   // ----------------- write the documentation for the class ---------------------------------
      os << "\\class " << FullClassName() << '\n'
      << "\\brief " << Denotation() << '\n';

   std::ranges::for_each(Description() | std::views::split('\n'), [&os](const auto& line) { 
      os << "\\details " << std::string_view(line.begin(), line.end()) << '\n'; });

   // -------------- write documentation for inherited classes -------------------------
   if (!parents.empty()) {
      os << "\\details this class is inherited by following class(es):\n"
         << "<table>\n"
         << "<tr><th>class</th><th>table</th><th>file</th></tr>\n";

      std::ranges::for_each(parents, [this, &os](auto const& p) {
         os << "<tr><td>" << p.FullClassName() << "</td>"
            << "    <td>\\ref " << p.Doc_RefName() << "</td>"
            << "    <td>" << (p.SrcPath().size() > 0 ? p.SrcPath() + "/"s : ""s) + p.SourceName() + ".h</td></tr>";
         });
      os << "</table>\n\n";
      }

   // ------------- write documentation for part of relationships ----------------------
   if(!part_of_data.empty()) {
      os << "\\details this class have composed data of following class(es). The following table show a composed "
         << "and types which created to use this classes:\n"
         << "<table><thead>\n"
         << "<tr><th rowspan=\"2\">name of key</th>\n"
         << "    <th rowspan=\"2\">class</th>\n"
         << "    <th rowspan=\"2\">table</th>\n"
         << "    <th colspan=\"2\">key attributes</th>\n"
         << "    <th rowspan=\"2\">additional keys</th>\n"
         << "    <th rowspan=\"2\">file</th></tr>\n"
         << "<tr><th> this</th>\n"
         << "<th> foreign</th>\n"
         << "</tr></thead><tbody>";
      for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
         os << "<tr><td rowspan=\"" << vecParams.size() << "\">" << FullClassName() << "::" << strType << "</td>\n"
            << "    <td rowspan=\"" << vecParams.size() << "\">" << table.FullClassName() << "</td>\n"
            << "    <td rowspan=\"" << vecParams.size() << "\">\\ref " << table.Doc_RefName() << "</td>\n";
         auto const& attr1 = table.FindAttribute(vecParams[0].first);
         auto const& attr2 = table.FindAttribute(vecParams[0].second);
         os << "    <td>" << attr1.Name() << "(" << Dictionary().FindDataType(attr1.DataType()).SourceType() << ")" << "</td>\n"
            << "    <td>" << attr2.Name() << "(" << Dictionary().FindDataType(attr2.DataType()).SourceType() << ")" << "</td>\n";
         int i = 0;
         os << "    <td rowspan=\"" << vecParams.size() << "\">";
         std::ranges::for_each(vecKeys, [&os, &table, &vecKeys, &i, this](auto const& k) {
            auto const& attr = table.FindAttribute(k);
            os << (i++ > 0 ? ", " : "") << attr.Name() << "(" << Dictionary().FindDataType(attr.DataType()).SourceType() << ")";
            });
         os << "</td>\n"
            << "    <td rowspan=\"" << vecParams.size() << "\">"
            << "\\ref " << (table.SrcPath().size() > 0 ? table.SrcPath() + "/"s : ""s) + table.SourceName() + ".h </td>\n</tr>\n";
         std::ranges::for_each(vecParams | std::views::drop(1), [&os, &table, this](auto const& p) {
            auto const& attr1 = table.FindAttribute(p.first);
            auto const& attr2 = table.FindAttribute(p.second);
            os << "    <td>" << attr1.Name() << "(" << Dictionary().FindDataType(attr1.DataType()).SourceType() << ")" << "</td>"
               << "    <td>" << attr2.Name() << "(" << Dictionary().FindDataType(attr2.DataType()).SourceType() << ")" << "</td>";
            });
      
         }

      os << "</tbody></table>\n\n";
      }

   std::ranges::for_each(Comment() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\note " << std::string_view(line.begin(), line.end()) << '\n'; });

   os << "\\author " << Dictionary().Author() << '\n'
      << "\\date " << CurrentDate() << " documentation for this project created\n"
      << "\\see system class for the table \\ref " << Doc_RefName() << "\n\n";

   // helping class for primary key
   auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();

   os << "\\class " << FullyQualifiedSourceName() << "::primary_key\n"
      << "\\brief primary key for elements of the class " << FullyQualifiedSourceName() << " in a container "
      << "or when seeking an entity in the database.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::primary_key()\n"
      << "\\brief standard constructor for the class " << FullyQualifiedSourceName() << "::primary_key\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::primary_key(primary_key const& other)\n"
      << "\\brief copy constructor for the class " << FullyQualifiedSourceName() << "::primary_key\n"
      << "\\param [in] other primary_key const& with the instance which values should be copied\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::primary_key(primary_key&&)\n"
      << "\\brief move constructor for the class " << FullyQualifiedSourceName() << "::primary_key\n"
      << "\\param [in] other primary_key&& with the instance which values should be occupied and moved to this instance\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::primary_key(" << ClassName() << " const& other)\n"
      << "\\brief initializing constructor with an instance of the encircling class " << FullyQualifiedSourceName()
      << " for the class " << FullyQualifiedSourceName() << "::primary_key\n"
      << "\\note this constructor can't be constexpr because the direct manipulator of the encircling class may throw an exception\n"
      << "\\throws std::runtime_error when a data element in the encircling class is empty\n"
      << "\\param [in] other " << FullyQualifiedSourceName() << " const& with the instance of the encircling class which values "
      << "should be copied\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::~primary_key()\n"
      << "\\brief destructor for the primary_key class\n"
      << "\n"

      
      << std::format("\\fn {0}::primary_key::primary_key({1}{2} p{3}", FullyQualifiedSourceName(), std::get<1>(prim_attr[0]).SourceType(),
                                                                  (std::get<1>(prim_attr[0]).UseReference() ? " const&" : ""),
                                                                   std::get<0>(prim_attr[0]).Name());
   for (auto const& [attr, dtype] : prim_attr | std::views::drop(1))
      os << std::format(", {0}{1} p{2}", dtype.SourceType(), (dtype.UseReference() ? " const&" : ""), attr.Name());
   os << ")\n"
      << "\\brief initializing constructor with the values for key attributes of the primary_key class\n";
   for (auto const& [attr, dtype] : prim_attr) {
      os << std::format("\\param [in] p{0} {1} {2} with {3}\n", attr.Name(), dtype.SourceType(), (dtype.UseReference() ? " const&" : ""), attr.Denotation());
      }
   os << "\n";
   for (auto const& [attr, dtype] : prim_attr) {
      std::string strFullAttr   = FullyQualifiedSourceName() + "::primary_key::"s + dtype.Prefix() + attr.Name();
      std::string strFullMethod = FullyQualifiedSourceName() + "::primary_key::"s + attr.Name();

      std::string strNote1 = "Source: key member "s + strFullAttr + " in class " + FullyQualifiedSourceName() + "::primary_key with the type "s + dtype.SourceType();
      std::string strNote2 = "Database: primary key attribute \""s + attr.DBName() + "\" in entity \\ref "s + Doc_RefName() + " with database type "s + dtype.DatabaseType();

      // direct data elements
      os << "\\var " << strFullAttr << "\n";
      if (attr.Denotation().size() > 0)
         os << "\\brief " << attr.Denotation() << '\n'
         << "\\details " << strNote1 << '\n';
      else
         os << "\\brief " << strNote1 << '\n';
      os << "\\details " << strNote2 << '\n';

      os << "\\fn " << strFullMethod << "() const\n"
         << "\\brief selector for the data element " << strFullAttr << "\n"
         << "\\returns " << dtype.SourceType() << (dtype.UseReference() ? " const& " : "") << " with the value for this member.\n"
         << "\\fn " << strFullMethod << "(" << dtype.SourceType() << (dtype.UseReference() ? " const&" : "") << " newVal)\n"
         << "\\brief manipulator for the data element " << strFullAttr << "\n"
         << "\\param[in] newVal " << dtype.SourceType() << (dtype.UseReference() ? " const& " : "") << " with the value of the member.\n"
         << "\\returns " << dtype.SourceType() << (dtype.UseReference() ? " const& " : "") << " with the new value for this member.\n";

      }

   // conversion operator for the encircling class
   os << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator " << ClassName() << " () const\n"
      << "\\brief conversion operator for the encircling class\n"
      << "\\returns value of an instance of the encircling class " << FullyQualifiedSourceName() << "\n\n";

   // relational operators for the key class
   os << "\\fn " << FullyQualifiedSourceName() << "::primary_key::_compare(primary_key const&) const\n"
      << "\\brief internal help function to compare the current instance with to another instance of the same class.\n"
      << "\\details the comparison is performed using a lambda function, \"comp_help\", which compares two objects and "
      << "returns -1 if the left - hand side (lhs) is less than the right - hand side(rhs), 1 if lhs is greater than rhs, "
      << "and 0 if they are equal. the member while compared in the order they are in the dictionary one after the other, "
      << "so long the values of the both instances are equal.\n"
      << "\\param [in] other a constant reference to another object of the same class whose is being compared with the current "
      << "object's values.\n"
      << "\\return an integer value representing the result of the comparison:\n"
      << "<ul>"
      << "<li> returns  0 if the values of both objects are equal.</li>\n"
      << "<li>returns -1 if the values of the current object is less than values of the other object.</li>\n"
      << "<li>returns  1 if the values of the current object are greater than the values of the other object.</li>\n"
      << "</ul>"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator == (primary_key const&) const\n"
      << "\\brief checks whether the object is equal to another object of the same primary key class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the objects are equal, false otherwise.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator != (primary_key const&) const\n"
      << "\\brief checks whether the object is not equal to another object of the same class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the objects are not equal, false otherwise.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator < (primary_key const&) const\n"
      << "\\brief checks whether the object is less than the other object of the same class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the object is less than to the other object, false otherwise.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator <= (primary_key const&) const\n"
      << "\\brief checks whether the object is less than or equal to the other object of the same class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the object is less than or equal to the other object, false otherwise.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator > (primary_key const&) const\n"
      << "\\brief checks whether the object is greater than the other object of the same class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the object is greater than to the other object, false otherwise.\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::primary_key::operator >= (primary_key const&) const\n"
      << "\\brief checks whether the object is greater than or equal the other object of the same class.\n"
      << "\\details this operator use the internal function primary_key::_compare() as implemetation and compare "
      << "the member in the sequence they are found in the dictionary."
      << "\\param other a constant reference to another object of the same class primary_key.\n"
      << "\\return boolean value, true if the object is greater than or equal to the other object, false otherwise.\n"
      << "\n";



   // helping types for composed data
   os << "\\typedef " << FullyQualifiedSourceName()<< "::container_ty\n"
      << "\\brief container type as std::map with the generated primary key type "
      << FullyQualifiedSourceName() << "::primary_key for instances of this class\n\n"
      << "\\details The type uses the key type " << FullyQualifiedSourceName() << "::primary_key" 
      << "previously created from the key attributes of table \\ref " << Doc_RefName() << " as the "
      << "key_type for the container to hold values of this class as value_type.\n"
      << "This means that the same rules apply in the database and the program, and the data can "
      << "be assigned quickly.\n"
      << "\n"
      << "\\typedef " << FullyQualifiedSourceName() << "::vector_ty\n"
      << "\\brief container type as vector for elements of this class.\n"
      << "\\details you can use a sort order to read data into this container or work with this later.\n"
      << "\n";

   os << "\\fn " << FullyQualifiedSourceName() << "::GetKey() const\n"
      << "\\brief method to get the primary key for this instance\n"
      << "\\returns type " << FullyQualifiedSourceName() << "\n"
      << "\\throw std::runtime::error if the attribute(s) of the primary key are empty.\n"
      << "\n";

   for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
      os << "\\typedef " << FullyQualifiedSourceName() << "::"s << strType << "\n"
         << "\\brief " << "composed data element for the table \\ref " << table.Doc_RefName() << "\n"
         << "\\details This type uses the data elements of the primary key of the table that are not used in the link to this type.\n"
         << "\\details <table><tr><th>attribute</th><th>data element</th><th>description</th></tr>\n";
      for(auto const& id : vecKeys) {
         auto const& attr  = table.FindAttribute(id);
         auto const& dtype = Dictionary().FindDataType(attr.DataType());
         os << "<tr><td>" << attr.Name() << "</td>\n"
            << "    <td>" << table.FullyQualifiedSourceName() << "::" << dtype.Prefix() << attr.Name() << "</td>\n"
            << "    <td>" << attr.Denotation() << "</td>\n"
            << "</tr>\n";
         }
      os << "</table>\n";
      }
   os << "\n";


   os << "\\name constructors and destructor for this class\n"
      << "\\{\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::" << ClassName() << "()\n"
      << "\\brief standard constructor for " << ClassName() << '\n'
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::" << ClassName() << "(" << ClassName() << " const& other)\n"
      << "\\brief copy constructor for " << ClassName() << '\n'
      << "\\param[in] other reference of an instance of an other instance of " << FullyQualifiedSourceName() << " to copy\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::" << ClassName() << "(" << ClassName() << "&&)\n"
      << "\\brief rvalue constructor for " << ClassName() << '\n'
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::~" << ClassName() << "()\n"
      << "\\brief destructor for " << ClassName() << '\n';
   os << "\\}\n\n";

   os << "\\name public functions for this class (following the framework for this project)\n"
      << "\\{\n"
      << "\\fn " << FullyQualifiedSourceName() << "::swap(" << ClassName() << "& other)"
      << "\\brief swap method for elements of this class, swap the paramter with the instance variables of this instance\n"
      << "\\param[in] other reference of an other instance of the same type to swap the content\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::init();\n"
      << "\\brief method to init this instance with empty / default values (reset it)\n"
      << "\n"
      << "\\fn " << FullyQualifiedSourceName() << "::init(primary_key const& key_values)\n"
      << "\\brief method to initialiaze an instance and copy the values of a primary key into it\n"
      << "\\param[in] key_values a const reference to primary_key type to init this instance\n"
      << "\\returns a reference to the instance\n"
      << "\n";

   if (Dictionary().UseBaseClass()) {
      auto param_ty = Dictionary().BaseNamespace() != Namespace() ? Dictionary().BaseNamespace() + "::"s + Dictionary().BaseClass() : Dictionary().BaseClass();
      os << std::format("\\fn {}::copy({} const& other);\n", FullyQualifiedSourceName(), param_ty)
         << "method to copy instances of the hierachie of classes into another\n\n";
      }
   else {
      os << std::format("\\fn {}::copy({} const& other);\n", FullyQualifiedSourceName(), ClassName())
         << "method to copy instances of this classes into another\n\n";
      }
   os << "\\}\n\n";


   // ------------------------- write documention for all the attributs ------------------------------
   os << "\\name direct data elements from table\n"
      << "\\{\n\n";
   for (auto const& [attr, dtype] : processing_data) {
      std::string strFullAttr = FullyQualifiedSourceName() + "::"s + dtype.Prefix() + attr.Name();
      std::string strNote1 = "Source: member "s + strFullAttr + " in class " + FullyQualifiedSourceName() + " with the type std::optional<"s + dtype.SourceType() + ">"s;
      std::string strNote2 = "Database: attribute \""s + attr.DBName() + "\" in entity \\ref "s + Doc_RefName() + " with database type "s + dtype.DatabaseType();

      // direct data elements
      os << "\\var " << strFullAttr << "\n";
      if (attr.Denotation().size() > 0) 
         os << "\\brief " << attr.Denotation() << '\n'
            << "\\details " << strNote1 << '\n';
      else 
         os << "\\brief " << strNote1 << '\n';
      os << "\\details " << strNote2 << '\n';
      if (attr.Description().size() > 0) {
         std::ranges::for_each(attr.Description() | std::views::split('\n') 
                                                  | std::views::filter([](const auto& line) { return !line.empty(); }), [&os](const auto& line) {
            os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
            });
         }
      if (attr.Comment().size() > 0) {
         std::ranges::for_each(attr.Comment() | std::views::split('\n')
                                              | std::views::filter([](const auto& line) { return !line.empty(); }), [&os](const auto& line) {
            os << "\\note "s << std::string_view(line.begin(), line.end()) << '\n';
            });
         }
      os << "\n";
      }
   os << "\n";
   os << "\\}\n\n";



   // ------------------------------ write documention for public selector with optional
   os << "\\name public selectors with an optional retval for direct access to attributes\n"
      << "\\{\n\n";
   for (auto const& [attr, dtype] : processing_data) {
      std::string strFullAttr = FullyQualifiedSourceName() + "::"s + dtype.Prefix() + attr.Name();
      std::string strFullFunc1 = FullyQualifiedSourceName() + "::"s + attr.Name();  // Name for Selectors, Manipulators as "Property"
      std::string strRetVal = "std::optional<"s + dtype.SourceType() + ">"s;

      os << "\\fn " << strRetVal << " const& " << strFullFunc1 << "() const\n";
      os << "\\brief selector for the data element " << strFullAttr << '\n';
      if (attr.Denotation().size() > 0) os << "\\details <b>" << attr.Denotation() << "</b>\n";
      os << "\\returns " << strRetVal << " const&: Reference to the data element " << strFullAttr << '\n';
      }
   os << "\n";
   os << "\\}\n\n";

   // ------------------------------ write documention for public special selectors 
   os << "\\name public selectors with an direct access to attributes\n"
      << "\\{\n\n";
   for (auto const& [attr, dtype] : processing_data) {
      std::string strRetVal = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
      std::string strSelector = FullyQualifiedSourceName() + "::_"s + attr.Name();
      std::string strComment = attr.Comment_Attribute();
      std::string strFullAttr = FullyQualifiedSourceName() + "::"s + dtype.Prefix() + attr.Name();

      os << "\\fn " << strRetVal << " " << strSelector << "() const\n";
      os << "\\brief special selector for data element " << strFullAttr << '\n';
      os << "\\details " << strComment << "\n";
      if (attr.Denotation().size() > 0) os << "\\details <b>" << attr.Denotation() << "</b>\n";
      os << "\\throw std::runtime::error if the attribute " << strFullAttr << " is empty\n";
      os << "\\returns " << strRetVal << " const&: Reference to the data element " << strFullAttr << '\n';
      }
   os << "\n";
   os << "\\}\n\n";


   // ------------------------------ write documention for public manipulators 
   // views don't have public manipulators, calculated fields don't have manipulators
   if (EntityType() != EMyEntityType::view && std::ranges::any_of(processing_data, [](auto const& p) { return !std::get<0>(p).IsComputed(); })) {
      os << "\\name public manipulators for direct access to the attributes\n"
         << "\\{\n\n";
      for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& p) { return !std::get<0>(p). IsComputed(); })) {
         std::string strFullAttr = FullyQualifiedSourceName() + "::"s + dtype.Prefix() + attr.Name();
         std::string strFullFunc1 = FullyQualifiedSourceName() + "::"s + attr.Name();  // Name for Selectors, Manipulators as "Property"
         std::string strRetVal = "std::optional<"s + dtype.SourceType() + ">"s;

         os << "\\fn " << strRetVal << " const& " << strFullFunc1 << "(" << strRetVal << " const& newVal)\n";
         os << "\\brief manipulator for the data element " << strFullAttr << '\n';
         if (attr.Denotation().size() > 0) os << "\\details <b>" << attr.Denotation() << "</b>\n";
         os << "\\param[in] newVal the new Value for the member " << strFullAttr << '\n';
         os << "\\returns " << strRetVal << " const&: Reference to the data element " << strFullAttr << '\n';
         os << "\n";
         }
      os << "\n";
      os << "\\}\n\n";
      }

   // ------------------------------ write documention for private manipulators 
   // views have only private manipulators, calculated fields have only private manipulators
   auto comp_attr = processing_data | std::views::filter([this](auto const& val) { return EntityType() == EMyEntityType::view || val.first.IsComputed(); });
   if (!comp_attr.empty()) {
      os << "\\name private manipulators for direct access to the attributes (for views or calculated fields)\n"
         << "\\{\n\n";
      for (auto const& [attr, dtype] : comp_attr) {
         std::string strFullAttr = FullyQualifiedSourceName() + "::"s + dtype.Prefix() + attr.Name();
         std::string strFullFunc1 = FullyQualifiedSourceName() + "::"s + attr.Name();  // Name for Selectors, Manipulators as "Property"
         std::string strRetVal = "std::optional<"s + dtype.SourceType() + ">"s;

         os << "\\fn " << strRetVal << " const& " << strFullFunc1 << "(" << strRetVal << " const& newVal)\n";
         os << "\\brief manipulator for the data element " << strFullAttr << '\n';
         if (attr.Denotation().size() > 0) os << "\\details <b>" << attr.Denotation() << "</b>\n";
         os << "\\param[in] newVal the new Value for the member " << strFullAttr << '\n';
         os << "\\returns " << strRetVal << " const&: Reference to the data element " << strFullAttr << '\n';
         os << "\n";
         }
      os << "\n";
      os << "\\}\n\n";
      }


   os << "\n"; // finishing loop

   return true;
   }

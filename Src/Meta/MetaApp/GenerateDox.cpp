
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

void TMyDictionary::Create_Doxygen(std::ostream& os) const {
   std::ranges::for_each(Tables(), [&os](auto const val) { 
                  os << std::format("\\include{{doc}} {}/{}.dox\n", std::get<1>(val).SrcPath(), std::get<1>(val).Name()); });

   os << "\n\\mainpage " << Denotation() << '\n'
      << "\\tableofcontents\n";
   os << "<a id = \"top_of_mainpage\"></a>\n"
      << "\n\\section secMainDescription project description / preamble\n";
   std::ranges::for_each(Description() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
      });

   os << "\n\\section secMainContent project content\n";
   std::ranges::for_each(Comment() | std::views::split('\n'), [&os](const auto& line) {
      os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n';
      });

   os << "\n\\section secMainDatatypes used datatypes\n"
      << "\\details the following table show datatypes and the transformation into types for the used sql server "
      << "and the programming language. hungarian notation is used in the program. the table contain "
      << "prefixes which are used for the data types.\n"
      << "\\details for some data types(e.g.integers), no exact distinction is made.\n";

   os << "\\details <table>\n"
         "<tr><th>datatype<th>db datatype<th>db check"
         "<th>src type<th>header<th>prefix<th>comment"
      << "</tr>\n";
   
   for(auto const& [name, datatype] : DataTypes()) {
      os << "<tr><td valign=\"top\">" << "<a id=\"datatyp_" << datatype.DataType() << "\"></a>" << datatype.DataType() << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.DatabaseType().size() > 0 ? datatype.DatabaseType() : " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.CheckSeq().size() > 0     ? datatype.CheckSeq() :     " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.SourceType().size() > 0   ? datatype.SourceType() :   " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.Headerfile().size() > 0   ? datatype.Headerfile() :   " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.Prefix().size() > 0       ? datatype.Prefix() :       " &nbsp;"s) << '\n'
         << "    <td valign=\"top\">" << toHTML(datatype.Comment().size() > 0      ? datatype.Comment() :      " &nbsp;"s) << '\n'
         << "</tr>\n";
      }


   os << "</table>\n\n";

   os << "\\section datamodel_er_diagram database diagram\n"
      << "\\details \\dot\n"
      << "graph ER {\n"
      << "   graph [fontname=\"Helvetica\", fontsize=8];\n";
   // create nodes with tables
   std::ranges::for_each(Tables(), [&os](auto const& val) {
            os << "   " << std::get<1>(val).Name() << " [label=\"" << std::get<1>(val).SQLName() 
               << "\", shape=box,fontname=\"Helvetica\", fontsize=8, URL=\"\\ref " << std::get<1>(val).Doc_RefName() << "\"];\n";
            });

   // create nodes with the relationships
   std::ranges::for_each(Tables(), [&os](auto const& table) {
      std::ranges::for_each(std::get<1>(table).References(), [&os](auto const& ref) {
              os << "   " << ref.Name() << " [label=\"" << ref.Description()
                 << "\", shape=diamond,fontname=\"Helvetica\", fontsize=8];\n";
              });
      });

   // create connections between nodes
   std::ranges::for_each(Tables(), [&os](auto const& pair) {
      TMyTable const& table = std::get<1>(pair);
      std::ranges::for_each(table.References(), [&os, &table](auto const& ref) {
         TMyTable const& refTable = table.Dictionary().FindTable(ref.RefTable());
         os << "   " << table.Name() << " -- " << ref.Name() << ";\n"
            << "   " << ref.Name() << " -- " << refTable.Name() << ";\n";
         });
      });


   //os << '\n';
   os << "   label = \"\\n\\nEntity Relation Diagram\\n" << Denotation() << "\"\n"
      << "   }\n"
      << "\\enddot\n\n";

   os << "\\section datamodel_all_tables overview about all tables\n"
      << "\\details the following tables are created or used by the application in the database:\n"
      << "\\details <table>\n"
      << "<tr><th>name\n"
      << "    <th>db name\n"
      << "    <th>source name\n"
      << "    <th>description\n"
      << "</tr>\n";

   for (auto const& [db_name, table] : Tables()) {
      os << "<tr><td>\\ref " << table.Doc_RefName() << '\n'
         << "    <td>" << table.FullyQualifiedSQLName() << '\n'
         << "    <td>" << table.FullyQualifiedSourceName() << '\n'
         << "    <td>" << (table.Comment().size() > 0 ? table.Comment() : " &nbsp;"s) << '\n'
         << "</tr>";
      }
   os << "</table>\n\n"
      << "<hr><a href = \"top_of_mainpage\">top of the page</a>\n";


   os << "\\section datamodel_all_attributes all attributes and relationships in tables\n";

   for (auto const& [db_name, table] : Tables()) {
      os << "\\subsection " << table.Doc_RefName() << ' ' << db_name << '\n';
      
      if (table.Comment().size() > 0)
         std::ranges::for_each(table.Comment() | std::views::split('\n'), [&os](const auto& line) {
                          os << "\\details "s << std::string_view(line.begin(), line.end()) << '\n'; });
      else os << "\\details descritions for the table missed.\n";

      os << "\\details the system class to work with this table is \\ref " << table.FullClassName() << ".\n";

      os << "\n\\subsubsection " << table.Doc_RefName() << "_attributes attributes\n";

      os << "\\details <table>\n"
         << "<tr><th>name\n"
         << "    <th>type\n"
         << "    <th>db type\n"
         << "    <th>size\n"
         << "    <th>not null\n"
         << "    <th>primary\n"
         << "    <th>init seq\n"
         << "    <th>source name\n"
         << "    <th>source type\n"
         << "    <th>description\n"
         << "</tr>\n";
      for (auto const& attr : table.Attributes()) {
         auto const& datatype = attr.Table().Dictionary().FindDataType(attr.DataType());
         os << "<tr><td valign =\"top\">" << attr.Name() << '\n'
            << "    <td valign = \"top\"><a href =\"#datatyp_" << attr.DataType() << "\">" << attr.DataType() << "</a>\n"
            << "    <td valign = \"top\">" << datatype.DatabaseType() << '\n';
         os << std::format("    <td valign = \"top\">{}\n", 
                              (datatype.UseLen() && datatype.UseScale() ? std::format("({}, {})", attr.Len(), attr.Scale()) :
                                               (datatype.UseLen() ? std::format("({})", attr.Len()) : ""s)));
         os << "    <td valign = \"top\">" << std::boolalpha << attr.NotNull() << '\n'
            << "    <td valign = \"top\">" << std::boolalpha << attr.Primary() << '\n'
            << "    <td valign = \"top\">" << toHTML(attr.InitSeq().size() > 0 ? attr.InitSeq() : " &nbsp;"s) << '\n'
            << "    <td valign = \"top\">" << datatype.Prefix() + attr.Name() << '\n'
            << "    <td valign = \"top\">" << datatype.SourceType() << '\n'
            << "    <td valign = \"top\">" << toHTML(attr.Comment().size() > 0 ? attr.Comment() : " &nbsp;"s) << '\n'
            << "</tr>"; 
         }

      os << "</table>\n\n";

      // --------------------------------------------------------------------------------------
      // create a section for the relationships (only when there relationships exists)
      // --------------------------------------------------------------------------------------
      if(!table.References().empty()) {
         os << "\n\\subsubsection " << table.Doc_RefName() << "_relationships relationships\n";

         os << "\\details <table>\n"
            << "<tr><th>type\n"
            << "    <th>related\n"
            << "    <th>Reference\n"
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
         }

      // --------------------------------------------------------------------------------------
      // create a section for the sql statements
      // --------------------------------------------------------------------------------------
      os << "\n\\subsubsection " << table.Doc_RefName() << "_create create statement\n"
         << "\\code{.sql}\n";
      table.SQL_Create_Table(os);
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
      os << "<hr><a href = \"#top_of_mainpage\">top of the page</a>\n";
      }

   os << "<hr>\n";
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
   }


bool TMyTable::CreateDox(std::ostream& os) const {
   os << "\\class " << FullClassName() << '\n'
      << "\\author " << Dictionary().Author() << '\n'
      << "\\date " << CurrentDate() << " documentation for this project created\n"
      << "\\see system class for the table \\ref " << Doc_RefName() << '\n';

   return true;
   }

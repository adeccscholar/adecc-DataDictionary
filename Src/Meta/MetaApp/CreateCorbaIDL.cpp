#include "DataDictionary.h"

#include "DictionaryHelper.h"

#include <sstream>
#include <set>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <locale>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;

bool TMyDictionary::CreateBasicCorbaIDL(std::ostream& out) const {

   out << "module BasicModule {\n\n"
       << "   struct TimePoint {\n"
       << "      long long milliseconds_since_epoch;  // Unix timestamp as milliseconds\n"
       << "      };\n\n"
       << "   struct YearMonthDay {\n"
       << "      long           year;\n"
       << "      unsigned short month;\n"
       << "      unsigned short day;\n"
       << "      };\n\n"
       << "   exception BasicException {\n"
       << "      long      errorCode;\n"
       << "      string    message;\n"
       << "      TimePoint time;\n"
       << "      string    position;\n"
       << "      };\n\n"
       << "   interface BasicInterface {\n"
       << "      TimePoint    get_current_time();\n"
       << "      YearMonthDay get_current_date();\n"
       << "      };\n\n"
       << "   };\n";

   return true;
   }

bool TMyDictionary::CreateBasicCorbaHeader(std::ostream& out) const {
   std::string strImplementationClass = Identifier() + "_Impl"s;
   out << "#pragma once\n"
      << "#include <tao/corba.h>\n\n"
      << std::format("class {} {{\n", "BasicInterface_Impl");
   return true;
   }



std::vector<std::tuple<std::string, std::string, std::vector<size_t>>> TMyDictionary::GetCompositions(TMyTable const& table) const {
   auto parts_of = table.GetPart_ofs(EMyReferenceType::composition);
   return  parts_of          | std::views::transform([this, &table](auto const& p) {
                                     std::string const& tabMod = FindNameSpace(table.Namespace()).CorbaName();
                                     std::string const& refMod = FindNameSpace(std::get<0>(p).Namespace()).CorbaName();
                                     std::string const& refTab = std::get<0>(p).Name();
                                     std::string strRetType = tabMod != refMod ? (refMod + "::"s) + refTab : refTab;
                                     if (std::get<3>(p).size() > 0) strRetType += "Seq";
                                     std::string strFuncName = "Get"s + std::get<0>(p).Name();
                                     auto vecKeys = std::get<4>(p)
                                        | std::views::transform([](auto const& p) { return p.first - 1; })
                                        | std::ranges::to<std::vector>();
                                     for (auto const& key : vecKeys) {
                                        strFuncName += "_"s + table.Attributes()[key].Name();
                                        }
                                     return std::make_tuple(strRetType, strFuncName, vecKeys);
                                     })
                              | std::ranges::to<std::vector>();
   }

std::vector<std::tuple<std::string, std::string, std::string, std::string, std::vector<size_t>>> TMyDictionary::GetRangeValues(TMyTable const& table) const {
   /*
   auto parts_of = table.GetParents(EMyReferenceType::range);
   return  parts_of | std::views::transform([this, &table](auto const& p) {
      std::string const& tabMod = FindNameSpace(table.Namespace()).CorbaName();
      std::string const& refMod = FindNameSpace(std::get<0>(p).Namespace()).CorbaName();
      std::string const& refTab = std::get<0>(p).Name();
      std::string strRetType = tabMod != refMod ? (refMod + "::"s) + refTab : refTab;
      if (std::get<3>(p).size() > 0) strRetType += "Seq";
      std::string strFuncName = "Get"s + std::get<0>(p).Name();
      std::string strRetTypeAll = (tabMod != refMod ? (refMod + "::"s) + refTab : refTab) + "Seq"s;
      std::string strFuncNameAll = "Get"s + std::get<0>(p).Name() + "All"s;
      auto vecKeys = std::get<4>(p)
         | std::views::transform([](auto const& p) { return p.first - 1; })
         | std::ranges::to<std::vector>();
      for (auto const& key : vecKeys) {
         strFuncName += "_"s + table.Attributes()[key].Name();
      }
      return std::make_tuple(strRetType, strFuncName, strRetTypeAll, strFuncNameAll, vecKeys);
      })
      | std::ranges::to<std::vector>();
   */
   return { };
}


bool TMyDictionary::CreateCorbaIDL(std::ostream& out) const {
 
   auto all_attributes = Tables() | std::views::values 
                         | std::views::transform([](auto const& t) { return t.Attributes(); })
                         | std::views::join;

   auto all_datatypes  = all_attributes
                         | std::views::transform([](auto const& a) { return a.Table().Dictionary().FindDataType(a.DataType()); })
                         | std::ranges::to<std::set>();

   auto all_includes = all_datatypes | std::views::transform([](auto const& d) { return d.CorbaModule(); }) 
                                     | std::views::filter([](auto const& i) { return i.size() > 0; })
                                     | std::ranges::to<std::set>();

   for (auto const& inc_file : all_includes) out << std::format("#include \"{}\"\n", inc_file);
   if (all_includes.size() > 0) out << '\n';

   auto max_length = std::ranges::max(all_datatypes | std::views::transform([](auto const & d) { return d.CorbaType().size(); }));

   auto topologic = TopologicalSequence();

   std::string strModule = ""s;
   for (auto const& table_name : topologic) {
      auto const& table = FindTable(table_name);

      auto processing_data = table.GetProcessing_Data();
      auto parents         = table.GetParents(EMyReferenceType::generalization);
      auto ranges_of_data  = table.GetPart_ofs(EMyReferenceType::range);

      if (table.Namespace() != strModule) {
         if (strModule.size() > 0) out << "   };\n\n";
         if (table.Namespace().size() > 0) {
            std::string const& strNewModule = FindNameSpace(table.Namespace()).CorbaName();
            out << "module " << strNewModule << " {\n";
            }
         strModule = table.Namespace();
         }

      bool has_prim_type = false;
      if(auto prim_attr = table.Attributes() | own::views::primary; !prim_attr.empty()) {
         has_prim_type = true;
         out << std::format("{}struct {}_primary {{\n", my_indent(1), table.SourceName());
      
         auto prim_datatypes = prim_attr | std::views::transform([this](auto const& val) { return FindDataType(val.DataType()); });
         auto prim_attr_max_length = std::ranges::max(prim_datatypes | std::views::transform([](auto const& d) { return d.CorbaType().size(); }));
         for(auto const& [attr, dtype] : std::views::zip(prim_attr, prim_datatypes)) {
            out << std::format("{}{:<{}} {};\n", my_indent(2), dtype.CorbaType(), prim_attr_max_length, attr.Name());
            }

         out << my_indent(2) << "};\n\n";
         }


      out << std::format("{}interface {}", my_indent(1), table.SourceName());

      if (!parents.empty()) {
         size_t i = 0;
         std::ranges::for_each(parents, [this, &out, &table, &i](auto const& p) {
            out << (i++ > 0 ? ", " : ": ") 
                << (FindNameSpace(table.Namespace()).CorbaName() != FindNameSpace(p.Namespace()).CorbaName() ? 
                         FindNameSpace(p.Namespace()).CorbaName() + "::"s + p.Name()
                         : p.Name());
            });
         }
      out << " {\n";

      //auto attr_max_length = std::ranges::max(processing_data | std::views::transform([](auto const& d) { return d.second.CorbaType().size(); }));
      auto attr_max_length = std::ranges::max(processing_data | own::views::second
                                                              | std::views::transform([](auto const& d) { return d.CorbaType().size(); }));

      auto comp_attr = processing_data | std::views::filter([this, &table](auto const& val) { return table.EntityType() == EMyEntityType::view || val.first.IsComputed(); });

      if (table.EntityType() != EMyEntityType::view && std::ranges::any_of(processing_data, [](auto const& p) { return !std::get<0>(p).IsComputed(); })) {
         auto attr_txt = [&comp_attr]() { return !comp_attr.empty() ? "attribute          "s : "attribute "s; }();
         for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
            out << std::format("{}{}{:<{}} {};\n", my_indent(2), attr_txt, dtype.CorbaType(), attr_max_length, attr.Name());
            }
         }

      if (!comp_attr.empty()) {
         for (auto const& [attr, dtype] : comp_attr) {
            out << std::format("{}readonly attribute {:<{}} {};\n", my_indent(2), dtype.CorbaType(), attr_max_length, attr.Name());
            }
         }

      if (auto buildfunc = GetCompositions(table);  !buildfunc.empty()) {
         static auto countOccurrences = [](auto const& input) -> std::map<std::string, size_t> {
            std::map<std::string, size_t> result;
            std::ranges::for_each(input | std::views::transform([](auto const& p) { return std::get<1>(p); }),
               [&result](auto const& data) { result[data]++; });
            return result;
            };

         out << '\n';

         auto occurrences = countOccurrences(buildfunc);
         auto ret_max_length = std::ranges::max(buildfunc | std::views::transform([](auto const& b) { return std::get<0>(b).size(); }));

         std::ranges::sort(buildfunc, [](auto const& lhs, auto const& rhs) { return std::get<1>(lhs) < std::get<1>(rhs); });

         for(auto const& [ret_type, func, _] : buildfunc) {
            out << std::format("{}{:<{}} {}();\n", my_indent(2), ret_type, ret_max_length, func);
            }          
         }

      if(has_prim_type) {
         out << '\n';
         out << std::format("{}{}_primary GetPrimary();\n", my_indent(2), table.SourceName());
         }

      if(auto buildfunc = GetRangeValues(table); !buildfunc.empty()) {
         auto ret_max_length = std::ranges::max(buildfunc | std::views::transform([](auto const& b) { return std::get<2>(b).size(); }));
         out << '\n';
         for (auto const& [ret_type, func, ret_type_all, func_all, _] : buildfunc) {
            out << std::format("{}{:<{}} {}();\n", my_indent(2), ret_type, ret_max_length, func);
            out << std::format("{}{:<{}} {}();\n", my_indent(2), ret_type_all, ret_max_length, func_all);
            }

          }

      out << "\n      };\n\n";

      if (has_prim_type) {
         out << std::format("{0:}typedef sequence<{1:}_primary> {1:}_primarySeq;\n", my_indent(1), table.SourceName());
         }

      out << std::format("{0:}typedef sequence<{1:}> {1:}Seq;\n\n", my_indent(1), table.SourceName());
      }
   if (strModule.size() > 0) out << "   };\n\n";

   out << "module " << Identifier() << "Module {\n\n"
       << "   interface SystemFactory {\n";

   {
   auto all_tables = Tables() | std::views::values 
                              | std::views::transform([this](auto const& t) { 
                                     std::string const& strModule = FindNameSpace(t.Namespace()).CorbaName();
                                     return std::make_pair(strModule + "::"s + t.Name() + "Seq"s, "Get"s + t.Name() + "All"s);
                                     })
                              | std::ranges::to<std::vector>();

   //auto max_type_length = std::ranges::max(all_tables | std::views::transform([](auto const& d) { return d.first.size(); }));
   
   auto max_type_length = std::ranges::max(all_tables | own::views::first | own::views::size);

   std::cout << std::format("\nTest: {}\n", max_type_length);
   for (size_t i = 0; auto const& t : all_tables | own::views::first | own::views::size_co) {
      std::cout << (i++ > 0 ? ", " : " ") << t;
      }
   std::cout << "\n";

   


   for(auto const& [type, table] : all_tables) {
      out << std::format("{}{:<{}} {}();\n", my_indent(2), type, max_type_length, table);
      }
   }
   out << "\n";
   {
      auto all_tables = Tables() | std::views::values
                         | std::views::transform([this](auto const& t) {
                               std::string const& strModule = FindNameSpace(t.Namespace()).CorbaName();
                               auto table_keys = t.Attributes() 
                                                         | std::views::filter([](auto const& a) { return a.Primary(); })
                                                         | std::views::transform([this](auto const& attr) { 
                                                                 return std::make_tuple(FindDataType(attr.DataType()).CorbaType(),
                                                                                        "p"s + attr.Name() ); 
                                                                 })
                                                         | std::ranges::to<std::vector>();
                               return std::make_tuple(strModule + "::"s + t.Name(), "Get"s + t.Name(), table_keys);
                               })
                         | std::ranges::to<std::vector>();

      auto max_type_length = std::ranges::max(all_tables | std::views::transform([](auto const& d) { return std::get<0>(d).size(); }));

      for (auto const& [type, func_name, key_attr] : all_tables) {
         out << std::format("{}{:<{}} {}(", my_indent(2), type, max_type_length, func_name);
         for(size_t pos = 0u; auto const& [key_type, key_name] : key_attr) {
            out << std::format("{}{} {}", (pos++ > 0 ? ", in " : "in "), key_type, key_name);
            }
         out << ");\n";
      }

   }

   out << "      };\n\n"
       << "   };\n\n";

   return true;
   }

  
   bool TMyDictionary::CreateCorbaImplementationHeader(std::ostream& out) const {
      return true;
      }


   bool TMyDictionary::CreateCorbaImplementationSource(std::ostream& out, std::string const& strHeader) const {
      out << std::format("#include \"{}\"\n\n", strHeader);
      return true;
      }

   
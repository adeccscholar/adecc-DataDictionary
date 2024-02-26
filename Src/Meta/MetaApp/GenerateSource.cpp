/** \file
 * \brief implementation of the generator of source code from adecc Scholar metadata management
 * \details this file is part of the implementations of the metadata management in the project
            adecc Scholar and containes the functions to generate the c++ source code with the 
            dictionary
* \details this is a spin-off from the previous DataDictionary.cpp file and implement parts of 
            the classes TMyTable and TMyDictionary
   \version 1.0
   \since Version 1.0
   \authors Volker Hillmann (VH)
   \date 10.02.2024 VH extracted from the previous file DataDictionary.cpp
   \copyright copyright &copy; 2024. All rights reserved.
   This project is released under the MIT License.
*/


#include "DataDictionary.h"

#include <sstream>
#include <set>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;

bool TMyTable::CreateHeader(std::ostream& os) const {
   static const std::string strTab = "      ";
   static auto constexpr StartNameDoc = [](size_t tab, std::string const& strName, std::string const& strClass) {
      return std::format("{:>{}}/** \\name {} for class {}\n        \\{{ */\n", ' ', tab, strName, strClass);
      };

   static auto constexpr EndNameDoc = [](size_t tab) {
      return std::format("{:>{}}/// \\}}\n\n", ' ', tab);
      };

   try {
      // write the documentations header for the file of this dataclass
      // -----------------------------------------------------------------------------------------
      os << "/*\n"
         << "* Project: " << Dictionary().Denotation() <<  "\n"
         << "* Definition of the data class " << ClassName() << "\n"
         << "* Content: " << Denotation() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
         if (Dictionary().Copyright().size() > 0) os << "* copyright © " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0)   os << "* " << Dictionary().License() << '\n';
      os << "*/\n\n";
  
      auto parents = GetParents(EMyReferenceType::generalization);
      auto part_of_data = GetPart_ofs(EMyReferenceType::composition);

      // write header files for base classes
      if (!parents.empty()) {
         os << "\n// includes for required header files of base classes\n";
         std::ranges::for_each(parents, [&os](auto const& p) { os << std::format("#include {}\n", p.Include()); });
         }
      else if(Dictionary().UseBaseClass()) {
         os << "\n// includes for common  virtual base class\n";
         os << std::format("#include \"{}\"\n", Dictionary().PathToBase().string());
         }

      // write header files for compositions
      if (!part_of_data.empty()) {
         os << "\n// includes for required header files for part of relationships\n";
         std::ranges::for_each(part_of_data, [&os](auto const& p) { os << std::format("#include {}\n", std::get<0>(p).Include()); } ); 
         }

      // find necessary header files for used datatypes
      auto types = Attributes() | std::views::transform([this](auto const& s) {
                                   auto const& dt = Dictionary().FindDataType(s.DataType());
                                   return dt.Headerfile(); })
                                | std::ranges::to<std::set<std::string>>();

      if (!types.empty()) {
         os << "\n// necessary additional headers for used datatypes\n";
         std::ranges::for_each(types | std::views::filter([](const std::string& line) { return !line.empty(); }),
                        [&os](const std::string& line) { os << "#include " << line << '\n'; });
         }

      os << "\n"
         << "#include <optional>\n"
         << "#include <stdexcept>\n"
         << "#include <map>\n"
         << "#include <tuple>\n"  // possible to avoid this (count of primary keys && count of composed keys < 2
         << "#include <memory>\n" // possible to avoid this when gerneral used std::tuple  !!!
         << "\n";

      auto processing_data = GetProcessing_Data();

      // ---------- determine the maximal width for attributes ---------------------------------
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


      // open the namespace when needed
      bool boHasNamespace = Namespace().size() > 0;
      if (boHasNamespace) os //<< Dictionary().FindNameSpace(Namespace()) // possiple exception, don't use this
                             << "namespace " << Namespace() << " {\n\n";

      // write comment to open class
      os << "// " << Denotation() << "\n";
      os << "class " << ClassName();
      
      // write base classes from references
      if (!parents.empty()) {
         size_t i = 0;
         std::ranges::for_each(parents, [this, &os, &i](auto const& p) { 
                                       os << (i++ > 0 ? ", public " : ": public ") 
                                          << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName()); 
                                       });
         }
      else if (Dictionary().UseBaseClass()) {
         os << std::format(" : virtual public {}", (Namespace() != Dictionary().BaseNamespace() ? 
                                                      Dictionary().BaseNamespace() + "::" + Dictionary().BaseClass() : 
                                                      Dictionary().BaseClass()));
         }
      os << " {\n";

      // -------------- generate common datatypes for this table ---------------------------------------
      os << "   public:\n"
         << strTab << "// public datatypes for this table\n";

      auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();
      switch(prim_attr.size()) {
         case 0: throw std::runtime_error("critical error: missing primary key for table.");
         case 1: 
            os << strTab << "using primary_key = " << std::get<1>(prim_attr[0]).SourceType() << ";\n"; 
            break;
         case 2: 
            os << strTab << "using primary_key = std::pair<" << std::get<1>(prim_attr[0]).SourceType()
               << ", " << std::get<1>(prim_attr[1]).SourceType() << ">;\n"; 
            break;
         default:
            os << strTab << "using primary_key = std::tuple<" << std::get<1>(prim_attr[0]).SourceType();
            for (auto const& [_, dt] : prim_attr | std::views::drop(1)) { os << ", " << dt.SourceType(); }
            os << ">;\n";
         }

      os << strTab << "using container_ty = std::map<primary_key, " << ClassName() << ">;\n\n";

      // ---------------- generate datatypes for composed tables ---------------------------------------
      if (!part_of_data.empty()) {
         os << strTab << "// public datatypes for composed tables\n";
         for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            os << "\n" << strTab << "/// datatype for composed table \\ref " << table.Doc_RefName() << '\n';
            os << strTab << "using " << strType << " = ";
            switch(vecKeys.size()) {
               case 0: 
                  os << (Namespace() != table.Namespace() ? table.FullClassName() : table.ClassName());
                  break;
               case 1: 
                  {
                  os << "std::map<" << Dictionary().FindDataType(table.FindAttribute(vecKeys[0]).DataType()).SourceType() << ", "
                     << (Namespace() != table.Namespace() ? table.FullClassName() : table.ClassName())
                     << ">";
                  }
                  break;
               default:
                  //auto strDatatype = Dictionary().FindDataType(table.FindAttribute(vecKeys[0]).Name());
                  ;
               }
            os << ";\n";
            }
         os << "\n";
         }


      // ------------------ generate the attributes for the table  -----------------------------------
      os << "   // private data elements, attributes from table \\ref " << Doc_RefName() << "\n"
         << "   private:\n";

      for (auto const& [attr, dtype] : processing_data) {
         std::string strType = "std::optional<"s + dtype.SourceType() + ">"s;
         std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
         std::string strComment = attr.Comment_Attribute();
         os << std::format("{0}// {5}\n{0}{1:<{2}}{3:<{4}}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr, strComment);
         }

      // --------------- generate data elements for the table which are part of related --------------
      if (!part_of_data.empty()) {
         //os << "\n   private:\n";
         os << "\n" << strTab << "// data elements for composed tables\n";
         for(auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            os << std::format("{0}/// composed data element for the table \\ref {1}\n{0}{2} {3};", strTab, table.Doc_RefName(), strType, strVar);
            }
         }

      os << "\n   public:\n"
         << strTab << "// constructors and destructor\n";
      os << std::format("{0}{1:}();\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} const&);\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} &&);\n", strTab, ClassName())
         << std::format("{0}virtual ~{1:}();\n", strTab, ClassName());
      os << "\n"; 

      os << strTab << "// public functions for this class (following the framework for this project)\n";
      os << std::format("{}void swap({}& rhs) noexcept;\n", strTab, ClassName());
      if (Dictionary().UseBaseClass()) {
         auto param_ty = Dictionary().BaseNamespace() != Namespace() ? Dictionary().BaseNamespace() + "::"s + Dictionary().BaseClass() : Dictionary().BaseClass();
         os << std::format("{0}virtual void init();\n", strTab)
            << std::format("{0}virtual void copy({1} const& other);\n", strTab, param_ty)
            << "\n";
         }
      else {
         os << std::format("{0}void init();\n", strTab)
            << std::format("{0}void copy({1} const& other);\n", strTab, ClassName())
            << "\n";
         }

            // ------------------ generate the selectors direct data elements for the table  -----------------------------------
            os << strTab << "// selectors for the data access to the direct data elements with std::optional retval\n";
            for (auto const& [attr, dtype] : processing_data) {
               std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
               std::string strSelector = attr.Name();
               std::string strAttribute = dtype.Prefix() + attr.Name();
               std::string strComment = attr.Comment_Attribute();
               os << std::format("{0}// selector for {5}\n{0}{1:<{2}}{3}() const {{ return {4}; }}\n", strTab, strRetType, maxLengthType + 22, strSelector, strAttribute, strComment);
               }
            os << "\n";

            for (auto const& [attr, dtype] : processing_data) {
               std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
               std::string strSelector = attr.Name();
               std::string strComment = attr.Comment_Attribute();
               os << std::format("{0}//< special selector with value for {4}\n{0}{1:<{2}}_{3}() const;\n", strTab, strRetType, maxLengthRet, strSelector, strComment);
               }
            os << "\n";

            /*
            // --------------- generate data elements for the table which are part of related --------------
            if (!part_of_data.empty()) {
               //os << "\n   private:\n";
               os << "\n" << strTab << "// data elements for composed tables\n";
               for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
                  os << std::format("{0}/// composed data element for the table \\ref {1}\n{0}{2} {3};", strTab, table.Doc_RefName(), strType, strVar);
               }
            }
            */

            // ------------------ generate the public manipulators for the table  -----------------------------------
            // Attention, views haven't manipulator for data elements, calculated fields don't have a manipulator
            if (EntityType() != EMyEntityType::view && std::ranges::any_of(processing_data, [](auto const& p) { return !std::get<0>(p).IsComputed(); })) {
               os << strTab << "// public manipulators for the class\n";
               for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
                  std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
                  std::string strManipulator = attr.Name();
                  std::string strAttribute = dtype.Prefix() + attr.Name();
                  std::string strComment = attr.Comment_Attribute();
                  os << std::format("{0}// manipulator for {4}\n{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthType + 22, strManipulator, strComment);
                  }
               os << "\n";
               }

            os << "   private:\n";

            // ---------------- generate the private manipulators for the table / views --------------------------------
            auto comp_attr = processing_data | std::views::filter([this](auto const& val) { return EntityType() == EMyEntityType::view || val.first.IsComputed(); });
            if(!comp_attr.empty()) {
               os << strTab << "// private  manipulators for the class\n";
               for (auto const& [attr, dtype] : comp_attr) {
                  std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
                  std::string strManipulator = attr.Name();
                  std::string strAttribute = dtype.Prefix() + attr.Name();
                  std::string strComment = attr.Comment_Attribute();
                  os << std::format("{0}// manipulator for {4}\n{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthType + 22, strManipulator, strComment);
                  }
               os << "\n";
               }
              

            os << StartNameDoc(6, "internal functions", ClassName())
               << "      void _init();\n"
               << "      void _copy(" << ClassName() << " const& other);\n";
            os << EndNameDoc(6);

            os << "   };\n\n";

            // datatypes for the container of this type

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
            if (EntityType() != EMyEntityType::view) {
               os << "// Implementations of the manipulators\n";
               for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
                  std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
                  std::string strManipulator = ClassName() + "::"s + attr.Name();
                  std::string strAttribute = dtype.Prefix() + attr.Name();
                  os << std::format("inline {0} {1}({0} newVal) {{\n   return {2} = newVal;\n   }}\n\n", strRetType, strManipulator, strAttribute);
                  }
               }

            if (boHasNamespace) os << "} // end of namespace " << Namespace() << "\n";
      }
   catch (std::exception& ex) {
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

         os << "/*\n"
            << "* Project: " << Dictionary().Denotation() << "\n"
            << "* Implementation of the data class " << ClassName() << "\n"
            << "* Content: " << Denotation() << "\n"
            << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
         if (Dictionary().Copyright().size() > 0) os << "* copyright " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0)   os << "* " << Dictionary().License() << '\n';
         os << "*/\n\n";

         os << std::format("#include {}\n\n", Include());

         bool boHasNamespace = Namespace().size() > 0;

         if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

         // create the default constructor for the class

         os << ClassName() << "::" << ClassName() << "()";
         if (!parents.empty()) {
            size_t i = 0;
            std::ranges::for_each(parents, [this, &os, &i](auto const& p) { 
                               os << (i++ > 0 ? ", " : " : ")
                                  << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName())
                                  << "()"; });
            }

         os << " {\n"
            << strTab << "_init();\n"
            << strTab << "}\n\n";

         // create the copy constructor for the class
         os << ClassName() << "::" << ClassName() << "(" << ClassName() << " const& other)";
         if (!parents.empty()) {
            size_t i = 0;
            std::ranges::for_each(parents, [this, &os, &i](auto const& p) { 
                               os << (i++ > 0 ? ", " : " : ") 
                                  << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName())
                                  << "(other)"; });
            }

         os << "{\n"
            << strTab << "_copy(other);\n"
            << strTab << "}\n\n";

         // create the destructor
         os << ClassName() << "::" << "~" << ClassName() << "() {"
            << strTab << "}\n\n";


         // _init: internal initialization method for the class

         os << "void " << ClassName() << "::_init() {\n";
         for (auto const& [attr, dtype] : processing_data) {
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("   {0:<{1}} = {{ }};\n", strAttribute, maxLengthAttr);
            }
         os << "   return;\n"
            << "   }\n\n";

         os << "void " << ClassName() << "::_copy(" << ClassName() << " const& other) {\n";
         for (auto const& [attr, dtype] : processing_data) {
            os << std::format("   {0}(other.{0}());\n", attr.Name());
            }
         os << "   return;\n"
            << "   }\n\n";

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

bool TMyDictionary::CreateBaseHeader(std::ostream& os) const {
   if (UseBaseClass()) [[likely]] {
      // write the comment for the base class when used
      // -----------------------------------------------------------------------------------------
      os << "/*\n"
         << "* Project: " << Denotation() << "\n"
         << "* Definition of the base class " << BaseClass() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
      if (Copyright().size() > 0) os << "* copyright © " << Copyright() << '\n';
      if (License().size() > 0)   os << "* " << License() << '\n';
      os << "*/\n\n";

      if(BaseNamespace().size() > 0) os << "namespace " << BaseNamespace() << " {\n\n";
 
      os << "class " << BaseClass() << " {\n"
         << "   public:\n"
         << "      " << BaseClass() << "() = default;\n"
         << "      " << BaseClass() << "(" << BaseClass() << " const&) = default;\n"
         << "      " << BaseClass() << "(" << BaseClass() << "&&) noexcept = default;\n"
         << "      virtual ~" << BaseClass() << "() = default;\n"
         << "\n"
         << "      virtual init() = 0;\n"
         << "      virtual copy(" << BaseClass() << " const&) = 0;\n"
         << "   };\n";

      if (BaseNamespace().size() > 0) os << "\n} // end of namespace " << BaseNamespace() << "\n";
      return true;
      }
   else return false;
   }

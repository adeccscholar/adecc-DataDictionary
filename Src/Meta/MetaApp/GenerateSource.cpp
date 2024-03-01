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

/*
 * \brief method to create the header file
 * \details system class for table
 * \author Volker Hillmann (adecc Scholar)
 * \todo adding methods for compare (key, full) and write (only member and structure)
 * \date 29.02.2024 commentar added belated
 * \todo function isn't complete for handling part_ofs
 * \todo write (for error messages and logs)
 * \todo compare and relational operators
 */

bool TMyTable::CreateHeader(std::ostream& os) const {
   static const std::string strTab = "      ";

   try {
      // write the documentations header for the file of this dataclass
      // -----------------------------------------------------------------------------------------
      os << "// -------------------------------------------------------------------------------------------------\n"
         << "/*\n"
         << "* Project: " << Dictionary().Denotation() <<  "\n"
         << "* Definition of the data class " << ClassName() << "\n"
         << "* Content: " << Denotation() << "\n"
         << "* Date: " << CurrentTimeStamp() << "  file created with adecc Scholar metadata generator\n";
         if (Dictionary().Copyright().size() > 0) os << "* copyright © " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0)   os << "* " << Dictionary().License() << '\n';
      os << "*/\n"
         << "// -------------------------------------------------------------------------------------------------\n"
         << "\n"
         << "#pragma once\n\n";
  
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
      // Attention: Part of Relationships to a relationship table need a second step, and are difficult to handle
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
         << "#include <vector>\n"
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
      os << "// -------------------------------------------------------------------------------------------------\n"
         << "// definition of the system class " << ClassName() << " for\n"
         << "// - " << Denotation() << "\n"
         << "// -------------------------------------------------------------------------------------------------\n"
         << "class " << ClassName();
      
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
      // -----------------------------------------------------------------------------------------------
      os << "   public:\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// public datatypes for this table\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";

      // ------------------ generate the type for the primary key -------------------------------------- 
      // -----------------------------------------------------------------------------------------------
      std::string strKeyGenerate;
      auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();
      switch(prim_attr.size()) {
         case 0: throw std::runtime_error("critical error: missing primary key for table.");
         case 1: 
            os << strTab << "using primary_key  = " << std::get<1>(prim_attr[0]).SourceType() << ";\n";
            strKeyGenerate = "primary_key GetKey() const { return _"s + std::get<0>(prim_attr[0]).Name() + "(); }"s;
            break;
         case 2: 
            os << strTab << "using primary_key  = std::pair<" << std::get<1>(prim_attr[0]).SourceType()
               << ", " << std::get<1>(prim_attr[1]).SourceType() << ">;\n"; 
            strKeyGenerate = "primary_key GetKey() const { return std::make_pair(_"s + std::get<0>(prim_attr[0]).Name() + "(), _"s +
                                 std::get<0>(prim_attr[1]).Name() + "()); }"s;
            break;
         default:
            os << strTab << "using primary_key  = std::tuple<" << std::get<1>(prim_attr[0]).SourceType();
            for (auto const& [_, dt] : prim_attr | std::views::drop(1)) { os << ", " << dt.SourceType(); }
            os << ">;\n";
            strKeyGenerate = "primary_key GetKey() const { return std::make_tuple(_"s + std::get<0>(prim_attr[0]).Name();
            for (auto const& [attr, _] : prim_attr | std::views::drop(1)) strKeyGenerate += ("(), _"s + attr.Name() + "()"s);
            strKeyGenerate += "); }"s;
         }

      // ------------------ create map and vector types for this class ---------------------------------
      // -----------------------------------------------------------------------------------------------
      os << strTab << "using container_ty = std::map<primary_key, " << ClassName() << ">;\n";
      os << strTab << "using vector_ty    = std::vector<" << ClassName() << ">;\n\n";

      // ---------------- generate datatypes for composed tables ---------------------------------------
      // Attention: Part of relationships as aggregation to relations are difficult and should treat other
      // -----------------------------------------------------------------------------------------------
      if (!part_of_data.empty()) {
         os << strTab << "// ----------------------------------------------------------------------------------------------\n"
            << strTab << "// public datatypes for composed tables\n"
            << strTab << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            //os << "\n" << strTab << "/// datatype for composed table \\ref " << table.Doc_RefName() << '\n';
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
                  /// \todo function isn't complete
                  ;
               }
            os << ";\n";
            }
         os << "\n";
         }


      // ---------------------------------------------------------------------------------------------
      // ------------------ generate the attributes for the table  -----------------------------------
      os << "   private:\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// private data elements, direct attributes from table " << SQLName() << "\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";

      for (auto const& [attr, dtype] : processing_data) {
         std::string strType = "std::optional<"s + dtype.SourceType() + ">"s;
         std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
         os << std::format("{0}{1:<{2}}{3:<{4}}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr);
         }

      // --------------- generate data elements for the table which are part of related --------------
      if (!part_of_data.empty()) {
         //os << "\n   private:\n";
         os << "\n" << strTab << "// ----------------------------------------------------------------------------------------------\n"
            << strTab << "// data elements for composed tables\n"
            << strTab << "// ----------------------------------------------------------------------------------------------\n";
         for(auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            os << std::format("{0}{1:<{2}}{3};\n", strTab, strType, maxLengthType + 15, strVar);
            }
         }

      os << "\n   public:\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// constructors and destructor\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";
      os << std::format("{0}{1:}();\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} const&);\n", strTab, ClassName())
         << std::format("{0}{1:}({1:} &&) noexcept;\n", strTab, ClassName())
         << std::format("{0}virtual ~{1:}();\n", strTab, ClassName());
      os << "\n"; 

      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// operators for this class\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << std::format("{0}{1:}& operator = ({1:} const&);\n", strTab, ClassName())
         << std::format("{0}{1:}& operator = ({1:}&&) noexcept;\n", strTab, ClassName())
         << "\n";


      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// public functions for this class (following the framework for this project)\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";
      os << std::format("{}void swap({}& rhs) noexcept;\n", strTab, ClassName());
      if (Dictionary().UseBaseClass()) {
         auto param_ty = Dictionary().BaseNamespace() != Namespace() ? Dictionary().BaseNamespace() + "::"s + Dictionary().BaseClass() : Dictionary().BaseClass();
         os << std::format("{0}virtual void init() override;\n", strTab)
            << std::format("{0}virtual void copy({1} const& other) override;\n", strTab, param_ty)
            << "\n";
         }
      else {
         os << std::format("{0}void init();\n", strTab)
            << std::format("{0}void copy({1} const& other);\n", strTab, ClassName())
            << "\n";
         }

      // ------------------ generate the selectors direct data elements for the table  -----------------------------------
      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// method to extract the key from data\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << strKeyGenerate << "\n"
         << "\n";
      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// selectors for the data access to the direct data elements with std::optional retval\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
         std::string strSelector = attr.Name();
         std::string strAttribute = dtype.Prefix() + attr.Name();
         os << std::format("{0}{1:<{2}}{3}() const {{ return {4}; }}\n", strTab, strRetType, maxLengthType + 22, strSelector, strAttribute);
         }
      os << "\n";

      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// public selectors for direct data access to the values inside std::optional (unboxing)\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
         std::string strSelector = attr.Name();
         std::string strComment = attr.Comment_Attribute();
         //os << std::format("{0}// special selector with value for {4}\n{0}{1:<{2}}_{3}() const;\n", strTab, strRetType, maxLengthRet, strSelector, strComment);
         os << std::format("{0}{1:<{2}}_{3}() const;\n", strTab, strRetType, maxLengthType + 22, strSelector);
          }
      os << "\n";

      if (!part_of_data.empty()) {
         os << strTab << "// ----------------------------------------------------------------------------------------------\n"
            << strTab << "// public selectors for container of composed tables\n"
            << strTab << "// ----------------------------------------------------------------------------------------------\n";
         std::ranges::for_each(part_of_data, [&os](auto const& p) {
                  // table, strType, strVar, vecKeys, vecParams
                  os << strTab << std::get<1>(p) << " const& " << std::get<0>(p).Name() << "() const { return " << std::get<2>(p) << "; }\n";
                  });
         os << "\n";
         }


      // ------------------ generate the public manipulators for the table  -----------------------------------
      // Attention, views haven't manipulator for data elements, calculated fields don't have a manipulator
      if (EntityType() != EMyEntityType::view && std::ranges::any_of(processing_data, [](auto const& p) { return !std::get<0>(p).IsComputed(); })) {
         os << strTab << "// ----------------------------------------------------------------------------------------------\n"
            << strTab << "// public manipulators for the class\n"
            << strTab << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
            std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
            std::string strManipulator = attr.Name();
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthType + 22, strManipulator);
            }
         os << "\n";

         if (!part_of_data.empty()) {
            os << strTab << "// ----------------------------------------------------------------------------------------------\n"
               << strTab << "// public manipulators for container of composed tables\n"
               << strTab << "// ----------------------------------------------------------------------------------------------\n";
            std::ranges::for_each(part_of_data, [&os](auto const& p) { // table, strType, strVar, vecKeys, vecParams
               os << strTab << std::get<1>(p) << "& " << std::get<0>(p).Name() << "()  { return " << std::get<2>(p) << "; }\n";
               });
            os << "\n";
            }
         }

      os << "   private:\n";

      // ---------------- generate the private manipulators for the table / views --------------------------------
      auto comp_attr = processing_data | std::views::filter([this](auto const& val) { return EntityType() == EMyEntityType::view || val.first.IsComputed(); });
      if(!comp_attr.empty()) {
         os << strTab << "// ----------------------------------------------------------------------------------------------\n"
            << strTab << "// private  manipulators for the class\n"
            << strTab << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [attr, dtype] : comp_attr) {
            std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
            std::string strManipulator = attr.Name();
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthType + 22, strManipulator);
            }
         os << "\n";
         }

      if (!part_of_data.empty() && EntityType() == EMyEntityType::view) {
          os << strTab << "// ----------------------------------------------------------------------------------------------\n"
             << strTab << "// private manipulators for container of composed tables\n"
             << strTab << "// ----------------------------------------------------------------------------------------------\n";
          std::ranges::for_each(part_of_data, [&os](auto const& p) { // table, strType, strVar, vecKeys, vecParams
            os << strTab << std::get<1>(p) << "& " << std::get<0>(p).Name() << "()  { return " << std::get<2>(p) << "; }\n";
            });
          os << "\n";
          }

      os << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << strTab << "// internal functions for this class\n"
         << strTab << "// ----------------------------------------------------------------------------------------------\n"
         << std::format("{}void _swap({}& rhs) noexcept;\n", strTab, ClassName())
         << strTab << "void _init();\n"
         << strTab << "void _copy(" << ClassName() << " const& other);\n"
         << "\n";

      os << "   };\n\n";

      os << "// -------------------------------------------------------------------------------------------------\n"
         << "// Implementations of the special selectors for return values instead std::optional\n"
         << "// -------------------------------------------------------------------------------------------------\n";
      for (auto const& [attr, dtype] : processing_data) {
               std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
               std::string strSelector = attr.Name();
               std::string strAttribute = dtype.Prefix() + attr.Name();
               std::string strReturn = std::format("   if({0}) [[likely]] return {0}.value();\n"
                  "   else throw std::runtime_error(\"value for attribute \\\"{1}\\\" in class \\\"{2}\\\" is empty.\");",
                  strAttribute, attr.Name(), ClassName());
               os << std::format("inline {1} {0}::_{2}() const {{\n{3};\n   }}\n\n", ClassName(), strRetType, strSelector, strReturn);
               }



            os << '\n';
            if (EntityType() != EMyEntityType::view) {   // must checked because there should be private manipulators
               os << "// Implementations of the manipulators\n";
               for (auto const& [attr, dtype] : processing_data ) { // | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
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

/*
 * \brief method to create the source file
 * \note alternative adding to the h file as inline code
 * \author Volker Hillmann (adecc Scholar)
 * \todo adding methods for compare (key, full) and write (only member and structure)
 * \date 29.02.2024 commentar added belated
 */

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

         os << std::format("#include {}\n\n", Include())
            << "#include <typeinfo>\n\n";



         bool boHasNamespace = Namespace().size() > 0;

         if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

         // ----------- create the default constructor for the class ----------------------------------

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

         // ---------------  create the copy constructor for the class ---------------------------
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


         // ---------------  create the copy constructor for the class ---------------------------
         os << ClassName() << "::" << ClassName() << "(" << ClassName() << "&& other) noexcept";
         if (!parents.empty()) {
            size_t i = 0;
            std::ranges::for_each(parents, [this, &os, &i](auto const& p) {
               os << (i++ > 0 ? ", " : " : ")
                  << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName())
                  << "(std::move(other))"; });
            }

         os << " {\n"
            << strTab << "_swap(other);\n"
            << strTab << "}\n\n";


         // -------------------- create the destructor ------------------------------------------
         os << ClassName() << "::" << "~" << ClassName() << "() {"
            << strTab << "}\n"
            << "\n";

         // -------------------------- operators -----------------------------------------------
         os << std::format("{0:}& {0:}::operator = ({0:} const& other) {{\n", ClassName())
            << "   copy(other);\n"
            << "   return *this;\n"
            << "   }\n"
            << "\n"
            << std::format("{0:}& {0:}::operator = ({0:}&& other) noexcept {{\n", ClassName())
            << "   swap(other);\n"
            << "   return *this;\n"
            << "   }\n"
            << "\n";

         // ------------------------ swap + init + copy ----------------------------------------
         auto CurrB = [this](TMyTable const& t) {
            return Namespace() != t.Namespace() ? t.FullClassName() : t.ClassName(); 
            };


         // ---------------------------------- swap --------------------------------------------

         os << "void " << ClassName() << "::swap(" << ClassName() << "& other) noexcept {\n";
         std::ranges::for_each(parents, [&os, &CurrB](auto const& p) { os << std::format("   {0}::swap(static_cast<{0}&>(other));\n", CurrB(p)); });
         os << "   _swap(other);\n"
            << "   }\n"
            << "\n";


         // ---------------------------------- init -------------------------------------------

         os << "void " << ClassName() << "::init() {\n";
         std::ranges::for_each(parents, [&os, &CurrB](auto const& p) { os << "   " << CurrB(p) << "::init();\n"; });
         os << "   _init();\n"
            << "   }\n"
            << "\n";

         // ----------------------------------- copy ------------------------------------------

         auto param_ty = Dictionary().BaseNamespace() != Namespace() ? Dictionary().BaseNamespace() + "::"s + Dictionary().BaseClass() : Dictionary().BaseClass();
         if (Dictionary().UseBaseClass()) {
            os << "void " << ClassName() << "::copy(" << param_ty << " const& other) {\n";
            }
         else {
            os << "void " << ClassName() << "::copy(" << ClassName() << " const& other) {\n";
            }

         if (!parents.empty()) {
            std::ranges::for_each(parents, [this, &os](auto const& p) {
               std::string strBaseClass = (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName());
               os << "   " << strBaseClass << "::copy(other);\n";
               });
             }
         if (Dictionary().UseBaseClass()) {
            os << "   try {\n"
               << "      " << ClassName() << " const& ref = dynamic_cast<" << ClassName() << " const&>(other);\n"
               << "      _copy(ref);\n"
               << "      }\n"
               << "   catch(std::bad_cast const&) { }\n"
               << "   }\n";
            }
         else {
            os << "   _copy(other);\n"
               << "   }\n";
            }
         os << "\n";


         // --------------------------- private methods for this class ------------------------------
         auto part_of_data = GetPart_ofs(EMyReferenceType::composition);
         size_t maxLength = 0;
         if (!part_of_data.empty()) {
            auto maxElement = std::ranges::max_element(part_of_data, [](auto const& lhs, auto const& rhs) {
                                       return std::get<2>(lhs).size() < std::get<2>(rhs).size(); });

            if (maxElement != part_of_data.end()) {
               maxLength = std::get<2>(*maxElement).size() + 1;
               }
            }

         os << "// _swap: internal swapping method for the class\n"
            << "void " << ClassName() << "::_swap(" << ClassName() << "& other) noexcept {\n";
         os << "   // swapping own data elements\n";
         for (auto const& [attr, dtype] : processing_data) os << std::format("   std::swap({0}, other.{0});\n", dtype.Prefix() + attr.Name());
         if (!part_of_data.empty()) {
            os << "   // swapping the composed classes\n";
            std::ranges::for_each(part_of_data, [&os, maxLength](auto const& p) {
               os << std::format("   std::swap({0}, other.{0});\n", std::get<2>(p));
               });
         }

         os << "   return;\n"
            << "   }\n"
            << "\n";

         os << "// _init: internal initialization method for the instance of this class\n"
            << "void " << ClassName() << "::_init() {\n";
         os << "   // initializing own data elements\n";
         auto maxSize = std::max(maxLengthAttr, maxLength);
         for (auto const& [attr, dtype] : processing_data) {
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("   {0:<{1}} = {{ }};\n", strAttribute, maxSize);
            }
         if (!part_of_data.empty()) {
            os << "   // initializing the composed classes\n";
            std::ranges::for_each(part_of_data, [&os, maxSize](auto const& p) {
               os << std::format("   {0:<{1}} = {{ }};\n", std::get<2>(p), maxSize);
               });
            }
         os << "   return;\n"
            << "   }\n"
            << "\n";

         os << "// _copy: internal copy method for the class\n"
            << "void " << ClassName() << "::_copy(" << ClassName() << " const& other) {\n";
         for (auto const& [attr, dtype] : processing_data) os << std::format("   {0}(other.{0}());\n", attr.Name());
         if (!part_of_data.empty()) {
            os << "   // copying the composed classes\n";
            std::ranges::for_each(part_of_data, [&os, maxLength](auto const& p) {
               os << std::format("   {0:<{1}} = {0};\n", std::get<2>(p), maxLength);
               });
            }
         os << "   return;\n"
            << "   }\n"
            << "\n";



         if (boHasNamespace) os << "} // end of namespace " << Namespace() << "\n";

      }
   catch (std::exception& ex) {
      std::cerr << ex.what() << '\n';
      return false;
      }
   return true;
   }


bool TMyTable::CreateReadData(std::ostream& os) {
   return true;
   }

bool TMyTable::CreateWriteData(std::ostream& os) {
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
      os << "*/\n\n"
         << "#pragma once\n\n";

      if(BaseNamespace().size() > 0) os << "namespace " << BaseNamespace() << " {\n\n";
 
      os << "class " << BaseClass() << " {\n"
         << "   public:\n"
         << "      " << BaseClass() << "() = default;\n"
         << "      " << BaseClass() << "(" << BaseClass() << " const&) = default;\n"
         << "      " << BaseClass() << "(" << BaseClass() << "&&) noexcept = default;\n"
         << "      virtual ~" << BaseClass() << "() = default;\n"
         << "\n"
         << "      virtual void init() = 0;\n"
         << "      virtual void copy(" << BaseClass() << " const&) = 0;\n"
         << "   };\n";

      if (BaseNamespace().size() > 0) os << "\n} // end of namespace " << BaseNamespace() << "\n";
      return true;
      }
   else return false;
   }

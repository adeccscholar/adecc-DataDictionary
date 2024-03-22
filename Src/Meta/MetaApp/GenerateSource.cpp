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
#include <locale>
#include <format>
#include <ranges>

namespace fs = std::filesystem;
using namespace std::string_literals;


// -----------------------------------------------------------------------------------------
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

      auto inherited = GetPart_ofs(EMyReferenceType::generalization);

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
         << "#include <iostream>\n"
         << "#include <iomanip>\n" // possible std::format, but still is no need to force C++20
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

      if (Dictionary().HasPersistenceClass()) {
         if (Dictionary().PersistenceNamespace().size() > 0) os << "namespace " << Dictionary().PersistenceNamespace() << " {\n";
         os << "   class " << Dictionary().PersistenceClass() << ";\n";
         if (Dictionary().PersistenceNamespace().size() > 0) os << "   }\n\n";
         }


      if (!inherited.empty()) {
         std::string strNamespace { ""s };
         bool boNamespace { false };
         for (auto const& [table, _1, _2, _3, _4] : inherited) {
            if(table.Namespace() != strNamespace) {
               if (table.Namespace() != Namespace()) {
                  if(strNamespace.size() > 0) os << "   }\n\n"s;
                  os << "namespace " << table.Namespace() << " {\n"s;
                  strNamespace = table.Namespace();
                  boNamespace = true;
                  }
               }
            os << "   class " << table.ClassName() << ";\n";
            }
         if (strNamespace.size() > 0) os << "   }\n\n"s;
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

      if (Dictionary().HasPersistenceClass()) os << "   friend class " << Dictionary().FullPersistenceClass() << ";\n";


      // -----------------------------------------------------------------------------------------------
      // -------------- generate common datatypes for this table ---------------------------------------
      // -----------------------------------------------------------------------------------------------
      os << "   public:\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// public datatypes for this table\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";

      // ----------------------- generate the type for the primary key ------------------------------------------- 
      // we extented this type to an own internal class type to use constructors and conversions
      // after finishing this class, we will use this for the primary attributes in the encircling class too
      // ---------------------------------------------------------------------------------------------------------
      auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();
      if(prim_attr.size() == 0) [[unlikely]] 
         throw std::runtime_error(std::format("critical error: missing primary key for table \"{}\".", Name()));
      else {
         // ------------------------------------------------------------------------------------------
         // determine the maximum length of the identifiers of the primary key attributes 
         // ------------------------------------------------------------------------------------------
         auto maxPrimElement = std::ranges::max_element(prim_attr, [](auto const& a, auto const& b) {
            static auto constexpr len = [](auto const& e) {
               return e.second.Prefix().size() + e.first.Name().size();
               };
            return len(a) < len(b);
            });

         size_t maxLengthPrimAttr = 0;
         if (maxPrimElement != prim_attr.end()) {
            maxLengthPrimAttr = maxPrimElement->second.Prefix().size() + maxPrimElement->first.Name().size() + 1;
            }

         // ------------------------------------------------------------------------------------------
         // determine the maximum length of the used types of the primary key attributes
         // ------------------------------------------------------------------------------------------
         maxPrimElement = std::ranges::max_element(prim_attr, [](auto const& a, auto const& b) {
            return a.second.SourceType().size() < b.second.SourceType().size(); });

         size_t maxLengthPrimType = 0;
         if (maxPrimElement != prim_attr.end()) {
            maxLengthPrimType = maxPrimElement->second.SourceType().size() + 1;
            }

         // ------------------------------------------------------------------------------------------
         //    create the primary key class with the encircling class as friend
         // ------------------------------------------------------------------------------------------
         os << my_indent(2) << "class primary_key {\n"
            << my_indent(3) <<    "friend class " << ClassName() << ";\n"
            << my_indent(3) <<    "friend std::ostream& operator << (std::ostream& out, primary_key const& data) { "
            << "return data.write(out); }\n" 
            << my_indent(3) <<    "private:\n";
         // member for the primary key attributes
         for (auto const& [attr, dtype] : prim_attr) {
            std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
            os << std::format("{0}{1:<{2}}{3:<{4}}\n", my_indent(4), dtype.SourceType(), maxLengthPrimType, strAttribute, maxLengthAttr);
            }
         os << "\n";

         // standard constructor for the primary_key type
         os << std::format("{0}primary_key();\n", my_indent(4));

         os << my_indent(3) << "public:\n";

         // initialize constructor for the primary_key type
         std::string strAttr = std::get<1>(prim_attr[0]).Prefix() + std::get<0>(prim_attr[0]).Name();
         os << std::format("{0}{1}primary_key({2}{3} p{4}", my_indent(4),
                                                         prim_attr.size() > 1 ? "" : "explicit ",
                                                         std::get<1>(prim_attr[0]).SourceType(),
                                                         (std::get<1>(prim_attr[0]).UseReference() ? " const&" : ""),
                                                         std::get<0>(prim_attr[0]).Name());
         for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) 
            os << std::format(", {0}{1} p{2}", dtype.SourceType(), (dtype.UseReference() ? " const&" : ""), attr.Name());
         os << ");\n";

         // constructor for the primary_key type with the incircling class
         // can't be constexpr because Manipulator can throw an exception
         os << std::format("{0}explicit primary_key({1} const& other);\n", my_indent(4), ClassName());

         // copy constructor for the primary_key type
         os << std::format("{0}primary_key(primary_key const& other);\n", my_indent(4));

         // move constructor for the primary_key type
         os << std::format("{0}primary_key(primary_key&& other) noexcept;\n", my_indent(4));

         // constructors for inherited classes
         // can't be constexpr because Manipulator can throw an exception
         // auto inherited = GetPart_ofs(EMyReferenceType::generalization);
         if(!inherited.empty()) {
            for(auto const& [table, type_name, var_name, key_val, key_pairs] : inherited) {
               os << std::format("{0}primary_key({1} const& other);\n", my_indent(4), table.FullClassName());
               }
            }

         os << std::format("{}~primary_key() {{ }}\n", my_indent(4));

         os << '\n'
            << my_indent(4) << "// conversions operator for this element to the encircling class\n"
            << std::format("{0}operator {1}() const;\n", my_indent(4), ClassName());

         os << '\n'
            << my_indent(4) << "// relational operators of the primary type class\n"
            << my_indent(4) << "bool operator == (primary_key const& other) const { return _compare(other) == 0; }\n"
            << my_indent(4) << "bool operator != (primary_key const& other) const { return _compare(other) != 0; }\n"
            << my_indent(4) << "bool operator <  (primary_key const& other) const { return _compare(other) <  0; }\n"
            << my_indent(4) << "bool operator <= (primary_key const& other) const { return _compare(other) <= 0; }\n"
            << my_indent(4) << "bool operator >  (primary_key const& other) const { return _compare(other) >  0; }\n"
            << my_indent(4) << "bool operator >= (primary_key const& other) const { return _compare(other) >= 0; }\n";

         os << '\n'
            << my_indent(4) << "// selectors the primary type class\n";
         for (auto const& [attr, dtype] : prim_attr) {
            std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? "const& "s : " ");
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}() const {{ return {4}; }}\n", my_indent(4), strRetType, maxLengthPrimType + 7, attr.Name(), strAttribute);
            }
         os << '\n'
            << my_indent(4) << "// manipulators the primary type class\n";
         for (auto const& [attr, dtype] : prim_attr) {
            std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? "const& "s : " ");
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}({1}newVal) {{ return {4} = newVal; }}\n", my_indent(4), strRetType, maxLengthPrimType + 7, attr.Name(), strAttribute);
            }
         os << "\n";

         os << my_indent(4) << "// method to write elements of the primary key type class to a stream\n"
            << my_indent(4) << "std::ostream& write(std::ostream& out) const;\n"
            << "\n";

         os << my_indent(3) << "private:\n";

         os << my_indent(4) << "int _compare(primary_key const& other) const;\n"
            << my_indent(3) << "};\n\n";
         }
      /*
      std::string strKeyGenerate;
      auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();
      switch(prim_attr.size()) {
         case 0: throw std::runtime_error("critical error: missing primary key for table.");
         case 1: 
            os << my_indent(2) << "using primary_key  = " << std::get<1>(prim_attr[0]).SourceType() << ";\n";
            strKeyGenerate = "primary_key GetKey() const { return _"s + std::get<0>(prim_attr[0]).Name() + "(); }"s;
            break;
         case 2: 
            os << my_indent(2) << "using primary_key  = std::pair<" << std::get<1>(prim_attr[0]).SourceType()
               << ", " << std::get<1>(prim_attr[1]).SourceType() << ">;\n"; 
            strKeyGenerate = "primary_key GetKey() const { return std::make_pair(_"s + std::get<0>(prim_attr[0]).Name() + "(), _"s +
                                 std::get<0>(prim_attr[1]).Name() + "()); }"s;
            break;
         default:
            os << my_indent(2) << "using primary_key  = std::tuple<" << std::get<1>(prim_attr[0]).SourceType();
            for (auto const& [_, dt] : prim_attr | std::views::drop(1)) { os << ", " << dt.SourceType(); }
            os << ">;\n";
            strKeyGenerate = "primary_key GetKey() const { return std::make_tuple(_"s + std::get<0>(prim_attr[0]).Name();
            for (auto const& [attr, _] : prim_attr | std::views::drop(1)) strKeyGenerate += ("(), _"s + attr.Name() + "()"s);
            strKeyGenerate += "); }"s;
         }
      */

      // ------------------ create map and vector types for this class ---------------------------------
      // -----------------------------------------------------------------------------------------------
      os << my_indent(2) << "using container_ty = std::map<primary_key, " << ClassName() << ">;\n";
      os << my_indent(2) << "using vector_ty    = std::vector<" << ClassName() << ">;\n\n";

      // ---------------- generate datatypes for composed tables ---------------------------------------
      // Attention: Part of relationships as aggregation to relations are difficult and should treat other
      // -----------------------------------------------------------------------------------------------
      if (!part_of_data.empty()) {
         os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
            << my_indent(2) << "// public datatypes for composed tables\n"
            << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            //os << "\n" << my_indent(2) << "/// datatype for composed table \\ref " << table.Doc_RefName() << '\n';
            os << my_indent(2) << "using " << strType << " = ";
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
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// private data elements, direct attributes from table " << SQLName() << "\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";

      for (auto const& [attr, dtype] : processing_data) {
         std::string strType = "std::optional<"s + dtype.SourceType() + ">"s;
         std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
         os << std::format("{0}{1:<{2}}{3:<{4}}\n", my_indent(2), strType, maxLengthType + 15, strAttribute, maxLengthAttr);
         }

      // --------------- generate data elements for the table which are part of related --------------
      if (!part_of_data.empty()) {
         //os << "\n   private:\n";
         os << "\n" << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
            << my_indent(2) << "// data elements for composed tables\n"
            << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
         for(auto const& [table, strType, strVar, vecKeys, vecParams] : part_of_data) {
            os << std::format("{0}{1:<{2}}{3};\n", my_indent(2), strType, maxLengthType + 15, strVar);
            }
         }

      // --------------- generate necessary constructors and the destructor --------------------------
      os << "\n   public:\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// constructors and destructor\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
      os << std::format("{0}{1:}();\n", my_indent(2), ClassName())
         << std::format("{0}{1:}({1:} const&);\n", my_indent(2), ClassName())
         << std::format("{0}{1:}({1:} &&) noexcept;\n", my_indent(2), ClassName())
         << std::format("{0}explicit {1:}(primary_key const&);\n", my_indent(2), ClassName())
         << std::format("{0}virtual ~{1:}();\n", my_indent(2), ClassName());
      os << "\n"; 

      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// operators for this class\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << std::format("{0}{1:}& operator = ({1:} const&);\n", my_indent(2), ClassName())
         << std::format("{0}{1:}& operator = ({1:}&&) noexcept;\n", my_indent(2), ClassName())
         << "\n"
         << std::format("{0}operator primary_key () const {{ return GetKey(); }}\n", my_indent(2));

      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// public functions for this class (following the framework for this project)\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
      os << std::format("{}void swap({}& rhs) noexcept;\n", my_indent(2), ClassName());
      if (Dictionary().UseBaseClass()) {
         auto param_ty = Dictionary().BaseNamespace() != Namespace() ? Dictionary().BaseNamespace() + "::"s + Dictionary().BaseClass() : Dictionary().BaseClass();
         os << std::format("{0}virtual void init() override;\n", my_indent(2))
            << std::format("{0}virtual void copy({1} const& other) override;\n", my_indent(2), param_ty)
            << "\n";
         }
      else {
         os << std::format("{0}void init();\n", my_indent(2))
            << std::format("{0}void copy({1} const& other);\n", my_indent(2), ClassName())
            << "\n";
         }
      os << std::format("{0}{1}& init(primary_key const&);\n", my_indent(2), ClassName())
         << "\n";

      // ------------------ generate the selectors direct data elements for the table  -----------------------------------
      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// method to extract the key from data\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         //<< my_indent(2) << strKeyGenerate << "\n"
         << my_indent(2) << "primary_key GetKey() const { return primary_key(*this); };\n"
         << "\n";
      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// selectors for the data access to the direct data elements with std::optional retval\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
         std::string strSelector = attr.Name();
         std::string strAttribute = dtype.Prefix() + attr.Name();
         os << std::format("{0}{1:<{2}}{3}() const {{ return {4}; }}\n", my_indent(2), strRetType, maxLengthType + 22, strSelector, strAttribute);
         }
      os << "\n";

      // ------ public selectors with direct access to the data elements (unboxing) --------------------
      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// public selectors for direct data access to the values inside std::optional (unboxing)\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
      for (auto const& [attr, dtype] : processing_data) {
         std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
         std::string strSelector = attr.Name();
         std::string strComment = attr.Comment_Attribute();
         os << std::format("{0}{1:<{2}}_{3}() const;\n", my_indent(2), strRetType, maxLengthType + 22, strSelector);
         }
      os << "\n";

      // --------------- public selectors for the composed data elements  -----------------------------
      if (!part_of_data.empty()) {
         os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
            << my_indent(2) << "// public selectors for container of composed tables\n"
            << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
         std::ranges::for_each(part_of_data, [&os](auto const& p) {
                  // table, strType, strVar, vecKeys, vecParams
                  os << my_indent(2) << std::get<1>(p) << " const& " << std::get<0>(p).Name() << "() const { return " << std::get<2>(p) << "; }\n";
                  });
         os << "\n";
         }


      // ------------------ generate the public manipulators for the table  -----------------------------------
      // Attention, views haven't public manipulators for data elements, calculated fields don't have public manipulators too
      if (EntityType() != EMyEntityType::view && std::ranges::any_of(processing_data, [](auto const& p) { return !std::get<0>(p).IsComputed(); })) {
         os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
            << my_indent(2) << "// public manipulators for the class\n"
            << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [attr, dtype] : processing_data | std::views::filter([](auto const& val) { return !val.first.IsComputed(); })) {
            std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
            std::string strManipulator = attr.Name();
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}({1} newVal);\n", my_indent(2), strRetType, maxLengthType + 22, strManipulator);
            }
         os << "\n";

         if (!part_of_data.empty()) {
            os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
               << my_indent(2) << "// public manipulators for container of composed tables\n"
               << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
            std::ranges::for_each(part_of_data, [&os](auto const& p) { // table, strType, strVar, vecKeys, vecParams
               os << my_indent(2) << std::get<1>(p) << "& " << std::get<0>(p).Name() << "()  { return " << std::get<2>(p) << "; }\n";
               });
            os << "\n";
            }
         }

      os << "   private:\n";

      // ---------------- generate the private manipulators for the table / views --------------------------------
      auto comp_attr = processing_data | std::views::filter([this](auto const& val) { return EntityType() == EMyEntityType::view || val.first.IsComputed(); });
      if(!comp_attr.empty()) {
         os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
            << my_indent(2) << "// private  manipulators for the class\n"
            << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
         for (auto const& [attr, dtype] : comp_attr) {
            std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
            std::string strManipulator = attr.Name();
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("{0}{1:<{2}}{3}({1} newVal);\n", my_indent(2), strRetType, maxLengthType + 22, strManipulator);
            }
         os << "\n";
         }

      if (!part_of_data.empty() && EntityType() == EMyEntityType::view) {
          os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
             << my_indent(2) << "// private manipulators for container of composed tables\n"
             << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n";
          std::ranges::for_each(part_of_data, [&os](auto const& p) { // table, strType, strVar, vecKeys, vecParams
            os << my_indent(2) << std::get<1>(p) << "& " << std::get<0>(p).Name() << "()  { return " << std::get<2>(p) << "; }\n";
            });
          os << "\n";
          }

      os << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << my_indent(2) << "// internal functions for this class\n"
         << my_indent(2) << "// ----------------------------------------------------------------------------------------------\n"
         << std::format("{}void _swap({}& rhs) noexcept;\n", my_indent(2), ClassName())
         << my_indent(2) << "void _init();\n"
         << my_indent(2) << "void _copy(" << ClassName() << " const& other);\n"
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
         if (Dictionary().Copyright().size() > 0) os << "* copyright ©  " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0)   os << "* " << Dictionary().License() << '\n';
         os << "*/\n\n";

         os << std::format("#include {}\n\n", Include());

         os << "#include <typeinfo>\n\n";

         bool boHasNamespace = Namespace().size() > 0;
         auto prim_attr = processing_data | std::views::filter([](auto const& a) { return std::get<0>(a).Primary(); }) | std::ranges::to<std::vector>();
         if (prim_attr.size() == 0) [[unlikely]]
            throw std::runtime_error(std::format("critical error: missing primary key for table \"{}\".", Name()));
         else {
            // ------------------------------------------------------------------------------------------
            // determine the maximum length of the identifiers of the primary key attributes 
            // ------------------------------------------------------------------------------------------
            auto maxPrimElement = std::ranges::max_element(prim_attr, [](auto const& a, auto const& b) {
               static auto constexpr len = [](auto const& e) {
                  return e.second.Prefix().size() + e.first.Name().size();
                  };
               return len(a) < len(b);
               });

            size_t maxLengthPrimAttr = 0;
            if (maxPrimElement != prim_attr.end()) {
               maxLengthPrimAttr = maxPrimElement->second.Prefix().size() + maxPrimElement->first.Name().size() + 1;
               }

            auto maxPrimName = std::ranges::max_element(prim_attr, [](auto const& a, auto const& b) {
               static auto constexpr len = [](auto const& e) {
                  return e.first.Name().size();
                  };
               return len(a) < len(b);
               });

            size_t maxLengthPrimName = 0;
            if (maxPrimName != prim_attr.end()) {
               maxLengthPrimName = maxPrimName->first.Name().size() + 4; // additional formating
               }


            auto inherited = GetPart_ofs(EMyReferenceType::generalization);
            if (!inherited.empty()) {
               for (auto const& [table, _1, _2, _3, _4] : inherited) {
                  os << std::format("#include {}\n", table.Include());
                  }
               os << "\n";
               }

            if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

            // constructor for inline class primary key

            os << "// ---------------------------------------------------------------------------------------\n"
               << std::format("// implementation for the primary_key class inside of {}\n", ClassName())
               << "// ---------------------------------------------------------------------------------------\n"
               << std::format("{0}::primary_key::primary_key() : ", ClassName())
               << std::get<1>(prim_attr[0]).Prefix() + std::get<0>(prim_attr[0]).Name() + " {}";
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
               os << std::format(", {} {{}}", dtype.Prefix() + attr.Name());
               }
            os << " { }\n\n";

            // initialize constructor for the primary_key type
            std::string strAttr = std::get<1>(prim_attr[0]).Prefix() + std::get<0>(prim_attr[0]).Name();
            os << std::format("{0}::primary_key::primary_key({1}{2} p{3}", ClassName(), std::get<1>(prim_attr[0]).SourceType(),
                                                                           (std::get<1>(prim_attr[0]).UseReference() ? " const&" : ""),
                                                                           std::get<0>(prim_attr[0]).Name());
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1))
                  os << std::format(", {0}{1} p{2}", dtype.SourceType(), (dtype.UseReference() ? " const&" : ""), attr.Name());
            os << ") : " << std::format("{0}(p{1})", strAttr, std::get<0>(prim_attr[0]).Name());
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
               std::string strAttr = dtype.Prefix() + attr.Name();
               os << std::format(", {0}(p{1})", strAttr, attr.Name());
               }
            os << " { }\n\n";

            // copy constructor for incircling class
            os << std::format("{0}::primary_key::primary_key({0} const& other) : {1}(other._{2}())", ClassName(), strAttr,
                                                                                                     std::get<0>(prim_attr[0]).Name());
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
               os << std::format(", {0}(other._{1}())", dtype.Prefix() + attr.Name(), attr.Name());
               }
            os << " { }\n\n";

            // copy constructor for the primary_key type
            os << std::format("{0}::primary_key::primary_key({0}::primary_key const& other) : {1}(other.{1})", ClassName(), strAttr);
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
               std::string strAttr = dtype.Prefix() + attr.Name();
               os << std::format(", {0}(other.{0})", strAttr);
               }
            os << " { }\n\n";

            // move constructor for the primary_key type
            // strAttr initialized still from copy constructor
            os << std::format("{0}::primary_key::primary_key(primary_key&& other) noexcept : {1}(std::move(other.{1}))", ClassName(), strAttr);
            for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
               std::string strAttr = dtype.Prefix() + attr.Name();
               os << std::format(", {0}(std::move(other.{0}))", strAttr);
               }
            os << " { }\n\n";

            // constructors for class which dependent of this class
            if (!inherited.empty()) {
               for (auto const& [table, type_name, var_name, key_val, key_pairs] : inherited) {
                  std::string const& strPrimAttr = FindAttribute(key_pairs[0].first).Name();
                  std::string const& strPrimAttrPrefix = Dictionary().FindDataType(FindAttribute(key_pairs[0].first).DataType()).Prefix();
                  os << std::format("{0}::primary_key::primary_key({1} const& other) : {2}{3}(other._{4}())", ClassName(), table.FullClassName(),
                                                                                                 strPrimAttrPrefix, strPrimAttr,
                                                                                              table.FindAttribute(key_pairs[0].second).Name());
                  for (auto const& [parent_id, inherited_id] : key_pairs | std::views::drop(1)) {
                     std::string const& strPrimAttr = FindAttribute(parent_id).Name();
                     std::string const& strPrimAttrPrefix = Dictionary().FindDataType(FindAttribute(parent_id).DataType()).Prefix();
                     os << std::format(", {0}{1}(other._{2}())", strPrimAttrPrefix, strPrimAttr, table.FindAttribute(inherited_id).Name());
                     }
                  os << " { }\n\n";
                  }
               }


            os << "// conversions operator for this element to the encircling class\n"
               << std::format("{0}::primary_key::operator {0}() const {{\n", ClassName())
               << std::format("{0}{1} ret;\n{0}return ret.init(*this);\n{0}}}\n\n", my_indent(1), ClassName());

            os << "// write method for this primary_key element\n"
               << std::format("std::ostream& {0}::primary_key::write(std::ostream& out) const {{\n", ClassName())
               << std::format("{0}out << \"elements of class {1}::primary_key:\\n\";\n", my_indent(1), ClassName());

            for (auto const& [attr, dtype] : prim_attr) {
               os << std::format("{0}out << std::left << std::setw({1}) << \" - {2}\" << \":\" << {3} << \'\\n\';\n", my_indent(1), 
                                 maxLengthPrimName, attr.Name(), dtype.Prefix() + attr.Name());
               }

            os << my_indent(1) << "return out;\n"
               << my_indent(1) << "}\n\n";

            /*
            os << "// write method for this primary_key element\n"
               << std::format("std::ostream& {0}::primary_key::write(std::ostream& out) const {{\n", ClassName())
               << std::format("{0}out << \"elements of class {1}::primary_key\"\n", my_indent(1), ClassName());

            for (auto const& [attr, dtype] : prim_attr) {
               os << std::format("{0}out << std::left << std::setw({1}) << \"{2}\" << \":\";\n", my_indent(1), maxLengthPrimName, attr.Name())
                  << std::format("{0}if(!{1}) out << \"<empty>\";\n{0}else out << *{1};\n\n", my_indent(1), dtype.Prefix() + attr.Name());
               }

            os << my_indent(1) << "return out;\n"
               << my_indent(1) << "}\n\n";
            */

            os << std::format("int {0}::primary_key::_compare(primary_key const& other) const {{\n", ClassName())
               << my_indent(1) << "static auto constexpr comp_help = [](auto const& lhs, auto const& rhs) -> int {\n"
               << my_indent(2) << "return (lhs < rhs ? -1 : (lhs > rhs ? 1 : 0));\n"
               << my_indent(2) << "};\n\n";

            for (auto const& [attr, dtype] : prim_attr) {
               os << std::format("{0}if(auto ret = comp_help(this->{1}, other.{1}); ret != 0) return ret;\n", my_indent(1), dtype.Prefix() + attr.Name());
            }
            os << my_indent(1) << "return 0;\n"
               << my_indent(1) << "}\n\n";
            
            }  // possible to move this bracket deeper because an exception thrown


         // ----------- create the default constructor for the class ----------------------------------
         os << "// ---------------------------------------------------------------------------------------\n"
            << std::format("// implementation of the class {}\n", ClassName())
            <<  "// ---------------------------------------------------------------------------------------\n"
            << std::format("{0}::{0}()", ClassName());
         if (!parents.empty()) {
            size_t i = 0;
            std::ranges::for_each(parents, [this, &os, &i](auto const& p) { 
                               os << (i++ > 0 ? ", " : " : ")
                                  << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName())
                                  << "()"; });
            }

         os << " {\n"
            << my_indent(1) << "_init();\n"
            << my_indent(1) << "}\n\n";

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
            << my_indent(1) << "_copy(other);\n"
            << my_indent(1) << "}\n\n";


         // ---------------  create the move constructor for the class ---------------------------
         os << ClassName() << "::" << ClassName() << "(" << ClassName() << "&& other) noexcept";
         if (!parents.empty()) {
            size_t i = 0;
            std::ranges::for_each(parents, [this, &os, &i](auto const& p) {
               os << (i++ > 0 ? ", " : " : ")
                  << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName())
                  << "(std::move(other))"; });
            }

         os << " {\n"
            << my_indent(1) << "_swap(other);\n"
            << my_indent(1) << "}\n\n";


         // create an initialize operator for the primary_key class
         os << std::format("{0}::{0}(primary_key const& other) : {1}(other.{2}())", ClassName(), 
                             std::get<1>(prim_attr[0]).Prefix() + std::get<0>(prim_attr[0]).Name(), std::get<0>(prim_attr[0]).Name());
         for (auto const& [attr, dtype] : prim_attr | std::views::drop(1)) {
            os << std::format(", {0}(other.{1}())", dtype.Prefix() + attr.Name(), attr.Name());
         }
         os << " { }\n\n";


         // -------------------- create the destructor ------------------------------------------
         os << ClassName() << "::" << "~" << ClassName() << "() {"
            << my_indent(1) << "}\n"
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


         os << "// init(primary_key const&): initialization method for the instance with the primary key attributes\n"
            << std::format("{0}& {0}::init(primary_key const& key_values) {{\n", ClassName())
            << my_indent(1) << "init();\n";
         for (auto const& [attr, _] : processing_data | std::views::filter([](auto const& p) { return std::get<0>(p).Primary(); })) {
            os << std::format("{0}{1}(key_values.{1}());\n", my_indent(1), attr.Name());
            }
         os << my_indent(1) << "return *this;\n"
            << my_indent(1) << "}\n"
            << "\n";


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
            os << std::format("   {0:<{1}} = {2};\n", strAttribute, maxSize, (attr.InitSeq().size() > 0 ? attr.InitSeq() : "{}"s));
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
               os << std::format("   {0:<{1}} = other.{0};\n", std::get<2>(p), maxLength);
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



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
      // write the file documentation for the header file of dataclasses
      // -----------------------------------------------------------------------------------------
      os << "/** \\file\n"
         << "  * \\brief definition of dataclass " << ClassName() << " of table \\ref " << Doc_RefName()
               << " in dictionary \"" << Dictionary().Name() << "\"\n";
      if (Comment().size() > 0)  os << "  * \\details " << Comment() << '\n';

      os << "  * \\see table \\ref " << Doc_RefName() << '\n';
      os << "  * \\version " << Dictionary().Version() << '\n'
         << "  * \\author " << Dictionary().Author() << '\n'
         << "  * \\date " << CurrentDate() << "  file created with adecc Scholar metadata generator\n";

      if (Dictionary().Copyright().size() > 0) {
         os << "  * \\copyright " << Dictionary().Copyright() << '\n';
         if (Dictionary().License().size() > 0) {
            os << "  * " << Dictionary().License() << '\n';
            }
         }
      os << "\n */\n\n";

      // find n
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

            if (!parents.empty()) {
               std::ranges::for_each(parents, [&os](auto const& p) { os << std::format("#include {}\n", p.Include()); });
               }

            os << "#include <optional>\n\n";

            bool boHasNamespace = Namespace().size() > 0;

            if (boHasNamespace) os << "namespace " << Namespace() << " {\n\n";

            os << "/// " << Comment() << "\n";
            os << "class " << ClassName();
            if (!parents.empty()) {
               size_t i = 0;
               std::ranges::for_each(parents, [this, &os, &i](auto const& p) { 
                                       os << (i++ > 0 ? ", public " : ": public ") 
                                          << (Namespace() != p.Namespace() ? p.FullClassName() : p.ClassName()); 
                                       });
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
               std::string strType = "std::optional<"s + dtype.SourceType() + ">"s;
               std::string strAttribute = dtype.Prefix() + attr.Name() + ";"s;
               std::string strComment = attr.Comment_Attribute();
               //os << std::format("{0}{1:<{2}}{3:<{4}} ///< {5}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr, strComment);
               os << std::format("{0}/// {5}\n{0}{1:<{2}}{3:<{4}}\n", strTab, strType, maxLengthType + 15, strAttribute, maxLengthAttr, strComment);
               }

            os << "\n   public:\n" << StartNameDoc(6, "constructors and destructor", ClassName());
            os << std::format("{0}{1:}();\n", strTab, ClassName())
               << std::format("{0}{1:}({1:} const&);\n", strTab, ClassName())
               << std::format("{0}{1:}({1:} &&);\n\n", strTab, ClassName())
               << std::format("{0}virtual ~{1:}();\n\n", strTab, ClassName());
            os << EndNameDoc(6);

            os << StartNameDoc(6, "virtual functions", ClassName());
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
               std::string strRetType = dtype.SourceType() + (dtype.UseReference() ? " const&"s : ""s);
               std::string strSelector = attr.Name();
               std::string strComment = attr.Comment_Attribute();
               os << std::format("{0}///< selector with value for {4}\n{0}{1:<{2}}_{3}() const;\n", strTab, strRetType, maxLengthRet, strSelector, strComment);
               }
            os << EndNameDoc(6);

            // ------------------ generate the manipulators for the table  -----------------------------------
            os << StartNameDoc(6, "manipulators", ClassName());
            for (auto const& [attr, dtype] : processing_data) {
               std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
               std::string strManipulator = attr.Name();
               std::string strAttribute = dtype.Prefix() + attr.Name();
               std::string strComment = attr.Comment_Attribute();
               //os << std::format("{0}{1:<{2}}{3}({1} newVal); ///< manipulator for {4}\n", strTab, strRetType, maxLengthType + 22, strManipulator, strComment);
               os << std::format("{0}/// manipulator for {4}\n{0}{1:<{2}}{3}({1} newVal);\n", strTab, strRetType, maxLengthType + 22, strManipulator, strComment);
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
               std::string strRetType = "std::optional<"s + dtype.SourceType() + "> const&"s;
               std::string strManipulator = ClassName() + "::"s + attr.Name();
               std::string strAttribute = dtype.Prefix() + attr.Name();
               os << std::format("inline {0} {1}({0} newVal) {{\n   return {2} = newVal;\n   }}\n\n", strRetType, strManipulator, strAttribute);
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


         os << "/** \\file\n"   
            << "  * \\brief implementation file of dataclass " << ClassName() << " of table \\ref " << Doc_RefName() << " in dictionary \"" << Dictionary().Name() << "\"\n";
         if (Comment().size() > 0)
            os << "  * \\details " << Comment() << '\n';
         os << "  * \\see table \\ref " << Doc_RefName() << '\n';

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

         // _init: internal initialization method for the class

         os << "void " << ClassName() << "::_init() {\n";
         for (auto const& [attr, dtype] : processing_data) {
            std::string strAttribute = dtype.Prefix() + attr.Name();
            os << std::format("   {0:<{1}} = {{ }};\n", strAttribute, maxLengthAttr);
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

#pragma once

#include "MyStatements.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <filesystem>
#include <ranges>
#include <format>

namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace std::string_view_literals;

class TMyAttribute;
class TMyReferences;
class TMyIndices;
class TMyTable;
class TMyDictionary;


enum class EQueryType : uint32_t { SelectAll, SelectPrim, UpdateAll, UpdateWithoutPrims, DeleteAll, DeletePrim, Insert,
                                   SelectUnique, SelectIdx };
template <EQueryType ty, EQueryType ref_ty>
concept IsSameType = (ty == ref_ty);

template <EQueryType... types>
struct TypePack {
   template <EQueryType ty>
   static constexpr bool contains() {
      return ((ty == types) || ...);
      }

};


using TableType = TypePack<EQueryType::SelectAll, EQueryType::SelectPrim, EQueryType::UpdateAll, EQueryType::UpdateWithoutPrims,
                           EQueryType::DeleteAll, EQueryType::DeletePrim, EQueryType::Insert>;

using IndexType = TypePack<EQueryType::SelectUnique, EQueryType::SelectIdx>;


template <EQueryType ty>
concept IsTableType = TableType::template contains<ty>();

template <EQueryType ty>
concept IsIndexType = IndexType::template contains<ty>();

//template<EQueryType ty>
//concept always_false = false;

template <typename ty>
concept always_false = false;

class Generator_SQL {
private:
   TMyDictionary const& dictionary;
public:
   Generator_SQL() = delete;
   Generator_SQL(TMyDictionary const& ref) : dictionary { ref } { }
   Generator_SQL(Generator_SQL const&) = delete;
   Generator_SQL(Generator_SQL&&) noexcept = delete;

   TMyDictionary const& Dictionary() const { return dictionary; }

   Generator_SQL const& WriteCreateTable(TMyTable const& table, fs::path file) const;
   Generator_SQL const& WriteCreateTable(TMyTable const& table, std::ostream& os) const;

   Generator_SQL const& WriteCreateView(TMyTable const& table, std::ostream& os) const;

   Generator_SQL const& WriteAlterTable(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WritePrimaryKey(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteForeignKeys(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteUniqueKeys(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteCreateIndices(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteCreateCheckConditions(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteRangeValues(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteCreatePostConditions(TMyTable const& table, std::ostream& os) const;
   Generator_SQL const& WriteCreateCleaning(TMyTable const& table, std::ostream& os) const;

   Generator_SQL const& WriteDescriptions(TMyTable const& table, std::ostream& os) const;


   // methods moved from TDataDictionary
   Generator_SQL const& WriteSQLTables(std::ostream&) const;
   Generator_SQL const& WriteSQLAdditionals(std::ostream&) const;
   Generator_SQL const& WriteSQLRangeValues(std::ostream&) const;
   Generator_SQL const& WriteSQLDocumentation(std::ostream&) const;
   Generator_SQL const& WriteSQLDropTables(std::ostream& os) const;

   template <enum EQueryType type> requires IsTableType<type>
   Generator_SQL const& WriteQueryHeader(TMyTable const& table, std::ostream& os) const;

   template <enum EQueryType type> requires IsTableType<type>
   Generator_SQL const& WriteQuerySource(TMyTable const& table, std::ostream& os) const;

   template <enum EQueryType type> requires IsIndexType<type>
   Generator_SQL const& WriteQueryHeader(TMyTable const& table, TMyIndices const& index, std::ostream& os) const;

   template <enum EQueryType type> requires IsIndexType<type>
   Generator_SQL const& WriteQuerySource(TMyTable const& table, TMyIndices const& index,  std::ostream& os) const;

private:

   myStatements CreateTable_Statements(TMyTable const& table) const;

   myStatements AlterTable_Statements(TMyTable const& table) const;
   myStatements PrimaryKey_Statements(TMyTable const& table) const;
   myStatements ForeignKeys_Statements(TMyTable const& table) const;
   myStatements UniqueKeys_Statements(TMyTable const& table) const;
   myStatements CheckConditions_Statements(TMyTable const& table) const;
   myStatements Indices_Statements(TMyTable const& table) const;

   myStatements RangeValues_Statements(TMyTable const& table) const;
   myStatements PostConditions_Statements(TMyTable const& table) const;
   myStatements Cleaning_Statements(TMyTable const& table) const;

   myStatements CreateView_Statements(TMyTable const& table) const;

   myStatements CreateSelectAll_Statement(TMyTable const& table) const;
   myStatements CreateSelectPrim_Statement(TMyTable const& table) const;
   myStatements CreateSelectUniqueKey_Statement(TMyTable const& table, TMyIndices const& idx) const;
   myStatements CreateSelectIndex_Statement(TMyTable const& table, TMyIndices const& idx) const;
   myStatements CreateUpdateAll_Statement(TMyTable const& table) const;
   myStatements CreateUpdateWithoutPrim_Statement(TMyTable const& table) const;
   myStatements CreateInsert_Statement(TMyTable const& table) const;
   myStatements CreateDeleteAll_Statement(TMyTable const& table) const;
   myStatements CreateDeletePrim_Statement(TMyTable const& table) const;


   std::string  CreateTable_SQLRow(TMyTable const& table, TMyAttribute const& attr, size_t len) const;
   std::string  SQLRow(TMyTable const& table, TMyReferences const& reference) const;
   std::string  SQLRow(TMyTable const& table, TMyIndices const& idx) const;

   constexpr std::string get_name_for_table(TMyTable const& table) const;

   template <enum EQueryType type>
   static constexpr std::string_view GetMethodName();

};

template <enum EQueryType type>
static constexpr std::string_view Generator_SQL::GetMethodName() {
   // ----------------------------------------------------------------------------------------------
   if      constexpr (type == EQueryType::SelectAll)          return "strSQL{}SelectAll"sv;
   else if constexpr (type == EQueryType::SelectPrim)         return "strSQL{}SelectDetail"sv;
   else if constexpr (type == EQueryType::UpdateAll)          return "strSQL{}UpdateWithPrim"sv;
   else if constexpr (type == EQueryType::UpdateWithoutPrims) return "strSQL{}UpdateWithoutPrim"sv;
   else if constexpr (type == EQueryType::DeleteAll)          return "strSQL{}DeleteAll"sv;
   else if constexpr (type == EQueryType::DeletePrim)         return "strSQL{}DeleteDetail"sv;
   else if constexpr (type == EQueryType::Insert)             return "strSQL{}Insert"sv;
   // ---------------------------------------------------------------------------------------------
   else if constexpr (type == EQueryType::SelectUnique)       return "strSQL{}Unq{}"sv;
   else if constexpr (type == EQueryType::SelectIdx)          return "strSQL{}Idx{}"sv;
   // ---------------------------------------------------------------------------------------------
   else static_assert(always_false<type>, "this type isn't supported with this function");
}

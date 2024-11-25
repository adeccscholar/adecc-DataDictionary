#pragma once

#include "MyStatements.h"
#include "TypesSQLGen.h"

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

class TMyAttribute;
class TMyReferences;
class TMyIndices;
class TMyTable;
class TMyDictionary;

// ---------------------------------------------------------------------------

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

   template <enum EQueryType type> requires IsReferenceType<type>
   Generator_SQL const& WriteQueryHeader(TMyTable const& table, TMyReferences const& ref, std::ostream& os) const;

   template <enum EQueryType type> requires IsReferenceType<type>
   Generator_SQL const& WriteQuerySource(TMyTable const& table, TMyReferences const& ref, std::ostream& os) const;


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
   myStatements CreateSelectReference_Statement(TMyTable const& table, TMyReferences const& ref) const;
   myStatements CreateSelectRevReference_Statement(TMyTable const& table, TMyReferences const& ref) const;
   myStatements CreateUpdateAll_Statement(TMyTable const& table) const;
   myStatements CreateUpdateWithoutPrim_Statement(TMyTable const& table) const;
   myStatements CreateInsert_Statement(TMyTable const& table) const;
   myStatements CreateDeleteAll_Statement(TMyTable const& table) const;
   myStatements CreateDeletePrim_Statement(TMyTable const& table) const;


   std::string  CreateTable_SQLRow(TMyTable const& table, TMyAttribute const& attr, size_t len) const;
   std::string  SQLRow(TMyTable const& table, TMyReferences const& reference) const;
   std::string  SQLRow(TMyTable const& table, TMyIndices const& idx) const;

   constexpr std::string get_name_for_table(TMyTable const& table) const;

};



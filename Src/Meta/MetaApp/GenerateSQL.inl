// internal implemenation of template Generator_SQL::WriteQuery in MyDataDictionary
// don't include this file directly

template <enum EQueryType type> requires IsTableType<type>
Generator_SQL const& Generator_SQL::WriteQueryHeader(TMyTable const& table, std::ostream& os) const {
   //std::string strFormat = table.Dictionary().GetMethodName<type>();
   os << "extern const std::string " << std::format(GetSQLQueryName<type>(), table.Name()) << ";\n";
   return *this;
   }

template <enum EQueryType type> requires IsTableType<type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, std::ostream& os) const {
   os << "const std::string " << std::format(GetSQLQueryName<type>(), table.Name()) << " =\n";
   if constexpr (type == EQueryType::SelectAll)               WriteSource(CreateSelectAll_Statement(table), os);
   else if constexpr (type == EQueryType::SelectPrim)         WriteSource(CreateSelectPrim_Statement(table), os);
   else if constexpr (type == EQueryType::UpdateAll)          WriteSource(CreateUpdateAll_Statement(table), os);
   else if constexpr (type == EQueryType::UpdateWithoutPrims) WriteSource(CreateUpdateWithoutPrim_Statement(table), os);
   else if constexpr (type == EQueryType::DeleteAll)          WriteSource(CreateDeleteAll_Statement(table), os);
   else if constexpr (type == EQueryType::DeletePrim)         WriteSource(CreateDeletePrim_Statement(table), os);
   else if constexpr (type == EQueryType::Insert)             WriteSource(CreateInsert_Statement(table), os);
   else static_assert(always_false<type>, "this type isn't supported with this function");
   os << "\n\n";
   return *this;
   }


template <enum EQueryType type> requires IsIndexType<type>
Generator_SQL const& Generator_SQL::WriteQueryHeader(TMyTable const& table, TMyIndices const& index, std::ostream& os) const {
   os << "extern const std::string " << std::format(GetSQLQueryName<type>(), table.Name(), index.Name()) << ";\n";
   return *this;
   }

template <enum EQueryType type> requires IsIndexType<type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, TMyIndices const& index, std::ostream& os) const {
   os << "const std::string " << std::format(GetSQLQueryName<type>(), table.Name(), index.Name()) << " =\n";
   if constexpr      (type == EQueryType::SelectUnique)  WriteSource(CreateSelectUniqueKey_Statement(table, index), os);
   else if constexpr (type == EQueryType::SelectIdx)     WriteSource(CreateSelectIndex_Statement(table, index), os);
   else static_assert(always_false<type>, "this type isn't supported with this function");
   os << "\n\n";
   return *this;
   }


template <enum EQueryType type> requires IsReferenceType<type>
Generator_SQL const& Generator_SQL::WriteQueryHeader(TMyTable const& table, TMyReferences const& ref, std::ostream& os) const {
   if constexpr (type == EQueryType::SelectRevRelation)
      os << "extern const std::string " << std::format(GetSQLQueryName<type>(), ref.RefTable(), ref.Name()) << ";\n";
   else if constexpr (type == EQueryType::SelectRelation)
      os << "extern const std::string " << std::format(GetSQLQueryName<type>(), table.Name(), ref.Name()) << ";\n";
   else static_assert(always_false_querytype<type>, "this type isn't supported with this function");
   return *this;
   }

template <enum EQueryType type> requires IsReferenceType<type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, TMyReferences const& ref, std::ostream& os) const {
   if constexpr (type == EQueryType::SelectRelation) {
      os << "const std::string " << std::format(GetSQLQueryName<type>(), table.Name(), ref.Name()) << " =\n";
      WriteSource(CreateSelectReference_Statement(table, ref), os);
      }
   else if constexpr (type == EQueryType::SelectRevRelation) {
      os << "const std::string " << std::format(GetSQLQueryName<type>(), ref.RefTable(), ref.Name()) << " =\n";
      WriteSource(CreateSelectRevReference_Statement(table.Dictionary().FindTable(ref.RefTable()), ref), os);
      }
   else static_assert(always_false_querytype<type>, "this type isn't supported with this function");
   os << "\n\n";
   return *this;
   }

// internal implemenation of template Generator_SQL::WriteQuery in MyDataDictionary
// don't include this file directly

template <enum EQueryType type> requires IsTableType<type>
Generator_SQL const& Generator_SQL::WriteQueryHeader(TMyTable const& table, std::ostream& os) const {
   os << "extern const std::string " << std::format(GetMethodName<type>(), table.Name()) << ";\n";
   return *this;
   }

template <enum EQueryType type> requires IsTableType<type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, std::ostream& os) const {
   os << "const std::string " << std::format(GetMethodName<type>(), table.Name()) << " =\n";
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
   os << "extern const std::string " << std::format(GetMethodName<type>(), table.Name(), index.Name()) << ";\n";
   return *this;
   }

template <enum EQueryType type> requires IsIndexType<type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, TMyIndices const& index, std::ostream& os) const {
   os << "const std::string " << std::format(GetMethodName<type>(), table.Name(), index.Name()) << " =\n";
   if constexpr      (type == EQueryType::SelectUnique)  WriteSource(CreateSelectUniqueKey_Statement(table, index), os);
   else if constexpr (type == EQueryType::SelectIdx)     WriteSource(CreateSelectIndex_Statement(table, index), os);
   else static_assert(always_false<type>, "this type isn't supported with this function");
   os << "\n\n";
   return *this;
   }


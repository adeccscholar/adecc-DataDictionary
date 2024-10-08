// internal implemenation of template Generator_SQL::WriteQuery in MyDataDictionary
// don't include this file directly

template <enum EQueryType type>
Generator_SQL const& Generator_SQL::WriteQueryHeader(TMyTable const& table, std::ostream& os) const {
   os << "extern const std::string " << std::format(GetMethodName<type>(), table.Name()) << ";\n";
   return *this;
   }

template <enum EQueryType type>
Generator_SQL const& Generator_SQL::WriteQuerySource(TMyTable const& table, std::ostream& os) const {
   os << "const std::string " << std::format(GetMethodName<type>(), table.Name()) << " =\n";
   if constexpr (type == EQueryType::SelectAll)               WriteSource(CreateSelectAll_Statement(table), os);
   else if constexpr (type == EQueryType::SelectPrim)         WriteSource(CreateSelectPrim_Statement(table), os);
   else if constexpr (type == EQueryType::UpdateAll)          WriteSource(CreateUpdateAll_Statement(table), os);
   else if constexpr (type == EQueryType::UpdateWithoutPrims) WriteSource(CreateUpdateWithoutPrim_Statement(table), os);
   else if constexpr (type == EQueryType::DeleteAll)          WriteSource(CreateDeleteAll_Statement(table), os);
   else if constexpr (type == EQueryType::DeletePrim)         WriteSource(CreateDeletePrim_Statement(table), os);
   else if constexpr (type == EQueryType::Insert)             WriteSource(CreateInsert_Statement(table), os);
   else throw std::runtime_error("this query type isn't allowed in this function");
   os << "\n\n";
   return *this;
   }


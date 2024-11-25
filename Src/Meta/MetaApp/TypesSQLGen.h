#pragma once

enum class EQueryType : uint32_t { SelectAll, SelectPrim, UpdateAll, UpdateWithoutPrims, DeleteAll, DeletePrim, Insert,
                                   SelectUnique, SelectIdx,
                                   SelectRelation, SelectRevRelation };

template <EQueryType ty, EQueryType ref_ty>
concept IsSameType = (ty == ref_ty);


template <EQueryType... types>
struct TypePack {
   template <EQueryType ty>
   static constexpr bool contains() {
      return ((ty == types) || ...);
      }

};


using TableType     = TypePack<EQueryType::SelectAll, EQueryType::SelectPrim, 
                               EQueryType::UpdateAll, EQueryType::UpdateWithoutPrims, 
                               EQueryType::DeleteAll, EQueryType::DeletePrim, 
                               EQueryType::Insert>;

using IndexType     = TypePack<EQueryType::SelectUnique, EQueryType::SelectIdx>;

using ReferenceType = TypePack<EQueryType::SelectRelation, EQueryType::SelectRevRelation>;


template <EQueryType ty>
concept IsTableType     = TableType::template contains<ty>();

template <EQueryType ty>
concept IsIndexType     = IndexType::template contains<ty>();

template <EQueryType ty>
concept IsReferenceType = ReferenceType::template contains<ty>();

template <EQueryType ty>
concept always_false_querytype = false;

template <typename ty>
concept always_false = false;

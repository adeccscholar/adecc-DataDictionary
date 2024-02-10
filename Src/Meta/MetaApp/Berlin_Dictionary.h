#pragma once

#include "DataDictionary.h"

class TDictionary_Berlin {
private:
   TMyDictionary dictionary;
public:
   TDictionary_Berlin();
   TDictionary_Berlin(TDictionary_Berlin const&) = delete;
   TDictionary_Berlin(TDictionary_Berlin&&) noexcept = delete;
   virtual ~TDictionary_Berlin() = default;

   TMyDictionary const& Dictionary() const { return dictionary; }
   TMyDictionary& Dictionary() { return dictionary; }
};

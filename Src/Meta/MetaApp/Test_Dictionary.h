#pragma once

#include "DataDictionary.h"

class TDictionary_Test {
private:
   TMyDictionary dictionary;
public:
   TDictionary_Test();
   TDictionary_Test(TDictionary_Test const&) = delete;
   TDictionary_Test(TDictionary_Test&&) noexcept = delete;
   virtual ~TDictionary_Test() = default;

   TMyDictionary const& Dictionary() const { return dictionary; }
   TMyDictionary& Dictionary() { return dictionary; }
};

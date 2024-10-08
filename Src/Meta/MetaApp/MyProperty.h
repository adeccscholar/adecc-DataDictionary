#pragma once

#include <iostream>
#include <concepts>
#include <functional>
#include <type_traits>


template <typename func_ty, typename ret_ty>
concept callable_with_ret_ty = requires(func_ty f) {
   { std::function<ret_ty()>(f) };
};

template <typename ret_ty, callable_with_ret_ty<ret_ty> func_ty>
class Property {
   template <typename ty, typename = std::enable_if_t<std::is_convertible_v<ret_ty, std::ostream&>>>
   friend std::ostream& operator << (Property& out, ty const& data) { return out.func() << data; }
public:
   Property(func_ty&& para) : func(std::forward<func_ty>(para)) {}
   operator ret_ty() { return func(); }
private:
   func_ty func;
};

template <typename func_ty>
Property(func_ty) -> Property<decltype(std::declval<func_ty>()()), func_ty>;

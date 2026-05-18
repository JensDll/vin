#include "vin/peel/string_view.hpp"

#include <fmt/base.h>
#include <fmt/format.h>
#include <glib-object.h>
#include <glib.h>
#include <peel/GObject/Object.h>

#include <string_view>

namespace {

void string_view_free(std::string_view* const self)
{
  delete self;
}

std::string_view* string_view_copy(std::string_view* const self)
{
  return self;
}

} // namespace

template<>
peel::GObject::Type peel::GObject::Type::of<std::string_view>()
{
  static ::GType type;

  if (g_once_init_enter_pointer(&type)) {
    const ::GType actual_type{ g_boxed_type_register_static(g_intern_static_string("string_view"),
      reinterpret_cast<::GBoxedCopyFunc>(string_view_copy),
      reinterpret_cast<::GBoxedFreeFunc>(string_view_free)) };
    g_once_init_leave_pointer(&type, actual_type);
  }

  return type;
}

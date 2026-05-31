#include "vin/lib/ipositionable.hpp"

#include <peel/class.h>

using namespace vin::lib;

peel::Type IPositionable::_peel_get_type() noexcept
{
  static ::GType type;

  if (_peel_once_init_enter(&type)) {
    const ::GType result_type{ ::g_type_register_static_simple(peel::Type::interface_(),
      ::g_intern_static_string("VinLibIPositionable"),
      sizeof(IPositionable),
      nullptr,
      0,
      nullptr,
      ::GTypeFlags(G_TYPE_FLAG_FINAL | G_TYPE_FLAG_ABSTRACT)) };
    _peel_once_init_leave(&type, result_type);
  }

  return type;
}

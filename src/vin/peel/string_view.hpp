#include <peel/GObject/Type.h>
#include <peel/GObject/Value.h>
#include <peel/signal.h>

#include <string_view>

template<>
peel::GObject::Type peel::GObject::Type::of<std::string_view>();

template<>
struct peel::GObject::Value::Traits<std::string_view>
{
  using UnownedType = std::string_view;

  static UnownedType get(const ::GValue* const value) noexcept
  {
    return *static_cast<UnownedType*>(g_value_get_boxed(value));
  }
};

template<>
struct peel::internals::SignalTraits<std::string_view>
{
  using CType = std::string_view*;
  using PlainCppType = std::string_view;

  static CType to_c(const PlainCppType v)
  {
    return new std::string_view(v);
  }

  static PlainCppType from_c(const CType v)
  {
    return *v;
  }
};

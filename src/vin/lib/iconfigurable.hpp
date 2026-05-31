#pragma once

#include <peel/GObject/Type.h>
#include <peel/GObject/TypeInterface.h>

extern "C" {
#include <lua.h>
}

namespace vin::lib {

class IConfigurable final : public peel::GObject::TypeInterface
{
private:
  using IConfigure = void (*)(IConfigurable*, lua_State*);

  IConfigure m_configure;

public:
  IConfigurable() = delete;
  IConfigurable(const IConfigurable&) = delete;
  IConfigurable(IConfigurable&&) = delete;
  IConfigurable& operator=(const IConfigurable&) = delete;
  IConfigurable& operator=(IConfigurable&&) = delete;
  ~IConfigurable() = delete;

  static peel::Type _peel_get_type() noexcept;

  void configure(lua_State* const state)
  {
    auto* const iface{ G_TYPE_INSTANCE_GET_INTERFACE(this, peel::Type::of<IConfigurable>(), IConfigurable) };
    if (iface->m_configure != nullptr) {
      iface->m_configure(this, state);
    }
  }

  class Iface : public peel::GObject::TypeInterface
  {
  private:
    [[maybe_unused]] unsigned char m_filler[sizeof(m_configure)];

  public:
    Iface() = delete;
    Iface(const Iface&) = delete;
    Iface(Iface&&) = delete;
    Iface& operator=(const Iface&) = delete;
    Iface& operator=(Iface&&) = delete;
    ~Iface() = delete;

    template<typename Derived>
    void override_vfunc_configure()
    {
      auto* const iface{ reinterpret_cast<IConfigurable*>(this) };
      iface->m_configure = static_cast<IConfigure>([](IConfigurable* const configurable, lua_State* const state) {
        Derived* const derived{ reinterpret_cast<Derived*>(configurable) };
        derived->Derived::vfunc_configure(state);
      });
    }
  };
};

static_assert(sizeof(IConfigurable) == sizeof(IConfigurable::Iface));
static_assert(alignof(IConfigurable) == alignof(IConfigurable::Iface));

} // namespace vin::lib

#pragma once

#include <peel/GObject/Type.h>
#include <peel/GObject/TypeInterface.h>

namespace vin::lib {

enum class Position : unsigned char { top, left, right, bottom };

class IPositionable final : public peel::GObject::TypeInterface
{
private:
  using IPosition = void (*)(IPositionable*, Position);

  IPosition m_position;

public:
  IPositionable() = delete;
  IPositionable(const IPositionable&) = delete;
  IPositionable(IPositionable&&) = delete;
  IPositionable& operator=(const IPositionable&) = delete;
  IPositionable& operator=(IPositionable&&) = delete;
  ~IPositionable() = delete;

  static peel::Type _peel_get_type() noexcept;

  void position(const Position position)
  {
    auto* const iface{ G_TYPE_INSTANCE_GET_INTERFACE(this, peel::Type::of<IPositionable>(), IPositionable) };
    iface->m_position(this, position);
  }

  class Iface : public peel::GObject::TypeInterface
  {
  private:
    [[maybe_unused]] unsigned char m_filler[sizeof(m_position)];

  public:
    Iface() = delete;
    Iface(const Iface&) = delete;
    Iface(Iface&&) = delete;
    Iface& operator=(const Iface&) = delete;
    Iface& operator=(Iface&&) = delete;
    ~Iface() = delete;

    template<typename Derived>
    void override_vfunc_position()
    {
      auto* const iface{ reinterpret_cast<IPositionable*>(this) };
      iface->m_position = static_cast<IPosition>([](IPositionable* const positionable, const Position position) {
        Derived* const derived{ reinterpret_cast<Derived*>(positionable) };
        derived->Derived::vfunc_position(position);
      });
    }
  };
};

static_assert(sizeof(IPositionable) == sizeof(IPositionable::Iface));
static_assert(alignof(IPositionable) == alignof(IPositionable::Iface));

} // namespace vin::lib

#pragma once

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include <cstdint>

namespace wayland {

class registry;

struct registry_listener
{
  void (*global)(void* data, registry* registry, uint32_t name, const char* interface, uint32_t version);
  void (*global_remove)(void* data, registry* registry, uint32_t name);
};

class registry
{
public:
  registry() = delete;
  registry(const registry&) = delete;
  registry(registry&&) = delete;
  registry& operator=(const registry&) = delete;
  registry& operator=(registry&&) = delete;
  ~registry() = delete;

  void add_listener(const registry_listener* const listener, void* const state)
  {
    ::wl_registry_add_listener(
      reinterpret_cast<::wl_registry*>(this), reinterpret_cast<const ::wl_registry_listener*>(listener), state);
  }

  template<typename T>
  T* bind(const uint32_t name, const ::wl_interface* const interface, const uint32_t version)
  {
    return reinterpret_cast<T*>(::wl_registry_bind(reinterpret_cast<::wl_registry*>(this), name, interface, version));
  }
};

} // namespace wayland

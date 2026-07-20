#pragma once

#include <peel/GLib/MainContext.h>
#include <peel/property.h>

namespace vin {

struct MainContext
{
  peel::GLib::MainContext* main_context;
  peel::GLib::MainContext* worker_context;
};

#define VIN_MAIN_CONTEXT_PROPERTY                                          \
  PEEL_PROPERTY(peel::GLib::MainContext, main_context, "main-context")     \
  void set_main_context(peel::GLib::MainContext* context);                 \
  [[nodiscard]] peel::GLib::MainContext* get_main_context() const;         \
                                                                           \
  PEEL_PROPERTY(peel::GLib::MainContext, worker_context, "worker-context") \
  void set_worker_context(peel::GLib::MainContext* context);               \
  [[nodiscard]] peel::GLib::MainContext* get_worker_context() const;

#define VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(tp)                     \
  void tp::set_main_context(peel::GLib::MainContext* const context) \
  {                                                                 \
    m_main_context = context;                                       \
    notify(prop_main_context());                                    \
  }                                                                 \
                                                                    \
  peel::GLib::MainContext* tp::get_main_context() const             \
  {                                                                 \
    return m_main_context;                                          \
  }                                                                 \
                                                                    \
  void tp::set_worker_context(GLib::MainContext* const context)     \
  {                                                                 \
    m_worker_context = context;                                     \
    notify(prop_worker_context());                                  \
  }                                                                 \
                                                                    \
  GLib::MainContext* tp::get_worker_context() const                 \
  {                                                                 \
    return m_worker_context;                                        \
  }

#define VIN_DEFINE_MAIN_CONTEXT_PROPERTY(tp)                                               \
  visitor.prop(prop_main_context()).set(&tp::set_main_context).get(&tp::get_main_context); \
  visitor.prop(prop_worker_context()).set(&tp::set_worker_context).get(&tp::get_worker_context);

} // namespace vin
